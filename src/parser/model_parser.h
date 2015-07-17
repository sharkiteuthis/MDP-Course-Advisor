#ifndef MODEL_PARSER_H
#define MODEL_PARSER_H

#include "tokenizer.h"
#include <vector>
#include <string>
#include <map>
#include "parser_types.h"

class model_parser_t
{
public:
	model_parser_t(string file);
	bool parse();

	map<string, variable_t*> var_table;
	map<string, dd_node_t*> dd_table;
	map<string, action_t*> action_table;
	vector<dd_node_t*> reward_dds;

	double discount;
	double tolerance;
	int horizon;
private:
	bool parse_vars();
	dd_node_t * parse_dd();
	action_t * parse_action();
	void print_dd(dd_node_t * root, string prefix);
	bool parse_reward();
	
	tokenizer_t tokenizer;
};

#endif
