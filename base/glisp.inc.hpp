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

bool lisp::validate(cell_t) { return true; }

template <typename... Args> bool lisp::validate(cell_t c, cell::type_t tt, Args... t) {
    return c->type == tt && lisp::validate(c + 1, t...);
}

template <typename... Args> bool lisp::validate(cell_t c, const cell &tt, Args... t) {
	bool equal;
	if (c->type == tt.type) {
		if (c->type == cell::typeInt ||
			c->type == cell::typeList ||
			c->type == cell::typeDetach) equal = c->i == tt.i;
		else if (c->type == cell::typeString || c->type == cell::typeIdentifier) equal = c->s == tt.s;
		else if (c->type == cell::typeFloat) equal = c->f == tt.f;
		else if (c->type == cell::typeInt64) equal = c->ii == tt.ii;
	}
	else equal = false;
    return equal && lisp::validate(c + 1, t...);
}
