#include "menu_maker.h"
#include <stdlib.h>
#include "macros.h"

menu_maker_t::menu_maker_t(vector<string> &items, vector<string> &descriptions,
						int lines, int cols, int start_line, int start_col) {
	num_choices = items.size();
	
	menu_win = newwin(lines, cols, start_line, start_col);
	menu_sub_win = derwin(menu_win, lines-2, cols - 2, 1, 1);
	box(menu_win, 0,0);

	keypad(menu_win, TRUE);

	//set up menu
	menu_items = (ITEM**)calloc(num_choices+1, sizeof(ITEM*));
	REP(i, num_choices)
		menu_items[i] = new_item(items[i].c_str(), descriptions[i].c_str());

	menu_items[num_choices] = new_item(NULL, NULL);

	menu = new_menu((ITEM**)menu_items);

	set_menu_win(menu, menu_win);
	set_menu_sub(menu, menu_sub_win);
	set_menu_mark(menu, " * ");
}

menu_maker_t::~menu_maker_t() {
	unpost_menu(menu);
	free_menu(menu);
	REP(i,num_choices) {
		free_item(menu_items[i]);
	}

	delwin(menu_sub_win);
	delwin(menu_win);
}

int menu_maker_t::run() {
	int c;
	post_menu(menu);
	wrefresh(menu_win);

	int cur_ndx = -1;

	while(cur_ndx == -1 && (c = wgetch(menu_win))) {
		switch(c) {
			case KEY_UP:
				menu_driver(menu, REQ_UP_ITEM);
				break;
			case KEY_DOWN:
				menu_driver(menu, REQ_DOWN_ITEM);
				break;
			case 0xD:	//the enter key...
				cur_ndx = item_index(current_item(menu)); 
		}
		wrefresh(menu_win);
	}
	
	return cur_ndx;
}

