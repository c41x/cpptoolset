/*
 * granite engine 1.0 | 2006-2014 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: tokenizer.cpp
 * created: 08-02-2014
 *
 * description: String tokenizator utility
 *
 * changelog:
 * - 08-02-2014: file created
 */

#include "tokenizer.h"

namespace granite { namespace base {

void tokenizer::addRule(int id, bool skip, const string &start, const string &exception, const string &end) {
	gassert(id >= 0, "rule id that are lower than 0 are reserved");
	_rules.push_back({id, skip, start, exception, end, end.size() > 0});
}

void tokenizer::removeRule(int id) {
	_rules.erase(std::remove_if(std::begin(_rules), std::end(_rules),
								[id](const rule &r){
									return r.id == id;
								}), std::end(_rules));
}

void tokenizer::tokenize(const string &input, bool copyInput) {
	tokens.clear();

	// copy input string
	const string *pin = &input;
	if(copyInput){
		this->input = input;
		pin = &this->input;
	}
	const string &in = *pin;

	// helpers
	auto contains = [](const string &s, string::const_iterator c) -> bool {
		return s.cend() != std::find(s.cbegin(), s.cend(), *c);
	};

	auto findRule = [this, contains](string::const_iterator c) -> std::vector<rule>::const_iterator {
		return std::find_if(_rules.cbegin(), _rules.cend(),
							[c, contains](const rule &r) -> bool {
								return contains(r.start, c);
							});
	};

	// extract tokens
	bool inToken = false;
	string::const_iterator tokenBegin = in.cbegin(), next;
	std::vector<rule>::const_iterator currentRule;
	for(auto it = in.cbegin(); it != in.cend(); ++it) {
		next = it + 1;
		// find rule if we are not in token
		if(!inToken) {
			currentRule = findRule(it);

			if(currentRule != _rules.cend() || next == in.cend()) {
				if(tokenBegin != it) {
					tokens.push_back({-1, stringRange(tokenBegin, it)});
				}

				tokenBegin = it;
				inToken = true;
				++it;
			}
		}

		// process token
		if(inToken) {
			if(it == in.cend()) {
				if(!currentRule->skip)
					tokens.push_back({currentRule->id, stringRange(tokenBegin, it)});
				it--;
			}
			else if(!currentRule->checkEnd && !contains(currentRule->start, it)) {
				if(!currentRule->skip)
					tokens.push_back({currentRule->id, stringRange(tokenBegin, it)});
				tokenBegin = it--;
			}
			else if(currentRule->checkEnd
					&& contains(currentRule->end, it)
					&& it != in.cbegin()
					&& !contains(currentRule->exception, it - 1)) {
				if(!currentRule->skip)
					tokens.push_back({currentRule->id, stringRange(tokenBegin + 1, it)});
				tokenBegin = next;
			}
			else if(isLineBreak(*it)) {
				if(!currentRule->skip)
					tokens.push_back({currentRule->id, stringRange(tokenBegin, it)});
				tokenBegin = next;
			}
			else continue;
			inToken = false;
		}
	}
}

void tokenizer::clear() {
	_rules.clear();
	tokens.clear();
}

}}
