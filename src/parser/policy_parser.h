#ifndef POLICY_PARSER_H
#define POLICY_PARSER_H

#include <string>
#include <vector>
#include <map>

#include "tokenizer.h"
#include "parser_types.h"

class policy_parser_t
{
public:
	//init the parser with a filename...
	policy_parser_t(string file);

	//do the actual parse, returning a pointer to the root of the decision diagram
	dd_root_t * parse();
private:
	void set_var_assignment(string s, vector<pair<string,string> > &assignment);
	bool insert_leaf(dd_root_t * root, vector<pair<string,string> > assignment, 
						int assign_ndx, double value, string action);
	void print_tree(dd_base_t * root, int lvl = 0);

	tokenizer_t tokenizer;
};

#endif
