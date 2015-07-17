#include <iostream>
#include <fstream>
#include <cctype>
#include <cstdlib>

#include "../macros.h"

using namespace std;

#include "policy_parser.h"

policy_parser_t::policy_parser_t(string file)
	: tokenizer(file)
{}

void policy_parser_t::set_var_assignment(string s, 
					vector<pair<string,string> > &assignment)
{
	//parse out the variable name and it's assignment
	string var, value;
	string * to_str = &var;

	s = s.substr(1, s.length()-2);
	
	for(int i=0; i < s.length(); ++i) {
		if(s[i] == '=')
			to_str = &value;
		else
			to_str->push_back(s[i]);
	}

	value = value.substr(0, value.length()-1);

	//now figure out what to do with it: if we have a full assignment and
	// 	we are replacing a var, we have to search, otherwise, just stick it
	//	in the vector and that becomes the default tree ordering
	//VAR is a macro that creates an x of type typeof(y)
	VAR(assign, assignment.begin());

	for(; assign != assignment.end(); ++assign) {
		if(assign->first == var)
			break;
	}

	if(assign == assignment.end()) {
		assignment.push_back(make_pair(var,value));
	} else {
		assign->second = value;
	}
}

bool policy_parser_t::insert_leaf(dd_root_t * root, 
					vector<pair<string,string> > assignment, 
					int assign_ndx, double value, string action)
{
	string assign_var = assignment[assign_ndx].first;
	string assign_value = assignment[assign_ndx].second;

	if(!root) {
		cout << "WTF NULL" << endl;
		return false;
	}

	if(assign_var != root->var) {
		cout << "WTF ASSIGNMENT ERROR" << endl;
		return false;
	}

	bool child_exists = (root->kids.find(assign_value) != root->kids.end());

	if(assignment.size() - 1 == assign_ndx) {	//leaf node
		dd_leaf_t * leaf_ptr = new dd_leaf_t();
		leaf_ptr->action = action;
		leaf_ptr->value = value;

		if(child_exists) {
			cout << "WTF ASSIGNMENT EXISTS!" << endl;
			return false;
		}

		root->kids[assign_value] = (dd_base_t*)leaf_ptr;
		return true;
	} else if(assignment.size() - 1 > assign_ndx) {
		dd_root_t * child_ptr = NULL;

		if(child_exists) {
			dd_base_t *	child_base_ptr = root->kids[assign_value];
			if(child_base_ptr->is_leaf()) {
				cout << "WTF A LEAF" << endl;
				return false;
			}

			child_ptr = (dd_root_t *)child_base_ptr;
		} else {
			child_ptr = new dd_root_t();
			
			string next_assign_var = assignment[assign_ndx+1].first;
			child_ptr->var = next_assign_var;
			root->kids[assign_value] = child_ptr;
		}
		
		return insert_leaf(child_ptr, assignment, assign_ndx+1, value, action);
	} else {
		cout << "WTF NDX TOO BIG" << endl;
		return false;
	}

	cout << "WTF SHOULDNT BE HERE" << endl;
	return false;
}

void policy_parser_t::print_tree(dd_base_t * root, int lvl)
{
	if(root->is_leaf()) {
		REP(i,lvl)
			cout << "  ";
	
		cout << ((dd_leaf_t*)root)->action << " " 
			 << ((dd_leaf_t*)root)->value << endl;

		return;
	}

	FOREACH(child, ((dd_root_t*)root)->kids) {
		REP(i,lvl)
			cout << "  ";
	
		cout << ((dd_root_t*)root)->var << " = " << child->first << endl;
		print_tree(child->second, lvl+1);
	}
}

dd_root_t * policy_parser_t::parse()
{
	dd_root_t * policy_root	= new dd_root_t();

	string s = tokenizer.get_next();

	vector<pair<string, string> > assignment;

	do {
		if(s[0] == '<') {
			set_var_assignment(s, assignment);

			//gots to set this up here
			if(policy_root->var == "")
				policy_root->var = assignment[0].first;
		} else if(isdigit(s[0])) {
			double value = atof(s.c_str());

			s = tokenizer.get_next();
			if(s[0] != '{') {
				cout << "Parse error!" << endl;
				return NULL;
			}
			string action = s.substr(8, s.length()-9);

			if(!insert_leaf(policy_root, assignment, 0, value, action)) {
				return NULL;
			}
		}

		s = tokenizer.get_next();
	} while(s != "");

	return policy_root;
}
