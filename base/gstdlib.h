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

}}
