#include <string>
#include <map>
#include <stdio.h>
#include <fstream>
#include <stdlib.h>

using namespace std;

#include "mdp_math.h"
#include "explainer.h"
#include "parser/policy_parser.h"
#include "parser/model_parser.h"
#include "macros.h"
#include "case_base/case_base.h"
#include "kb/kb.h"


//PSY_CS_SWITCH
string case_base_file("../dat/casebase/psy_casebase.txt");
string kb_file("../dat/kb/psy_advise.kb");
//string case_base_file("../dat/casebase/casebase_extended.txt");
//string kb_file("../dat/kb/extended_advise.kb");

//don't try to make conditional distributions that have less than this fraction
// of the cases of the naive distribution
#define CASE_CUTOFF_FRACTION	0.1

//don't try to make conditional distributions that have less than this many
// total cases
#define CASE_CUTOFF_ABSOLUTE	20

int case_cutoff(int total_cases)
{
	double cutoff = (double)total_cases * CASE_CUTOFF_FRACTION;

	if(cutoff < CASE_CUTOFF_ABSOLUTE)
		return CASE_CUTOFF_ABSOLUTE;
	
	return (int)cutoff;
}

//ofstream debug_out;

string probability_adv(double d, int num_outcomes)
{
	//adjust from binary outcome scale to n-outcome scale
	double adj = num_outcomes / 2.0;

	if(d > (0.8/adj))
		return "very likely";
	if(d > (0.5/adj))
		return "likely";
	if(d > (0.2/adj))
		return "unlikely";
	
	return "very unlikely";
}

explainer_t::explainer_t(model_parser_t * pmp, dd_root_t * root)
{
	mp_ptr = pmp;
	policy_root = root;
	cb.init(case_base_file);
	kb.init(kb_file);
}

string explainer_t::get_recommendation(map<string,string> state)
{
	string act;
	int error = get_optimal_action(act, state, policy_root);

	if(error) {
		string ret = "ERROR ";
		ret += error;
		ret += "getting recommendataion";
		return ret; 
	}

	string m = generate_mdp_explain(act, state);
	string c = generate_case_base_explain(act, state);
	string g = generate_time_to_goal_explain(act, state);

	return "The recommended action is " + kb.get_action_string(act)
										+ ". " + m + " " + c + " " + g; 
}

bool dist_is_better(map<string,double> d1, map<string,double> d2,
						vector<string> assignments)
{
	double d1sum=0, d2sum=0;

	REP(i, assignments.size()) {
		string a = assignments[i];
		double r;

		//this is hackish, and should be pulled from the policy parser instead
		if(a == "Good") {
			r=6;
		} else if(a == "Pass") {
			r=4;
		} else {
			r=0;
		}


		d1sum += (d1[a]*r);
		d2sum += (d2[a]*r);
	}

	return d1sum > d2sum;
}

string explainer_t::generate_case_base_explain(string act, 
												map<string,string> state)
{
//	debug_out.open("DEBUG.TXT");

	//retrieve the naive distribution corresponding to the given action
	string dist_course;
	int total_cases;
	map<string,double> naive_dist = cb.get_naive_dist(act, dist_course,
														total_cases);

	int cutoff = case_cutoff(total_cases);

//	debug_out << dist_course << " with " << total_cases << " cases (cutoff " 
//				<< cutoff << ")" << endl;

//	FOREACH(d, naive_dist)
//		debug_out << "\t" << d->first << "\t" << d->second << endl;
	
//	debug_out << endl;

	if(total_cases < CASE_CUTOFF_ABSOLUTE)
		return "";

	vector<string> rewarding_grades = kb.rewarding_assignments(dist_course);
	vector<string> exp;	

	// what we should be doing here is building a set of partial assignments
	// over the other variables in the state and choosing the most convincing.
	// but we dont. we just consider each assignment individually.
	FOREACH(a, state) {
		string past_course	= a->first;
		string past_val		= a->second;

		if(past_course == dist_course)
			continue;

//		debug_out << "\t" << past_course << ":\t" << past_val << endl;

		vector<string> grades;

		//First, we try just the grade in question
		grades.push_back(past_val);
		map<string,double> dist_one = cb.get_conditional_dist(past_course, grades, 
														act, cutoff);

//		debug_out << "\t\tone:" << endl;

//		FOREACH(d, dist_one)
//			debug_out << "\t\t\t" << d->first << "\t" << d->second << endl;
		
		bool use_one = false, use_many = false;

		if(dist_one.empty() || !dist_is_better(dist_one, naive_dist, 
								rewarding_grades)) 
		{
			//Now, we try the grade and all lesser grades
			grades = kb.var_assign_less_eq(past_course, past_val);
			map<string,double> dist_many = cb.get_conditional_dist(past_course, 
											grades, act, cutoff);

//			debug_out << "\t\tmany: ";
//			FOREACH(g, grades)
//				debug_out << *g << " ";

//			debug_out << endl;

//			FOREACH(d, dist_many)
//				debug_out << "\t\t\t" << d->first << "\t" << d->second << endl;

			if(!dist_many.empty() && dist_is_better(dist_many, naive_dist,
													rewarding_grades))
				use_many = true;
		} else {
			use_one = true;
		}

		if(use_one || use_many) {
			string e = "Our database indicates that with a grade of ";

			if(use_many) {
				e += "at least ";
			}
			
			e += past_val + " in " + kb.get_variable_string(past_course);
			e += ", you are more likely to recieve a grade of ";
			e += rewarding_grades.back() + " or better in ";
			e += kb.get_variable_string(dist_course) + ".";

			exp.push_back(e);
		}
	}

//	debug_out.close();

	if(exp.size())
		return exp[exp.size()-1];
	
	return "";
}

string explainer_t::generate_mdp_explain(string act, map<string,string> state)
{
	map<string,int> 	afdv_better_count;
	int error = get_afdv_counts(afdv_better_count, *mp_ptr, policy_root,
									state, act);

	if(error) {
		string ret = "ERROR ";
		ret += error;
		ret += "getting afdvs";
		return ret; 
	}

	vector<string> to_use;

	FOREACH(i, afdv_better_count) {
		if(i->second == 0)
			to_use.push_back(i->first);
	}

	string s = "Our model indicates that taking this action will augment your ability to succeed if you ";

	//apply some heuristic here... also need to check for to_use.size() == 0
	// and then iteratively go lower
	FOREACH(t, to_use) {
		s += kb.get_action_string(*t);
		s += ", ";
	}

	s = s.substr(0, s.length()-2);	//remove the last comma... lol

	s += " in future " + kb.get_timestep_noun_plural()  + " . ";

	to_use.clear();

	map<string,int> afrapds_better_count;
	error = get_afrapds_counts(afrapds_better_count, *mp_ptr, policy_root,
									state, act);
	
	if(error) {
		string ret = "ERROR";
		ret += error;
		ret += "getting afrapds";
		return ret;
	}

	FOREACH(i, afrapds_better_count) {
		if(i->second == 0)
			to_use.push_back(i->first);
	}

	s += "(Actions that are more likely to become optimal under recommendation: ";

	//apply some heuristic here... also need to check for to_use.size() == 0
	// and then iteratively go lower
	FOREACH(t, to_use) {
		s += kb.get_action_string(*t);
		s += ", ";
	}

	s = s.substr(0, s.length()-2);	//remove the last comma... lol
	s+=").";

	return s;
}

string explainer_t::generate_time_to_goal_explain(string act, map<string,string> state)
{
	char tmp[1024];

	//this is a terrible hack, and there might be a better way to do this, but we need to remove the "NotTaken"
	// grades from the current state in order to do any matching, as well as the GPA variable
	map<string, string> cur_state;
	FOREACH(it,state) {
		if(it->second == "NotTaken")
			continue;
		if(it->first == "GPA")
			continue;

		cur_state.insert(*it);
	}

	FOREACH(goal,kb.goal_scenarios) {
		string name = goal->first;
		map<string,vector<string> > objective = goal->second;

		map<int,double> tts;

		tts = cb.get_times_to_scenario(cur_state, objective, &act);
		if(!tts.empty()) {
			double d = 0;
			FOREACH(t, tts) {
				d += t->first * t->second;
			}
			int ts = ROUND_TO_INT(d);

			snprintf(tmp,1024,"Past students have taken the recommended action and accomplished their goal of achieving %s in an average of %d %s from the current state.", 
							name.c_str(), ts, kb.get_timestep_noun_plural().c_str());

			return tmp;
		}

		tts = cb.get_times_to_scenario(cur_state, objective, NULL);
		if(!tts.empty()) {
			double d = 0;
			FOREACH(t, tts) {
				d += t->first * t->second;
			}
			int ts = ROUND_TO_INT(d);

			snprintf(tmp,1024,"Past students have accomplished their goal of achieving %s in an average of %d %s from the current state.", 
							name.c_str(), ts, kb.get_timestep_noun_plural().c_str());

			return tmp;
		}
	}

	return "";
}

