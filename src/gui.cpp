#include <iostream>
#include <ncurses.h>
#include <menu.h>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>

using namespace std;

#include "macros.h"
#include "parser/model_parser.h"
#include "parser/policy_parser.h"
#include "menu_maker.h"
#include "mdp_math.h"
#include "explainer.h"

///////////////////////////////////////////////////////////
// FILE GLOBALS
///////////////////////////////////////////////////////////
//
//PSY_CS_SWITCH
//
string model_file 	= "../dat/models/psych_advise.spudd";
string policy_file 	= "../dat/policies/psych_advise.policy";
//string model_file 	= "../dat/models/extended_advise.spudd";
//string policy_file 	= "../dat/policies/extended_advise.policy";

model_parser_t 	mp(model_file);
policy_parser_t 	pp(policy_file);

dd_root_t * policy_root = NULL;

explainer_t * explainer_ptr;

///////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
///////////////////////////////////////////////////////////

string draw_state_action_screen(map<string,string> cur_state, 
								vector<string> actions);
map<string,string> draw_next_states_screen(string action, 
											map<string,string> cur_state);
void run_gui();
void draw_long_string(WINDOW* win, string s, int nlines, int ncols); 

map<string, double> get_action_values(map<string,string> state);

static void finish_curses(int sig) {
	endwin();

	exit(sig);
}

static void init_curses()
{
	signal(SIGINT, finish_curses);

	initscr();
	keypad(stdscr, TRUE);
	nonl();
	cbreak();
	noecho();

	getmaxyx(stdscr, LINES, COLS);

	if(has_colors()) {
		start_color();
        init_pair(1, COLOR_RED,     COLOR_BLACK);
        init_pair(2, COLOR_GREEN,   COLOR_BLACK);
        init_pair(3, COLOR_YELLOW,  COLOR_BLACK);
        init_pair(4, COLOR_BLUE,    COLOR_BLACK);
        init_pair(5, COLOR_CYAN,    COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(7, COLOR_WHITE,   COLOR_BLACK);
	}
}

///////////////////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////////////////

int main()
{
	if(!mp.parse()) {
		cout << "Error parsing model" << endl;
		return 0;
	}

	policy_root = pp.parse();

	if(!policy_root) {
		cout << "Error parsing policy" << endl;
		return 0;
	}

	explainer_ptr = new explainer_t(&mp, policy_root);

	init_curses();

	run_gui();
	
	finish_curses(0);
	return 0;
}

///////////////////////////////////////////////////////////
// LOCAL FUNCTION DEFS
///////////////////////////////////////////////////////////

string draw_state_action_screen(map<string,string> cur_state, 
								vector<string> actions)
{
	int rec_screen_lines = 15;

	//split screen, one side just shows state (var assignments), the
	// other side is a menu to select the next action
	WINDOW * state_win = newwin(LINES-rec_screen_lines, COLS/2, 0,0);
	WINDOW * rec_win   = newwin(rec_screen_lines, COLS, LINES-rec_screen_lines, 0);
	box(state_win, 0,0);
	box(rec_win, 0,0);

	string rec_str = explainer_ptr->get_recommendation(cur_state);
	
	draw_long_string(rec_win, rec_str, 11, COLS);

	menu_maker_t * action_menu = new menu_maker_t(actions, actions, 
													LINES-rec_screen_lines, COLS/2, 0, COLS/2);

	int line = 1;
	FOREACH(var, cur_state) {
		mvwprintw(state_win, line, 1, "%s = %s", var->first.c_str(), 
												var->second.c_str());
		line++;
	}

	wrefresh(state_win);
	wrefresh(rec_win);

	int ndx_choice = action_menu->run();
	
	delete action_menu;
	delwin(state_win);
	delwin(rec_win);

	return actions[ndx_choice];
}

map<string,string> draw_next_states_screen(string action, 
											map<string,string> cur_state)
{
	// here we have to make a menu... a menu of next states with 
	// associated probabilities
	vector<map<string,string> > next_states;
	vector<double> probabilities;

	int err =  get_next_states(mp, cur_state, action, next_states,
									probabilities);
	
	if(err) {
		endwin();
		cout << "Error generating next states: " << err << endl;
		exit(1);
	}

	//Turn everything into strings to put in a menu
	vector<string> next_state_str_deltas;
	vector<string> str_probs;
	
	REP(i, next_states.size()) {
		string str_delta = "";
		
		FOREACH(a, next_states[i]) {
			string var_name = a->first;
			string next_val = a->second;
			
			if(cur_state.find(var_name) == cur_state.end()) {
				endwin();
				cout << "Could not find var " << var_name << " in state!" 
					 << endl;
				exit(1);
			}

			if(cur_state[var_name] != next_val) {
				if(str_delta != "")
					str_delta += ",";
				str_delta += var_name;
				str_delta += "=";
				str_delta += next_val;
			}
		}

		if(str_delta == "")
			str_delta = "UNCHANGED";

		next_state_str_deltas.push_back(str_delta);

		char tmp[16];
		sprintf(tmp, "%.2f", probabilities[i]);
		str_probs.push_back(tmp);
	}

	menu_maker_t * state_menu = new menu_maker_t(str_probs, 
								next_state_str_deltas, LINES, COLS, 0, 0);
	int ndx_choice = state_menu->run();
	delete(state_menu);

	return next_states[ndx_choice];
}

void run_gui()
{
	//initialize state
	map<string,string> cur_state;
	FOREACH(var, mp.var_table) {
		cur_state[var->first] = var->second->values[0];
	}

/*
	cur_state["GPA"] = "Pass";
	cur_state["CS115"] = "Good";
	cur_state["CS215"] = "Pass";
	cur_state["CS216"] = "Pass";
	cur_state["MA114"] = "Pass";
*/

	vector<string> actions;

	FOREACH(action, mp.action_table) {
		actions.push_back(action->first);
	}

	while(1) {
		string action = draw_state_action_screen(cur_state, actions);
		cur_state = draw_next_states_screen(action, cur_state);
	}
}

void draw_long_string(WINDOW* win, string s, int nlines, int ncols)
{
	//find split points
	int maxwidth = ncols - 4;
	int start = 0;
	int len_left = s.length();
	int line = 1;
	
	while(len_left) {
		int width;

		if(len_left < maxwidth) {
			width = len_left;
		} else {
			width = maxwidth;
			
			for( ; width > 0; width--) {
				if(s[width] == ' ') { 
					width++; 
					break;
				}
			}
		}
		
		wmove(win, line, 2);

		for(int i = start; i < start+width; ++i) {
			waddch(win, s[i]);
		}

		line++;
		start += width;
		len_left -= width;
	}

	wrefresh(win);
}

