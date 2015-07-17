#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

#include <iostream>

using namespace std;

#include "kb.h"
#include "../macros.h"

// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

bool knowledge_base_t::init(string filename)
{
	ifstream in(filename.c_str());

	string s;
	in >> s;

	while(in) {
		if(s == "VAR") {
			string name;
			kb_var_t v;

			in >> name;
			in.ignore(1, '\n');
			getline(in, v.noun);
			trim(v.noun);
			getline(in, v.assign_noun);
			trim(v.assign_noun);

			in >> s;
			while(s != "VAREND") {
				v.assign_order.push_back(s);
				in >> s;
			}
		
			vars[name] = v;
		} else if(s == "ACTION") {
			string name;
			kb_action_t a;
			
			in >> name;
			in.ignore(1, '\n');
			getline(in, a.verb);
			trim(a.verb);

			actions[name] = a;
		} else if(s == "TIME_STEP") {
			in >> time_step;
		} else if(s == "GOAL") {
			string name;
			map<string, vector<string> > g;
			
			getline(in,name);
			trim(name);

			getline(in, s);
			while(s != "GOALEND") {
				istringstream iss(s);
				string v,a;
				iss >> v;
				g[v] = vector<string>();

				iss >> a;
				while(iss) {
					g[v].push_back(a);
					iss >> a;
				}

				getline(in,s);
			}

			goal_scenarios[name] = g;
		}

		in >> s;
	}

	//replace the placeholders in the action verb phrases
	FOREACH(act,actions) {
		act->second.verb = fill_placeholders(act->second.verb);
	}

	return true;
}

string knowledge_base_t::fill_placeholders(string s)
{
	unsigned start = s.find("$$");
	if(start == string::npos)
		return s;

	unsigned end = s.find("$$",start+1);
	if(start == string::npos)
		return s;

	string var = s.substr(start+2,(end-start-2));
	if(vars.find(var) == vars.end())
		return s;

	string noun = vars[var].noun;
	s.replace(start, end-start+2, noun);

	return s;
}

vector<string> knowledge_base_t::var_assign_greater_eq(string var, string val)
{
	vector<string> a;
	kb_var_t v = vars[var];

	bool found = false;

	REP(i, v.assign_order.size()) {
		if(v.assign_order[i] == "0")
			continue;

		a.push_back(v.assign_order[i]);
		if(v.assign_order[i] == val) {
			found = true;
			break;
		}
	}

	//if val is not in the partial order, we can't return anything 
	if(!found)
		a.clear();
	
	return a;
}

vector<string> knowledge_base_t::var_assign_less_eq(string var, string val)
{
	vector<string> a;
	kb_var_t v = vars[var];

	bool found = false;

	REP(i, v.assign_order.size()) {
		if(v.assign_order[i] == "0")
			break;
		
		if(v.assign_order[i] == val)
			found = true;

		if(found)
			a.push_back(v.assign_order[i]);
	}

	return a;
}

vector<string> knowledge_base_t::rewarding_assignments(string var)
{
	vector<string> a;
	kb_var_t v = vars[var];

	REP(i, v.assign_order.size()) {
		if(v.assign_order[i] == "0")
			break;

		a.push_back(v.assign_order[i]);
	}

	return a;
}

vector<string> knowledge_base_t::var_assign_order(string var)
{
	return vars[var].assign_order;
}

string knowledge_base_t::get_action_string(string act)
{
	if(actions.find(act) != actions.end())
		return actions[act].verb;
	
	return act;
}

string knowledge_base_t::get_variable_string(string var)
{
	if(vars.find(var) != vars.end())
		return vars[var].noun;

	return var;
}

string knowledge_base_t::get_timestep_noun()
{
	return time_step;
}

string knowledge_base_t::get_timestep_noun_plural()
{
	return get_timestep_noun() + "s";
}

