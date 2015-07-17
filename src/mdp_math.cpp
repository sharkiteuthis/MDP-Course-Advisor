#include "mdp_math.h"
#include "macros.h"
#include <algorithm>

#include <iostream>
#include <fstream>

using namespace std;

//ofstream debug_out;

dd_base_t * get_leaf(dd_base_t * root, map<string,string> state, int &err)
{
	while(!root->is_next_step() && !root->is_leaf()) {
		string var_name = root->get_name();

		if(var_name == "") {
			err = 3;
			return NULL;
		}

		if(state.find(var_name) == state.end()) {
			err = 4;
			return NULL;
		}

		string val = state[var_name];

		dd_base_t * kid = root->get_child(val);

		if(kid == NULL) {
			err = 5;
			return NULL;
		}

		root = kid;
	}

	return root;
}

int get_state_value(dd_root_t * policy_root, map<string,string> state, 
						double & val)
{
	int err = 0;
	dd_leaf_t * policy_leaf = (dd_leaf_t*)get_leaf((dd_base_t*)policy_root, 
													state, err);
	
	if(!policy_leaf || err)
		return err;

	val = policy_leaf->value;

	return 0;
}

int get_distribution(string action, string var, 
			map<string, string> &state, model_parser_t &parser,
			vector<pair<string, double> > &dist)
{
	if(parser.action_table.find(action) == parser.action_table.end())
		return 1;
	
	action_t * action_effect = parser.action_table[action];

	if(action_effect->var_dds.find(var) == action_effect->var_dds.end())
		return 2;
	
	int err = 0;
	dd_node_t * var_dd = (dd_node_t*)get_leaf(action_effect->var_dds[var], 
												state, err);

	if(!var_dd)
		return err;
	
	if(var_dd->var_name != var)
		return 6;

	FOREACH(ns, var_dd->children) {
		string next_val = ns->first;
		double next_prob = ns->second->value;

		if(next_prob > 0)

		dist.push_back(make_pair(next_val, next_prob));
	}

	return 0;
}

int get_optimal_action(string &act, map<string,string> state, 
						dd_root_t * policy_root)
{
	int err = 0;
	dd_leaf_t * leaf = (dd_leaf_t*)get_leaf(policy_root, state, err);

	if(!leaf)
		return err;

	act = leaf->action;
	return 0;
}

int get_next_states(model_parser_t &mp, map<string,string> cur_state, 
					string action, vector<map<string,string> > &next_states,
					vector<double> &probabilities)
{
	map<string, vector<pair<string, double> > > var_dists;
	FOREACH(var, cur_state) {
		//for each variable in the state, we need to return a probability
		// distribution of changes to the variable
		vector<pair<string, double> > value_dist;
		int ret = get_distribution(action, var->first, cur_state, mp, 
									value_dist);

		if(ret)
			return ret;

		var_dists[var->first] = value_dist;
	}

	//here we multiply out the probability distributions to get next states
	// and their likelyhoods

	next_states.push_back(cur_state);
	probabilities.push_back(1.0);

	//each distribution is a vector<pair<string, double> >::iterator, 
	// var_dists is a map of those suckers, with the var name as key
	FOREACH(d, var_dists) {
		string var_name = d->first;
		vector<pair<string, double> > var_dist = d->second;

		vector<map<string,string> > tmp_states;
		vector<double> tmp_probs;

		REP(i, next_states.size()) {
			FOREACH(v, var_dist) {
				map<string,string> process_state = next_states[i];
				string value = v->first;

				double new_prob = v->second * probabilities[i];
				
				if(process_state.find(var_name) == process_state.end()) {
					return 11;
				}
				
				process_state[var_name] = value;

				tmp_states.push_back(process_state);
				tmp_probs.push_back(new_prob);
			}
		}

		next_states = tmp_states;
		probabilities = tmp_probs;
	}

	return 0;
}

double evaluate_reward(model_parser_t &mp, map<string,string> state)
{
	double R = 0;

	REP(i, mp.reward_dds.size()) {
		dd_node_t * node = mp.reward_dds[i];
	
		int err = 0;
		dd_node_t * leaf = (dd_node_t*)get_leaf(node, state, err);

		if(leaf != NULL)
			R += leaf->value;
	}

	return R;
}

int get_2step_recommendation_percent(double & p, model_parser_t &mp, 
						dd_root_t * policy_root, 
						map<string,string> cur_state, string act1, string act2)
{
	int ret = 0;

	p = 0;

	//get next state dist under act1
	vector<map<string,string> > ns_1step;	// s' states
	vector<double>				prob_1step;	// T(s'|cur_state,a1)
	
	ret = get_next_states(mp, cur_state, act1, ns_1step, prob_1step);
	if(ret) return ret;

	REP(i, ns_1step.size()) {
		map<string,string> ns1 = ns_1step[i];

		string act;
		ret = get_optimal_action(act, ns1, policy_root);

//		debug_out << "\t\t\t\t" << act << "\t" << prob_1step[i] << endl;

		if(act == act2) {
			p += prob_1step[i];
		}

	}

//	debug_out << "\t\t\t" << p << endl;

	return ret;
}

int get_2step_value(double & V, model_parser_t &mp, dd_root_t * policy_root, 
						map<string,string> cur_state, string act1, string act2)
{
	int ret = 0;

	//get next state dist under act1
	vector<map<string,string> > ns_1step;	// s' states
	vector<double>				prob_1step;	// T(s'|cur_state,a1)
	
	double R = 0;
	double Va1_sum = 0;	//outer sum over s' states

	ret = get_next_states(mp, cur_state, act1, ns_1step, prob_1step);
	if(ret) return ret;

	REP(i, ns_1step.size()) {
		map<string,string> ns1 = ns_1step[i];

		//get next state dist under act2
		vector<map<string,string> > ns_2step;	// s'' states
		vector<double>				prob_2step;	// T(s''|s',a2)

		double Va2_sum = 0;		//innermost sum over s'' states
		ret = get_next_states(mp, ns1, act2, ns_2step, prob_2step);
		if(ret) return ret;
		
		REP(j, ns_2step.size()) {
			map<string,string> ns2 = ns_2step[j];
			double Vpi_ns2 = 0;		//V^{\pi}(s'')
			ret = get_state_value(policy_root, ns2, Vpi_ns2);
			if(ret) return ret;

			Va2_sum += prob_2step[j] * Vpi_ns2;
			
//			debug_out << "\t\t\t\t\t" << prob_1step[i] * prob_2step[j] << "\t" 
//						<< Vpi_ns2 << endl;

		}
		
		//Va1_sum += (prob_1step[i] * (mp.discount * Va2_sum));
		
		R = evaluate_reward(mp, ns1);

		Va1_sum += (prob_1step[i] * (R + (mp.discount * Va2_sum)));
//		debug_out << "\t\t\t\t" << prob_1step[i] << "\t" << R << "\t" << endl;
	}

	//V = (mp.discount * Va1_sum);
	
	R = evaluate_reward(mp, cur_state);
	V = (R + (mp.discount * Va1_sum));

//	debug_out << "\t\t\t" << R << endl;

	return ret;
}

//returns a map of afdvs for some second step action act_2nd, indexed by the
//first step action
int get_afdvs(map<string,double> & afdvs, model_parser_t &mp,
				dd_root_t * policy_root, map<string,string> cur_state,
				string policy_act, string act2)
{
	double V2_policy = 0;
	int err = get_2step_value(V2_policy, mp, policy_root, cur_state, 
								policy_act, act2);
	if(err) return err;

//	debug_out << '\t' << "(" << policy_act << "," << act2 << ")" 
//				<< '\t' << V2_policy << endl;

	afdvs[policy_act] = 0;	//identity
	
	FOREACH(act_it, mp.action_table) {
		string act1 = act_it->first;
		if(act1 == policy_act)	//no need to compute this, identically 0
			continue;

		double V2_act = 0;

		err = get_2step_value(V2_act, mp, policy_root, cur_state, act1, act2);
		if(err) return err;


//		debug_out << "\t\t" << "(" << act1 << "," << act2 << ")" 
//					<< '\t' << V2_act << endl;

		afdvs[act1] = (V2_policy - V2_act);
	}

	return 0;
}

//returns a map of AFRAPDs for some second step action act_2nd, indexed by the
//first step action
// AFRAPD is action factored recommended action percent difference - a 
// measure of how much more likely act2 is to be the recommended action in 
// the next step after some a is taken instead of policy_act
int get_afrapds(map<string,double> & afrapds, model_parser_t &mp,
				dd_root_t * policy_root, map<string,string> cur_state,
				string policy_act, string act2)
{
	double p_policy = 0;
	int err = get_2step_recommendation_percent(p_policy, mp, policy_root, 
										cur_state, policy_act, act2);
	if(err) return err;

//	debug_out << '\t' << "(" << policy_act << "," << act2 << ")" 
//				<< '\t' << p_policy << endl;

	afrapds[policy_act] = 0;	//identity
	
	FOREACH(act_it, mp.action_table) {
		string act1 = act_it->first;
		if(act1 == policy_act)	//no need to compute this, identically 0
			continue;

		double p_act = 0;

		err = get_2step_recommendation_percent(p_act, mp, policy_root, 
												cur_state, act1, act2);
		if(err) return err;


//		debug_out << "\t\t" << "(" << act1 << "," << act2 << ")" 
//					<< '\t' << p_act << endl;

		afrapds[act1] = (p_policy - p_act);
	}

	return 0;
}

//returns the count of afdvs > 0, indexed by second step action
int get_afdv_counts(map<string,int> & counts, model_parser_t &mp, 
				dd_root_t * policy_root, map<string,string> cur_state,
				string policy_act)
{
//	debug_out.open("DEBUG.TXT");
//	debug_out << "COMPUTING AFDVS FOR " << policy_act << endl << endl;
	map<string,double> afdvs;

	FOREACH(act_it, mp.action_table) {
		string act2 = act_it->first;
		counts[act2] = 0;
		
		int err = get_afdvs(afdvs, mp, policy_root, cur_state, 
									policy_act, act2);
		if(err) return err;

		FOREACH(afdv,afdvs) {
			if(afdv->second < 0) {
				counts[act2]++;
			}
		}
	}

//	debug_out.close();

	return 0;
}

//returns the count of afrapds > 0, indexed by second step action
int get_afrapds_counts(map<string,int> & counts, model_parser_t &mp, 
				dd_root_t * policy_root, map<string,string> cur_state,
				string policy_act)
{
//	debug_out.open("DEBUG.TXT");
//	debug_out << "COMPUTING AFRAPDS FOR " << policy_act << "..." << endl; 
	map<string,double> afrapds;

	FOREACH(act_it, mp.action_table) {
		string act2 = act_it->first;
		counts[act2] = 0;

//		debug_out << " AND " << act2 << endl;
		
		int err = get_afrapds(afrapds, mp, policy_root, cur_state, 
									policy_act, act2);
		if(err) return err;

		FOREACH(afrapd,afrapds) {
//		debug_out << afrapd->first << " " << afrapd->second << endl;
			if(afrapd->first != policy_act &&		//this is identically 0
					afrapd->second <= 0) 
			{
				counts[act2]++;
			}
		}
//		debug_out << counts[act2] << " actions cause " << act2 << 
//					" to become recommended more often or equally often" 
//					<< endl;
//		debug_out << endl << endl;
	}

//	debug_out.close();

	return 0;
}

//this computes a Q value - no longer used but no need to get rid of it
/*
int get_action_values(model_parser_t &mp, dd_root_t * policy_root,
						map<string,string> cur_state, 
						map<string, double> &action_vals)
{
	FOREACH(act_it, mp.action_table) {
		string action = act_it->first;

		vector<map<string,string> > next_states;
		vector<double> probabilities;

		int ret = get_next_states(mp, cur_state, action, next_states,
									probabilities);

		if(ret)
			return ret;

		double act_val = 0;
	
		//COMPUTE NEXT STATE VALUE
		REP(i, next_states.size()) {
			map<string,string> nst = next_states[i];
			double prob = probabilities[i];

			double nst_val = 0;
			int ret = get_state_value(policy_root, nst,nst_val);
			if(ret)
				return ret;

			act_val += prob * nst_val; 
		}

		action_vals[action] = act_val;
	}

	return 0;
}

void print_afdvs(string action, map<string,double> afdvs)
{
	double sum = 0;

	cout << action << endl;
	FOREACH(a, afdvs) {
		cout << a->first << "\t" << a->second << endl;
		sum += a->second;
	}
	cout << "SUM: " << sum << endl;
}

int get_action_factored_differential_values(model_parser_t &mp, 
		dd_root_t * policy_root, map<string,string> cur_state, string action,
		map<string,double> &afdvs, double &afdv_sum)
{
	//1. generate next states and next state probabilities
	vector<map<string,string> > next_states;
	vector<double> ns_probs;
	int ret = get_next_states(mp, cur_state, action, next_states, ns_probs);

	if(ret)
		return ret;

	//make sure to clear these
	FOREACH(act_it, mp.action_table) {
		afdvs[act_it->first] = 0.0;
	}

	//2. generate Q values for current state
	map<string, double> cur_act_vals;
	ret = get_action_values(mp, policy_root, cur_state, cur_act_vals);

	if(ret)
		return ret;

	afdv_sum = 0;

	//3. foreach next state..
	REP(i, next_states.size()) {
		map<string,string> next_state = next_states[i];
		double ns_prob = ns_probs[i];

		//3.1 generate Q values for next state
		map<string,double> ns_act_vals;
		ret = get_action_values(mp, policy_root, next_state, ns_act_vals);

		if(ret)
			return ret;

		//3.2 foreach action
		FOREACH(act_it, mp.action_table) {
			string act_name = act_it->first;

			if(cur_act_vals.find(act_name) == cur_act_vals.end() ||
					ns_act_vals.find(act_name) == ns_act_vals.end())
			   return 9;
		
			double cur_q = cur_act_vals[act_name];
			double ns_q  = ns_act_vals[act_name];

			//3.2.1 compute partial AFDV for this action in this state
			double partial = ns_prob * (ns_q - cur_q);
			afdvs[act_name] += partial;
			afdv_sum += partial;
		}
	}

	return 0;
}

int get_optimal_afdv_set(model_parser_t &mp, dd_root_t * policy_root, 
		map<string,string> cur_state, string action, map<string,double> &afdvs,
		map<string, int> &afdv_better_count)
{
	afdvs.clear();
	afdv_better_count.clear();
	double optimal_sum = 0;
	int error = get_action_factored_differential_values(mp, policy_root, 
									cur_state, action, afdvs, optimal_sum);
	if(error)
		return error;

	//print_afdvs(action, afdvs);

	FOREACH(a, mp.action_table) {
		if(a->first == action)	//no need to do this again
			continue;

		map<string,double> tmp_afdvs;
		double sum;
		error = get_action_factored_differential_values(mp, policy_root, 
										cur_state, a->first, tmp_afdvs, sum);
		if(error)
			return error;
		
		FOREACH(t, tmp_afdvs) {
			if(afdv_better_count.find(t->first) == afdv_better_count.end())
				afdv_better_count[t->first] = 0;

			if(afdvs.find(t->first) == afdvs.end())
				return 50;

			if(t->second > afdvs[t->first])
				afdv_better_count[t->first]++;
		}
	}

	return 0;
}
*/

