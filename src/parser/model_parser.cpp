#include <iostream>
#include <fstream>
#include <cctype>
#include <cstdlib>

#include "../macros.h"

using namespace std;

#include "model_parser.h"

model_parser_t::model_parser_t(string file)
	: tokenizer(file)
{
	discount = tolerance = horizon = -1;
}

bool model_parser_t::parse_vars()
{
	int paren_cnt = 1;	//initial opening par

	string s = tokenizer.get_next();
	if(s != "variables") {
		cout << "Unknown token in variable table -	" << s << endl;
		return false;
	}

	variable_t * new_var = NULL;

	while(paren_cnt) {

		s = tokenizer.get_next();

		if(s == "(") {
			paren_cnt++;
		} else if(s == ")") {
			if(new_var) {
				var_table[new_var->name] = new_var;
				new_var = NULL;
			}

			paren_cnt--;
		} else {
			if(!new_var)
				new_var = new variable_t(s);
			else
				new_var->add_value(s);
		}
	}

	return true;
}

dd_node_t * model_parser_t::parse_dd()
{
	int paren_cnt = 0;
	dd_node_t * root = new dd_node_t();

	do {
		string s = tokenizer.get_next();

		if(s == "(") {
			paren_cnt++;
		} else if(s == ")") {
			paren_cnt--;
		} else {
			if(paren_cnt == 1) {
				if(isdigit(s[0]) || s[0] == '-') {
					root->value = atof(s.c_str());
				} else {
					root->next_step = false;
					
					if(s[s.length()-1] == '\'') {
						root->next_step = true;
						s = s.substr(0, s.length()-1);
					}

					//it's either a variable, or a named dd
					if(var_table.find(s) != var_table.end()) {
						root->var_name = s;
					} else if(!root->next_step && 	//flag for "i must be a var"
								dd_table.find(s) != dd_table.end()) {
						delete root;
						root = dd_table[s];
					} else {
						cout << "Unknown Token in DD - " << s << endl;
						return NULL;
					}
				}
			} else if(paren_cnt == 2) {
				dd_node_t * dd = parse_dd();
				root->children[s] = dd;
			}
		}
	} while(paren_cnt);

	return root;
}

void model_parser_t::print_dd(dd_node_t * root, string prefix) 
{
	if(root == NULL) {
		cout << "NULL!" << endl;
		return;
	}

	if(root->children.size() > 0) {
		cout << prefix << root->var_name << endl;

		FOREACH(i, root->children) {
			cout << "\t" << prefix << i->first << endl;
			print_dd(i->second, prefix+"\t\t");
		}
	} else {
		cout << prefix << root->value << endl;
	}
}

action_t * model_parser_t::parse_action()
{
	action_t * act_root = new action_t();

	for(int i=0; i < var_table.size(); ++i) {
		string s = tokenizer.get_next();

		if(s == "endaction") {
			cout << "Action does not define effect on all variables!" << endl;
			return NULL;
		}

		if(var_table.find(s) == var_table.end()) {
			cout << "Undefined variable in action - " << s << endl;
			return NULL;
		}

		dd_node_t * dd_root = parse_dd();

		act_root->var_dds[s] = dd_root;
	}

	return act_root;
}

//this is not a robust parser... at all.
bool model_parser_t::parse_reward()
{
	string s = tokenizer.get_next();

	if(s != "[+") {
		cout << "Reward function is not addative!" << endl;
		return false;
	}

	s = tokenizer.get_next();

	while(s != "]") {
		if(s != ")" && s != "(") {
			if(dd_table.find(s) != dd_table.end()) {
				reward_dds.push_back(dd_table[s]);
			} else {
				cout << "undefined DD used in reward function!" << endl;
				return false;
			}
		}
		s = tokenizer.get_next();
	}
	return true;
}

bool model_parser_t::parse()
{
	string s = tokenizer.get_next();
	int count = 0;

	while(s != "") {
		if(s == "(") {
			if(!parse_vars())
				return false;
		
			/*
			FOREACH(i,var_table) {
				cout << i->first << "\t" << i->second->values.size() << endl;
			}
			return 0;
			*/
		} else if(s == "dd") {
			string name = tokenizer.get_next();
			
			dd_node_t * dd_root = parse_dd();
		
			string t = tokenizer.get_next();

			if(t != "enddd") {
				cout << "dd parse error - " << name << endl;
				cout << t << endl;
				return false;
			}

			if(dd_root != NULL) {
				dd_table[name] = dd_root;
			} else {
				return false;
			}
		} else if(s == "action") {
			string name = tokenizer.get_next();

			action_t * act_root = parse_action();

			if(tokenizer.get_next() != "endaction") {
				cout << "action parse error" << name << endl;
				return false;
			}

			if(act_root != NULL) {
				action_table[name] = act_root;
			} else {
				cout << "action generation error" << name << endl;
				return false;
			}
		} else if(s == "reward") {
			if(!parse_reward()) {
				cout << "failed on reward function" << endl;
				return false;
			}
		} else if(s == "horizon") {
			string s = tokenizer.get_next();
			horizon = atoi(s.c_str());
		} else if(s == "discount") {
			string s = tokenizer.get_next();
			discount = atof(s.c_str());
		} else if(s == "tolerance") {
			string s = tokenizer.get_next();
			tolerance = atof(s.c_str());
		} else {
			cout << "UNKNOWN TOKEN -	" << s << endl;
		}

		s = tokenizer.get_next();
	}

	return true;
}
