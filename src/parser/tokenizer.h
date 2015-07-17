#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>

class tokenizer_t {
public:
	tokenizer_t(string infile);
	string get_next();
private:
	string text;
	int text_pos;
};

#endif
