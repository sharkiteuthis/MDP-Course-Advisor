#include <fstream>
#include <string>

using namespace std;

#include "tokenizer.h"

tokenizer_t::tokenizer_t(string infile) {
	ifstream in(infile.c_str());

	text = "";
	text_pos = 0;

	string s;
	while(getline(in, s)) {
		if(s.size() >= 2 && s[0] == '/' && (s[1] == '/' || s[1] == '*'))
			continue;
	
		text += s;
	}

	in.close();
}

string tokenizer_t::get_next() {
	string tok("");
	bool break_and_inc = false;

	while(text_pos < text.size()) {
		char c = text[text_pos++];			

		if(isspace(c)) {
			
			if(tok != "") {
				return tok;
			}

		} else if(c == ')' || c == '(') {
			
			if(tok == "")
				tok.push_back(c);
			else
				text_pos--;

			return tok;
		} else {
			tok.push_back(c);
		}
	}

	return tok;
}

