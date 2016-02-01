// specializations for writing one cell
template <> inline size_t stream::read(cell &e) {
	size_t readed = 0;
	uint type;
	readed += read(type);
	e.type = (cell::type_t)type;
	switch (type) {
		case cell::typeInt: readed += read(e.i); break;
		case cell::typeFloat: readed += read(e.f); break;
		case cell::typeVector: readed += read(e.v4); break;
		case cell::typeString:
		case cell::typeIdentifier: readed += read(e.s); break;
		case cell::typeList: readed += read(e.i) + read(e.j); break;
		case cell::typeInt64: readed += read(e.ii); break;
	}
	return readed;
}

template <> inline void stream::write(const cell &e) {
	write<uint>(e.type);
	switch (e.type) {
		case cell::typeInt: write(e.i); break;
		case cell::typeFloat: write(e.f); break;
		case cell::typeVector: write(e.v4); break;
		case cell::typeString:
		case cell::typeIdentifier: write(e.s); break;
		case cell::typeList: write(e.i); write(e.j); break;
		case cell::typeInt64: write(e.ii); break;
		case cell::typeDetach: gassertl(false, "could not write detached cell: data invalid."); break;
	}
}

/*
// from/to stream conversions
template <> inline stream &toStream(const cells_t &c, stream &s) {
	s.expand(c.size() * sizeof(cell)); // just estimation
	s.write<uint64>(c.size());
	for (const cell &e : c)
		s.write(e);
	return s;
}

template <> inline cells_t &fromStream(stream &s, cells_t &c) {
	uint64 count = 0;
	cell e;
	s.read(count);
	c.reserve(c.size() + count);
	while (count-- > 0) {
		c.push_back(cell());
		s.read(c.back());
	}
	return c;
}
*/

//- validate
namespace detail {
inline bool compare(const cell &c, const cell &tt) {
	if (c.type == tt.type) {
		if (c.type == cell::typeInt ||
			c.type == cell::typeList ||
			c.type == cell::typeDetach) return c.i == tt.i;
		else if (c.type == cell::typeString || c.type == cell::typeIdentifier) return c.s == tt.s;
		else if (c.type == cell::typeFloat) return c.f == tt.f;
		else if (c.type == cell::typeInt64) return c.ii == tt.ii;
	}
	return false;
}

inline bool countedValidate(int32, cell_t) { return true; }

template <typename T, typename... Args> bool countedValidate(int32 itemsToCheck, cell_t c, T tt, Args... t) {
	if (itemsToCheck > 0)
		return lisp::validate(c, tt) && countedValidate(itemsToCheck - 1, c + 1, t...);

	return true; // could not contunue validating! needs some special tag for list end
}
}

bool lisp::validate(cell_t) { return true; }

template <typename... Args> bool lisp::validate(cell_t c, cell::type_t tt, Args... t) {
    return c->type == tt && lisp::validate(c + 1, t...);
}

template <typename... Args> bool lisp::validate(cell_t c, const cell &tt, Args... t) {
    return detail::compare(*c, tt) && lisp::validate(c + 1, t...);
}

template <typename... Args> bool lisp::validate(cell_t c, const cell::listRange &tt, Args... t) {
    return c->type == cell::typeList
		&& c->i >= tt.min
		&& (tt.max == -1 || c->i <= tt.max)
		&& detail::countedValidate(c->i, c + 1, t...);
}

template <typename... Args> bool lisp::validate(cell_t c, const cell::anyOf &tt, Args... t) {
    return (c->type == tt.t1 || c->type == tt.t2) && lisp::validate(c + 1, t...);
}

//- validateStr
string lisp::validateStr(cell::type_t tt) {
	switch (tt) {
		case cell::typeIdentifier: return "identifier";
		case cell::typeInt: return "int";
		case cell::typeInt64: return "int64";
		case cell::typeFloat: return "float";
		case cell::typeVector: return "vector";
		case cell::typeString: return "string";
		case cell::typeList: return "list";
		default: return "?";
	}
}

string lisp::validateStr(const cell &tt) {
	switch (tt.type) {
		case cell::typeIdentifier: return strs("identifier<", tt.s, ">");
		case cell::typeInt: return strs("int<", tt.i, ">");
		case cell::typeInt64: return strs("int64<", tt.ii, ">");
		case cell::typeFloat: return strs("float<", tt.f, ">");
		case cell::typeVector: return strs("vector<", vec4f(tt.v4), ">");
		case cell::typeString: return strs("string<", tt.s, ">");
		case cell::typeList: return strs("list<", tt.i, ">");
		default: return "?";
	}
}

string lisp::validateStr(const cell::listRange &tt) {
	return strs("list<", tt.min, ", ", tt.max, ">");
}

string lisp::validateStr(const cell::anyOf tt) {
	return strs(validateStr(tt.t1), "|", validateStr(tt.t2));
}

template <typename... Args, typename T> string lisp::validateStr(T tt, Args... t) {
	return strs(validateStr(tt), " ", validateStr(t...));
}
