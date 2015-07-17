#include <iostream>
#include <set>
#include <string>

using namespace std;

#include "../macros.h"
#include "case_base.h"

string cb_input_file = "../../dat/casebase/psy_casebase.txt";

void test1(case_base_t &cb)
{
	cout << "Testing distribution retrieval:" << endl;

	int total = 0;
	string s;
	vector<string> grades;
	grades.push_back("Good");

	//
	map<string, double> dist = 
			cb.get_conditional_dist("PSY100", grades, "take_PSY215", 20);

	cout << "PSY100->PSY215:" << endl;
	FOREACH(d,dist) {
		cout << "\t" << d->first << "\t" << d->second << endl;
	}

	//
	dist = cb.get_naive_dist("take_PSY215", s, total);

	cout << "PSY215 naive:" << endl;
	FOREACH(d,dist) {
		cout << "\t" << d->first << "\t" << d->second << endl;
	}

	//
	dist = cb.get_conditional_dist("PSY215", grades, "take_PSY216", 20);

	cout << "PSY215->PSY216:" << endl;
	FOREACH(d,dist) {
		cout << "\t" << d->first << "\t" << d->second << endl;
	}
	
	//
	dist = cb.get_naive_dist("take_PSY216", s, total);

	cout << "PSY216 naive:" << endl;
	FOREACH(d,dist) {
		cout << "\t" << d->first << "\t" << d->second << endl;
	}

	//
	dist = cb.get_conditional_dist("PSY215", grades, "take_PSY311", 20);

	cout << "PSY215->PSY311:" << endl;
	FOREACH(d,dist) {
		cout << "\t" << d->first << "\t" << d->second << endl;
	}
	
	//
	dist = cb.get_naive_dist("take_PSY311", s, total);

	cout << "PSY311 naive:" << endl;
	FOREACH(d,dist) {
		cout << "\t" << d->first << "\t" << d->second << endl;
	}
}

void test2(case_base_t &cb)
{
	cout << "Testing time to scenario:" << endl;
	map<string,string> cur_state;
	map<string,vector<string> > goal_state;
	vector<string> grades;
	grades.push_back("Good");
	grades.push_back("Pass");

	cur_state["PSY100"] = "Good";
	goal_state["PSY100"] = grades;
	goal_state["PSY215"] = grades;
	goal_state["PSY216"] = grades;
	goal_state["PSY311"] = grades;
	goal_state["PSY312"] = grades;
	goal_state["PSY313"] = grades;
	goal_state["PSY314"] = grades;

	map<int,double> tts = cb.get_times_to_scenario(cur_state,goal_state,NULL);
	double average = 0;

	FOREACH(t,tts) {
		cout << t->first << "\t" << t->second << endl;
		average += t->first * t->second;
	}

	cout << "Average: " << ROUND_TO_INT(average) << endl;



	cur_state.clear();
	cur_state["PSY100"] = "Good";
	cur_state["PSY215"] = "Good";
	cur_state["PSY216"] = "Good";
	cur_state["PSY311"] = "Good";
	cur_state["PSY312"] = "Good";
	cur_state["PSY313"] = "Good";
	tts = cb.get_times_to_scenario(cur_state,goal_state,NULL);
	average = 0;

	FOREACH(t,tts) {
		cout << t->first << "\t" << t->second << endl;
		average += t->first * t->second;
	}

	cout << "Average: " << ROUND_TO_INT(average) << endl;




	cout << "Testing conditional time to scenario:" << endl;

	cur_state.clear();
	cur_state["PSY100"] = "Good";
	string act = "take_PSY215";
	tts = cb.get_times_to_scenario(cur_state,goal_state,&act);
	average = 0;

	FOREACH(t,tts) {
		cout << t->first << "\t" << t->second << endl;
		average += t->first * t->second;
	}

	cout << "Average: " << ROUND_TO_INT(average) << endl;
}

int main()
{
	//this is how you define a case base object...
	case_base_t cb;

	//and this is how you initialize one
	if(!cb.init(cb_input_file)) {
		cout << "Error initializing case base!" << endl;
		return 0;
	}

	test1(cb);
	test2(cb);

	return 0;
}

