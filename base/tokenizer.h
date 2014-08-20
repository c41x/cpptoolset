/*
 * granite engine 1.0 | 2006-2014 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: tokenizer.*
 * created: 08-02-2014
 *
 * description: String tokenizator utility
 *
 * changelog:
 * - 08-02-2014: file created
 */

#pragma once

#include "includes.h"
#include "string.h"

namespace granite { namespace base {

class tokenizer;

struct token {
	int id;
	stringRange value;
};

class tokenizer {
	struct rule{
		int id;
		bool skip;
		string start;
		string exception;
		string end;
		bool checkEnd;
	};
	std::vector<rule> _rules;

public:
	std::vector<token> tokens;
	string input;

    tokenizer() {}
    ~tokenizer() {}

	void addRule(int id, bool skip, const string &start, const string &exception = "", const string &end = "");
	void removeRule(int id);
	void tokenize(const string &input, bool copyInput = false);
	bool hasInputCopy() const { return input.size(); }
	void clear();
};

}}
