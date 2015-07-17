#ifndef CASE_BASE_H
#define CASE_BASE_H

#include <string>
#include <vector>
#include <map>
#include <set>

//A case in this implementation contains
//	a unique id (from the data file)
// 	a course label (e.g. CS100)
// 	a time (year + semester)
//	a grade
//
//This is distinnt from the use of 'case' in the paper, which
// refers to two case_t objects sharing an id and ordered in time.
//At a glance:
//	case_t: 			action->state 
//						(take CS100 -> get "A")
//
//  paper definition: 	action->state->action->state
//						(take CS100 -> get "A" -> take CS115 -> get "A"
//
class case_t {
public:
	int id;
	string course;
	int year;
	string semester;
	string grade;
};

//the case base - just vector with associated functions, but could
// easily be made more efficient by using a multimap or map of lists
// indexed by id
class case_base_t
{
public:
	bool init(string filename);

	//Finds the population of cases which match the class described by
	// the action, to_take. 
	//Returns:
	//	dist_course - the name of the course associated with the action 
	//	total_cases - size of the population
	//	map<string,double> - a map of grades to their proportions in the population
	//		e.g.	
	//			"Good" -> 0.5
	//			"Pass" -> 0.3
	//			"Fail" -> 0.2
	//
	map<string, double> get_naive_dist(string to_take, string &dist_course,
											int &total_cases);
	
	//Finds the population of cases which match the class described by
	// the action, to_take, and which were taken by a student who has
	// also taken the course course_taken previously and gotten a grade
	// which appears in the grades vector
	//
	//This is a conditional probability distribution for to_take conditioned
	// on course_taken and the provided grades
	//
	//An empty map will be returned if there are fewer matches than the provided
	// cutoff
	//
	//Returns:
	//	map<string,double> - a map of grades to their proportions in the population
	//		e.g.	
	//			"Good" -> 0.5
	//			"Pass" -> 0.3
	//			"Fail" -> 0.2
	//
	map<string,double> get_conditional_dist(string course_taken, vector<string> grades,
												string to_take, int cutoff=0);

	//Attempts to find "careers" (see section 4.3 of the paper) which match the current state and the
	// given objective, and returns information about the number of timesteps between the current state
	// and the satisfaction of the objective.
	//
	//If the parameter next_step_action is supplied, only careers which have the provided action in the
	// timestep after satisfaction of the current state will be returned.
	//
	//An empty map will be returned if there are no matches
	//
	//Returns:
	//	map<int,double> - a map of timestep lengths to proportion of careers satisfying the paramters
	//						e.g.	
	//							1 -> 0.1
	//							2 -> 0.3
	//							3 -> 0.4
	//							4 -> 0.2
	//
	map<int,double> get_times_to_scenario(map<string,string> cur_state,
								map<string, vector<string> > objective, 
								string * next_step_action);
private:
	//Fetches all of the case_t pairs which match the input parameters 
	/// as described by get_conditional_dist
	//
	//NB: the 'cases' referred to here match the description of cases
	// from the paper i.e. action->state->action->state sequences
	// but the return value is the vector of case_t (action->state) 
	// for the second action in that sequence
	vector<case_t> get_conditioned_cases(string taken, set<string> grade, string to_take);

	//Functions for translating incoming course and action names into
	// strings that the case_base can easily match
	// i.e. the grade vector ["Good","Pass"] becomes the vector ["A","B","C"]
	string translate_var_name(string v);
	set<string> translate_var_values(vector<string> vs);
	string translate_action_name(string a);
	string detranslate_grade(string g);
	string detranslate_course_name(string v);

	vector<case_t> cases;
};

#endif
