#include <iostream>
#include <string>

using namespace std;

#include "kb.h"
#include "../macros.h"

string file = "../../dat/kb/psy_advise.kb";

int main()
{
	knowledge_base_t kb;
	kb.init(file);

	cout << "Testing var assign functions..." << endl;

	vector<string> a = kb.var_assign_greater_eq("PSY215", "Pass"); 
	vector<string> b = kb.var_assign_greater_eq("PSY215", "blurg"); 
	vector<string> c = kb.var_assign_less_eq("PSY215", "Pass"); 
	vector<string> d = kb.var_assign_less_eq("PSY215", "blurg");

	cout << a.size() << " " << b.size() << " " << c.size() << " " << d.size() 
		 << endl;

	FOREACH(i,a)
		cout << *i << endl;
	cout << endl;

	FOREACH(i,b)
		cout << *i << endl;
	cout << endl;
	
	FOREACH(i,c)
		cout << *i << endl;
	cout << endl;
	
	FOREACH(i,d)
		cout << *i << endl;
	cout << endl;

	cout << endl << "Testing goal scenarios..." << endl;

	FOREACH(g,kb.goal_scenarios) {
		cout << g->first << endl;
		FOREACH(v,g->second) {
			cout << "\t" << v->first << endl;
			FOREACH(a, v->second) {
				cout << "\t\t" << *a << endl;
			}
		}
	}

	return 0;
}

