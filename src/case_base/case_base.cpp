#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <list>
#include <algorithm>

using namespace std;

#include "case_base.h"
#include "../macros.h"

int timesteps_between(pair<int,string> a, pair<int,string> b)
{
	//interestingly enough, our case base seems to label the spring of 2010, 
	//for example, and the "spring 2009" semester.

	int year = 2*(b.first - a.first);
	int semester = (b.second == "Fall" ? 0 : 1) - (a.second == "Fall" ? 0 : 1);

	return year + semester;
}

//return true if a is_later than b
bool is_later(pair<int,string> a, pair<int,string> b)
{
	return timesteps_between(a,b) < 0;
//	if(a.first == b.first) //same year, Fall > Spring, but
//		return a.second < b.second;	// S > F lexigraphically

//	return a.first > b.first;
}

bool case_base_t::init(string filename)
{
	cases.empty();
	ifstream in(filename.c_str());

	case_t this_case;
	int i=0;
	string s;

	while(in) {
		in >> s;
		switch(i%5) {
			case 0:
				this_case.id = atoi(s.c_str());
				break;
			case 1:
				this_case.course = s;
				break;
			case 2:
				this_case.year = atoi(s.c_str());
				break;
			case 3:
				this_case.semester = s;
				break;
			case 4:
				this_case.grade = s;
				cases.push_back(this_case);
		}

		++i;
	}

	return true;
}

map<string,double> case_base_t::get_naive_dist(string to_take, 
									string &dist_course, int &total_cases)
{
	map<string, double> dist;
	total_cases = 0;

	//translate incoming action language into case_base course language
	string course = translate_action_name(to_take);

	//and then translate the case_base course language into incoming action
	// language for return
	dist_course = detranslate_course_name(course);


	//linear search through cases
	FOREACH(c, cases) {
		if(c->course == course) {
			string g = detranslate_grade(c->grade);
			
			//if we haven't seen this grade key before, init to 0
			if(dist.find(g) == dist.end())
				dist[g] = 0;

			dist[g]++;
			total_cases++;
		}
	}

	FOREACH(d,dist)
		d->second /= total_cases;
	
	return dist;
}

map<string,double> case_base_t::get_conditional_dist(string taken, 
								vector<string> grades, string to_take,
								int cutoff)
{
	//Find all of the case pairs which match the inputs
	//
	//NB: the 'cases' referred to here match the description of cases
	// from the paper i.e. action->state->action->state sequences
	// but the return value is the vector of case_t (action->state) 
	// for the second action in that sequence. Try to keep up ;)
	map<string,double> dist;
	vector<case_t> matches = get_conditioned_cases(translate_var_name(taken),
													translate_var_values(grades),
													translate_action_name(to_take));

	//build a distribution if we have enough cases
	if(matches.size() >= cutoff) {
		FOREACH(m,matches) {
			string g = detranslate_grade(m->grade);

			//if this is the first time we've seen a key, init value to 0
			if(dist.find(g) == dist.end())
				dist[g] = 0;

			dist[g]++;
		}
		
		FOREACH(d, dist)
			d->second /= matches.size();
	}

	return dist;
}

//This is O(N^2) and could be O(N*logN) with better data structures - but 
// case base is small enough that it doesnt matter
vector<case_t> case_base_t::get_conditioned_cases(string taken, set<string> grades, 
										string to_take)
{
	//abuse of std::pair to make a list of (id,year,semester) tuples
	vector<pair<int, pair<int,string> > > ids;
	vector<case_t> matches;

	//find all of the case_t which have one of the matching grades in the
	// previously taken course
	//
	// complexity O(N)
	FOREACH(c, cases) {
		if(c->course == taken && grades.find(c->grade) != grades.end()) {
			ids.push_back(make_pair(c->id, make_pair(c->year, c->semester)));
		}
	}

	//now find the first case_t which match the second half of the 'case'
	// pair. Citeria are: (a) occured later (b) match the provided action to_take
	// (c) have matching ids
	FOREACH(c, cases) {
		bool found = false;
		pair<int,string> when;

		FOREACH(i, ids) {
			if(c->id == i->first) {
				found = true;
				when = i->second;
				break;
			}
		}

		if(!found)
			continue;

		if(c->course == to_take && is_later(make_pair(c->year, c->semester), 
											when)) {
			matches.push_back(*c);
		}
	}

	return matches;
}

map<int,double> case_base_t::get_times_to_scenario(map<string,string> cur_state,
							map<string, vector<string> > objective, 
							string * next_step_action)
{	
	//build lists of cases indexes by id.
	// the case base itself should actually be a map<int, list<case_t> > 
	// but i dont want to have to retest everything else at this point.
	map<int,list<int> > case_map;

	//time to scenario distribution
	// maps timestep -> P(timestep)
	map<int, double> tts_dist;
	int total_tts = 0;

	if(cur_state.empty())
		return tts_dist;

	//Here we remove any of the current state assignments which match 
	//assignments in the objective, so that we can accurately count the
	//timesteps between the current state and the attainment of the objective
	FOREACH(a, cur_state) {
		if(objective.find(a->first) == objective.end())
			continue;

		bool found = false;
		FOREACH(g,objective[a->first]) {
			if(a->second == *g) {
				found = true;
			}
		}

		if(found)
			objective.erase(a->first);
	}

	//This is not great, but it will suffice
	if(objective.empty()) {
		tts_dist[0] = 1;
		return tts_dist;
	}

	REP(i,cases.size()) {
		case_map[cases[i].id].push_back(i);
	}

	//this loops through individual students - the ndx_list is a list of courses and grades for a given student
	FOREACH(ndx_list,case_map) {
		bool next_step_action_matched = false;
		int current_matches = 0;
		int objective_matches = 0;

		pair<int,string> start_date = make_pair(-666, "");
		pair<int,string> end_date = make_pair(666, "");

		//see if past case for this individual matches the state we're looking for
		FOREACH(a,cur_state) {
			string v = translate_var_name(a->first);
			set<string> grades = translate_var_values(vector<string>(1,a->second));
			
			// now loop through grade assignments linked to the same ID
			FOREACH(ndx,ndx_list->second) {
				case_t * the_case = &(cases[*ndx]);

				if(the_case->course == v && grades.end() != find(grades.begin(), grades.end(), the_case->grade)) { 
					pair<int,string> date = make_pair(the_case->year, the_case->semester);

					//if this is the first date we've seen, or if the date
					//in question is later than the current latest seen date
					if(current_matches == 0 || is_later(date, start_date))
						start_date= date;

					current_matches++;
				}
			}
		}

		//have we matched every part of the state?
		if(current_matches != cur_state.size())
			continue;

		// now match the next step action, if one is provided
		if(next_step_action != NULL) {
			string v = translate_action_name(*next_step_action);

			// now loop through grade assignments linked to the same ID
			FOREACH(ndx,ndx_list->second) {
				case_t * the_case = &(cases[*ndx]);
				
				if(the_case->course == v) { 
					pair<int,string> date = make_pair(the_case->year, the_case->semester);
					
					if(timesteps_between(start_date, date) == 1) {
						next_step_action_matched = true;
						break;
					}
				}
			}

			if(!next_step_action_matched)
				continue;
		}

		// now match the objectives
		FOREACH(a,objective) {
			string v = translate_var_name(a->first);
			set<string> grades = translate_var_values(a->second);

			// now loop through grade assignments linked to the same ID
			FOREACH(ndx,ndx_list->second) {
				case_t * the_case = &(cases[*ndx]);
				
				if(the_case->course == v && grades.end() != find(grades.begin(), grades.end(), the_case->grade)) { 
					pair<int,string> date = make_pair(the_case->year, the_case->semester);
					
					//if this is the first date we've seen, or if the date
					//in question is later than the current latest seen date
					if(objective_matches == 0 || is_later(date,end_date))
						end_date = date;

					//TODO: this should probably be a map, and the method of
					// counting objectives matched should be fixed to allow
					// for correct dating of repeats (i.e. a student who has
					// an objective assignment twice).
					objective_matches++;
				}
			}
		}
		
		if(objective_matches == objective.size()) {
			int dt = timesteps_between(start_date,end_date);
			
			//TODO: this is kind of a hack, but I can't think of anything better
			// at the moment. It is possible to complete courses out of order,
			// but we would rather not present those, so just nip them here.
			if(dt < 0)
				continue;

			total_tts++;

			// if we havent seen this key before, init value to 0 so we can safely increment
			if(tts_dist.find(dt) == tts_dist.end()) {
				tts_dist[dt] = 0;
			}

			tts_dist[dt]++;
		}
	}

	FOREACH(it,tts_dist) {
		it->second /= total_tts;
	}
	
	return tts_dist;
}

string case_base_t::translate_var_name(string v)
{
// PSY_CS_SWITCH
//
// FOR CS
//	string s = v.substr(0,2);
//	s += "_";
//	s += v.substr(2);

// FOR PSY
	string s = v.substr(0,3);
	s += "_";
	s += v.substr(3);
	return s;
}

string case_base_t::detranslate_course_name(string v)
{
// PSY_CS_SWITCH
//
//FOR CS
//	return v.substr(0,2) + v.substr(3); 
//
//FOR PSY
	return v.substr(0,3) + v.substr(4);
}

set<string> case_base_t::translate_var_values(vector<string> vs)
{
	set<string> s;
	FOREACH(v,vs) {
		if(*v == "Good") {
			s.insert("A");
		} else if(*v == "Pass") { 
			s.insert("B");
			s.insert("C");
		} else if(*v == "Fail") {
			s.insert("D");
			s.insert("E");
		}
	}

	return s;
}

string case_base_t::translate_action_name(string a)
{
	return translate_var_name(a.substr(5));
}

string case_base_t::detranslate_grade(string g)
{
	if(g == "A")
		return "Good";
	if(g == "B" || g == "C")
		return "Pass";
	if(g == "D" || g == "E")
		return "Fail";
	
	return "NotTaken";
}

