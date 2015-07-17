#ifndef KB_H
#define KB_H

#include <string>
#include <vector>
#include <map>

class kb_action_t
{
public:
	string verb;
};

class kb_var_t
{
public:
	string noun;
	string assign_noun;
	vector<string> assign_order;
};

class knowledge_base_t 
{
public:
	bool init(string filename);

	vector<string> var_assign_greater_eq(string var, string val);
	vector<string> var_assign_less_eq(string var, string val);
	vector<string> var_assign_order(string var);
	vector<string> rewarding_assignments(string var);

	string get_action_string(string act);
	string get_variable_string(string var);

	string get_timestep_noun();
	string get_timestep_noun_plural();
	
	map<string,map<string,vector<string> > > goal_scenarios;

	string fill_placeholders(string s);
private:
	map<string, kb_action_t> actions;
	map<string, kb_var_t> vars;
	string time_step;
};

#endif

