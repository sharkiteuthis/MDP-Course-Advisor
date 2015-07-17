#ifndef EXPLAINER_H
#define EXPLAINER_H

#include <string>
#include <map>

#include "case_base/case_base.h"
#include "kb/kb.h"

class dd_node_t;
class model_parser_t;

class explainer_t
{
public:
	explainer_t(model_parser_t * pmp, dd_root_t * root);
	string get_recommendation(map<string,string> state);
private:
	string generate_case_base_explain(string act, map<string,string> state);
	string generate_mdp_explain(string act, map<string,string> state);
	string generate_time_to_goal_explain(string act, map<string,string> state);

	dd_root_t * 		policy_root;
	model_parser_t * 	mp_ptr;
	case_base_t 		cb;
	knowledge_base_t	kb;
};

#endif
