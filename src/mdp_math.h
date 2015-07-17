#include <string>
#include <vector>
#include <map>

using namespace std;

#include "parser/model_parser.h"

int get_state_value(dd_root_t * policy_root, map<string,string> state, 
						double & val);

int get_distribution(string action, string var, 
			map<string, string> &state, model_parser_t &parser,
			vector<pair<string, double> > &dist);

int get_optimal_action(string &act, map<string,string> state, 
						dd_root_t * policy_root);

int get_next_states(model_parser_t &mp, map<string,string> cur_state, 
					string action, vector<map<string,string> > &next_states,
					vector<double> &probabilities);

int get_afdv_counts(map<string,int> & counts, model_parser_t &mp, 
				dd_root_t * policy_root, map<string,string> cur_state,
				string policy_act);

int get_afrapds_counts(map<string,int> & counts, model_parser_t &mp, 
				dd_root_t * policy_root, map<string,string> cur_state,
				string policy_act);

/*
int get_action_values(model_parser_t &mp, dd_root_t * policy_root, 
						map<string,string> cur_state, 
						map<string, double> &action_vals);

int get_action_factored_differential_values(model_parser_t &mp, 
		dd_root_t * policy_root, map<string,string> cur_state, string action,
		map<string,double> &afdvs, double &afdv_sum);

int get_optimal_afdv_set(model_parser_t &mp, dd_root_t * policy_root, 
		map<string,string> cur_state, string action, map<string,double> &afdvs,
		map<string, int> &afdv_better_count);
*/
