stream::stream(size_t size) {
	if (size > 0)
		_mem.resize(size);
	_pos = 0;
}

stream::~stream() {}

size_t stream::read(void *data, size_t size) {
	size_t n = std::min(_mem.size() - _pos, size);
	memcpy(data, &_mem[_pos], n);
	_pos += n;
	return n;
}

void stream::write(const void *data, size_t size) {
	size_t need = std::max(_mem.size(), _pos + size);
	_mem.resize(need);
	memcpy(&_mem[_pos], data, size);
	_pos += size;
}

void stream::setPosFromBegin(size_t bytes) {
	_pos = bytes;
}

void stream::setPosFromEnd(size_t bytes) {
	gassert(_mem.size() >= bytes, "index out of range");
	_pos = _mem.size() - bytes;
}

void stream::setPosOffset(size_t bytes) {
	gassert(_pos + bytes < _mem.size(), "index out of range");
	_pos += bytes;
}

size_t stream::getPos() const {
	return _pos;
}

std::vector<uint8> &&stream::take() {
	return std::move(_mem);
}

template <typename T> size_t stream::read(T &out) {
	return read(&out, sizeof(out));
}

template <typename T> void stream::write(const T &in) {
	write(&in, sizeof(in));
}

// specializations for writing strings (writing size, then string itself)
template <> size_t stream::read(string &s) {
	size_t len;
	size_t r = read<size_t>(len);
	s.resize(len);
	return read(&s[0], len) + r;
}

template <> void stream::write(const string &s) {
	write<size_t>(s.size());
	write(&s[0], s.size());
}
