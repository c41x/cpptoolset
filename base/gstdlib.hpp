/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
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
void mapc(T_ITERATOR begin, T_ITERATOR end, const T_OPERATION &op) {
	for(T_ITERATOR it = begin; it != end; ++it) {
		op(*it);
	}
}

// ELISP mapcar
template <typename T_ITERATOR, typename T_OPERATION, typename T_CONTAINER>
T_CONTAINER mapcar(T_ITERATOR begin, T_ITERATOR end, const T_OPERATION &op) {
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
T_AGG mapconcat(T_ITERATOR begin, T_ITERATOR end, const T_OPERATION &op, T_AGG separator) {
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
T_ITERATOR find_if_backwards(T_ITERATOR begin, T_ITERATOR end, const T_OPERATION &pred) {
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
T_ITERATOR backwards_until(T_ITERATOR begin, T_ITERATOR end, const T_OPERATION &pred) {
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

// deletes range [del_begin, del_end), inserts [copy_begin, copy_end) instead
template <typename T_CONTAINER, typename T_ITERATOR>
T_ITERATOR remove_copy(T_CONTAINER &container,
					   T_ITERATOR del_begin, T_ITERATOR del_end,
					   T_ITERATOR copy_begin, T_ITERATOR copy_end) {
	size_t delSize = std::distance(del_begin, del_end);
	size_t copySize = std::distance(copy_begin, copy_end);
	size_t firstPartSize = std::min(delSize, copySize);

	// copy first part
	std::copy(copy_begin, copy_begin + firstPartSize, del_begin);

	// return pos if sizes were equal
	if (copySize == delSize)
		return del_begin;

	// remember iterator position
	size_t delPosition = std::distance(container.begin(), del_begin);

	// allocate more space and copy second part, or...
	if (copySize > delSize)
		container.insert(del_end, copy_begin + delSize, copy_end);
	else
		// ... remove unused part
		container.erase(del_begin + firstPartSize, del_end);

	// return valid iterator
	return container.begin() + delPosition;
}

// erase_if - remove_if equivalent for associative containers
template <typename T_CONTAINER, typename T_OP>
void erase_if(T_CONTAINER &container, const T_OP &pred) {
	for (auto it = container.begin(); it != container.end(); ) {
		if (pred(*it))
			it = container.erase(it);
		else ++it;
	}
}

// overlapping - safe copy, returns iterator to last element
template <typename T_INPUT, typename T_OUTPUT>
T_OUTPUT copy_safe(T_INPUT begin, T_INPUT end, T_OUTPUT where) {
	if (where < begin)
		return std::copy(begin, end, where);
	std::copy_backward(begin, end, where);
	return where + std::distance(begin, end);
}

// append two vectors
template <typename T_INPUT, typename T_OUTPUT>
T_OUTPUT &append(T_OUTPUT &out, const T_INPUT &in) {
	out.insert(out.end(), in.begin(), in.end());
	return out;
}

}}
