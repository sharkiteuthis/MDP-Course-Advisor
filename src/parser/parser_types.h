#ifndef PARSER_TYPES_H
#define PARSER_TYPES_H

#include <string>
#include <map>
#include <vector>

/////////////////////////////////////////////
//SHARED TYPES
/////////////////////////////////////////////
//
//base class for a decision diagram node
//
class dd_base_t {
public:
	virtual bool is_next_step() = 0;
	virtual bool is_leaf() = 0;
	virtual string get_name() = 0;
	virtual dd_base_t * get_child(string key) = 0;
};

/////////////////////////////////////////////
//MODEL PARSER TYPES
/////////////////////////////////////////////
class variable_t {
public:
	variable_t(string n) { name = n; }

	void add_value(string v) { values.push_back(v); }

	string name;
	vector<string> values;
};

class dd_node_t : public dd_base_t {
public: 
	dd_node_t() {}

	bool is_next_step() { return next_step; }
	bool is_leaf() { return children.empty(); }

	string get_name() { return var_name; }

	dd_base_t * get_child(string key) { 
		if(children.find(key) == children.end())
			return NULL;

		return children[key];
	}

	string var_name;
	bool next_step;		//in the model, am i a 'primed' node (result at next time step)
	double value;		//terminal nodes (children.size() == 0)
	map<string, dd_node_t*> children; 
};

class action_t {
public:
	map<string, dd_node_t*> var_dds;
};

///////////////////////////////////////////////////
//POLICY PARSER TYPES
///////////////////////////////////////////////////

class dd_root_t : public dd_base_t {
public:
	bool is_next_step() { return false; }
	bool is_leaf() { return false; }
	string get_name() { return var; }
	dd_base_t * get_child(string key) { 
		if(kids.find(key) == kids.end())
			return NULL;

		return kids[key];
	}
	
	string var;
	map<string, dd_base_t*> kids;
};

class dd_leaf_t : public dd_base_t {
public:
	bool is_next_step() { return true; }
	bool is_leaf() { return true; }
	string get_name() { return action; }
	dd_base_t * get_child(string key) { return NULL; }
	double value;
	string action;
};

#endif

