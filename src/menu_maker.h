#include <ncurses.h>
#include <menu.h>
#include <string>
#include <vector>

using namespace std;

class menu_maker_t {
public:
	menu_maker_t(vector<string> &items, vector<string> &descriptions,
					int lines, int cols, int start_line, int start_col);
	~menu_maker_t();
	int run();

private:
	ITEM** menu_items;
	MENU* menu;
	int num_choices;

	WINDOW * menu_win;
	WINDOW * menu_sub_win;
};
