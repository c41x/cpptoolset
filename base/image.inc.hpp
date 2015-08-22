image::operator const_stream() {
	return const_stream(data.data(), data.size());
}

inline image toImage(const_stream s) {
	image r;
	toImage(s, r);
	return r;
}

inline stream fromImage(const image &i, imageCodec codec) {
	stream s;
	fromImage(i, s, codec);
	return s;
}
