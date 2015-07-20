/*
 * granite engine 1.0 | 2006-2014 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: tokenizer.*
 * created: 08-02-2014
 *
 * description: String tokenizator utility
 *
 * changelog:
 * - 08-02-2014: file created
 * - 27-08-2014: immediate mode
 */

#pragma once

#include "includes.h"
#include "string.h"

namespace granite { namespace base {

class tokenizer;

struct token {
	int id;
	stringRange value;
	operator bool() { return id != -2; }
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
	std::vector<rule>::const_iterator _rule;
	string::const_iterator _begin, _i, _end;

public:
	string input;

    tokenizer() {}
    ~tokenizer() {}

	void addRule(int id, bool skip, const string &start, const string &exception = "", const string &end = "");
	void removeRule(int id);
	token begin(const string &input, bool copyInput = true);
	token next();
	bool hasInputCopy() const { return input.size() > 0; }
	void clear();
};

}}
