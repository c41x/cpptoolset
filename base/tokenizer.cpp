/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: tokenizer.cpp
 * created: 08-02-2014
 *
 * description: String tokenizator utility
 *
 * changelog:
 * - 08-02-2014: file created
 * - 27-08-2014: immediate mode
 */

#include "tokenizer.hpp"

namespace granite { namespace base {

void tokenizer::addRule(int id, bool skip, const string &start, const string &exception, const string &end) {
	gassert(id >= 0, "rule ids that are lower than 0 are reserved");
	_rules.push_back({id, skip, start, exception, end, end.size() > 0});
}

void tokenizer::removeRule(int id) {
	_rules.erase(std::remove_if(std::begin(_rules), std::end(_rules),
								[id](const rule &r){
									return r.id == id;
								}), std::end(_rules));
}

token tokenizer::begin(const string &iinput, bool copyInput) {
	if(copyInput) {
		input = iinput;
		_begin = _i = input.cbegin();
		_end = input.cend();
	}
	else {
		_begin = _i = iinput.cbegin();
		_end = iinput.cend();
	}
	return next();
}

token tokenizer::next() {
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

	// reset state
	string::const_iterator tokenBegin = _i;
	_rule = _rules.cend();

	while(_i != _end) {
		// find rule if not in token
		if(_rule == _rules.cend()) {
			_rule = findRule(_i);

			// flush if found new token and token not empty
			if(_rule != _rules.cend() && _i != tokenBegin)
				return {-1, stringRange(tokenBegin, _i)};
		}

		// we are in token?
		if(_rule != _rules.cend()) {
			if(!_rule->checkEnd && _i != tokenBegin) {
				// reset loop otherwise return token
				if(_rule->skip) {
					tokenBegin = _i;
					_rule = _rules.cend();
					continue;
				}
				else return {_rule->id, stringRange(tokenBegin, _i)};
			}
			else if(_rule->checkEnd && _i != tokenBegin // check end conditions including escape chars
					&& contains(_rule->end, _i)
					&& _i - 1 != _begin && !contains(_rule->exception, _i - 1)) {
				// reset loop otherwise return token without delimiters
				if(_rule->skip) {
					tokenBegin = _i;
					_rule = _rules.cend();
					continue;
				}
				else return {_rule->id, stringRange(tokenBegin + 1, _i++)};
			}
		}
		++_i;
	}

	// return last valid token or invalid token (end of parsing)
	if(_i != tokenBegin) {
		if(_rule != _rules.cend()) {
			if (!_rule->skip)
				return {_rule->id, stringRange(tokenBegin, _i)};
		}
		else return {-1, stringRange(tokenBegin, _i)};
	}

	// return end of parsing token
	return {-2, stringRange(_begin, _end)};
}

void tokenizer::clear() {
	_rules.clear();
	input.clear();
}

}}
