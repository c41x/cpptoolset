/*
 * granite engine 1.0 | 2006-2014 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: Granite standard lib
 * created: 15-11-2014
 *
 * description: random helper functions and algorithms
 *
 * changelog:
 * - 15-11-2014: file created
 */

#pragma once

namespace granite { namespace base {

// ELISP mapc
template <typename T_ITERATOR, typename T_OPERATION>
void mapc(T_ITERATOR begin, T_ITERATOR end, T_OPERATION op) {
	for(T_ITERATOR it = begin; it != end; ++it) {
		op(*it);
	}
}

// ELISP mapcar
template <typename T_ITERATOR, typename T_OPERATION, typename T_CONTAINER>
T_CONTAINER mapcar(T_ITERATOR begin, T_ITERATOR end, T_OPERATION op) {
	T_CONTAINER v;
	for(T_ITERATOR it = begin; it != end; ++it)
		v.push_back(op(*it));
	return v;
}

// concat
template <typename T_ITERATOR, typename T_AGG>
T_AGG concat(T_ITERATOR begin, T_ITERATOR end, T_AGG separator) {
	T_AGG r;

	// return "" if no elements
	if(begin == end)
		return r;

	// return *begin if one element
	if(std::distance(begin, end) < 2)
		return *begin;

	// glue all elements
	for(T_ITERATOR it = begin; it != end; ++it) {
		r += *it;
		if(it != std::prev(end))
			r += separator;
	}
	return r;
}

// ELISP mapconcat
template <typename T_ITERATOR, typename T_OPERATION, typename T_AGG>
T_AGG mapconcat(T_ITERATOR begin, T_ITERATOR end, T_OPERATION op, T_AGG separator) {
	T_AGG r;

	// return "" if no elements
	if(begin == end)
		return r;

	// return *begin if one element
	if(std::distance(begin, end) < 2)
		return op(*begin);

	// glue all elements
	for(T_ITERATOR it = begin; it != end; ++it) {
		r += op(*it);
		if(it != std::prev(end))
			r += separator;
	}
	return r;
}

// same as std::find_if but performes search backward
// using reverse iterators is annoying and std::end is overkill
template <typename T_ITERATOR, typename T_OPERATION>
T_ITERATOR find_if_backwards(T_ITERATOR begin, T_ITERATOR end, T_OPERATION pred) {
	// empty sequence
	if (begin == end)
		return end;

	// search for first match
	for (auto it = end - 1;; --it) {
		if (pred(*it))
			return it;
		if (it == begin)
			break;
	}

	// not found
	return end;
}

// search for last element, break if pred returns false
// returns last element [e] that [pred(e)] == true
template <typename T_ITERATOR, typename T_OPERATION>
T_ITERATOR backwards_until(T_ITERATOR begin, T_ITERATOR end, T_OPERATION pred) {
	// empty sequence
	if (begin == end)
		return end;

	// search until pred returns false
	auto last_pass = end;
	for (auto it = end - 1;; --it) {
		if (!pred(*it))
			last_pass = it;
		else return last_pass;
		if (it == begin)
			break;
	}

	// not found
	return end;
}

}}
