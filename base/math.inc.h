/*
 * granite engine 1.0 | 2006-2013 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com 
 * file: math.inc.h
 * created: 27-10-2013
 * 
 * description: inlines from math.h
 * 
 * changelog:
 * - 27-10-2013: file created
 */

inline matrix::matrix(float i11, float i12, float i13, float i14,
			   float i21, float i22, float i23, float i24,
			   float i31, float i32, float i33, float i34,
			   float i41, float i42, float i43, float i44)
	: x(i11, i12, i13, i14),
	  y(i21, i22, i23, i24),
	  z(i31, i32, i33, i34),
	  t(i41, i42, i43, i44) {}

inline matrix::matrix(const vec &_x, const vec &_y, const vec &_z, const vec &_t)
	: x(_x), y(_y), z(_z), t(_t) {}

inline matrix::matrix(const float *v)
	: x(v), y(v + 4), z(v + 8), t(v + 12) {}

inline matrix &matrix::operator()(float i11, float i12, float i13, float i14,
								  float i21, float i22, float i23, float i24,
								  float i31, float i32, float i33, float i34,
								  float i41, float i42, float i43, float i44) {
	x(i11, i12, i13, i14);
	y(i21, i22, i23, i24);
	z(i31, i32, i33, i34);
	t(i41, i42, i43, i44);
	return *this;
}

inline matrix &matrix::operator()(const vec &_x, const vec &_y, const vec &_z, const vec &_t) {
	x = _x;
	y = _y;
	z = _z;
	t = _t;
	return *this;
}

inline matrix &matrix::operator()(const float *v) {
	x(v); y(v + 4); z(v + 8); t(v + 12);
	return *this;
}

inline matrix &matrix::operator*=(const matrix &m) {
	return *this = *this * m;
}

inline matrix &matrix::operator-=(const matrix &m) {
	x -= m.x; y -= m.y; z -= m.z; t -= m.t;
	return *this;
}

inline matrix &matrix::operator+=(const matrix &m) {
	x += m.x; y += m.y; z += m.z; t += m.t;
	return *this;
}

inline matrix matrix::operator*(const matrix &m) const {
	matrix r;
	vec rl;
	
	rl = x * _mm_shuffle_ps(m.x, m.x, SSE_RSHUFFLE(0, 0, 0, 0));
	rl += y * _mm_shuffle_ps(m.x, m.x, SSE_RSHUFFLE(1, 1, 1, 1));
	rl += z * _mm_shuffle_ps(m.x, m.x, SSE_RSHUFFLE(2, 2, 2, 2));
	rl += t * _mm_shuffle_ps(m.x, m.x, SSE_RSHUFFLE(3, 3, 3, 3));
	r.x = rl;

	rl = x * _mm_shuffle_ps(m.y, m.y, SSE_RSHUFFLE(0, 0, 0, 0));
	rl += y * _mm_shuffle_ps(m.y, m.y, SSE_RSHUFFLE(1, 1, 1, 1));
	rl += z * _mm_shuffle_ps(m.y, m.y, SSE_RSHUFFLE(2, 2, 2, 2));
	rl += t * _mm_shuffle_ps(m.y, m.y, SSE_RSHUFFLE(3, 3, 3, 3));
	r.y = rl;
	
	rl = x * _mm_shuffle_ps(m.z, m.z, SSE_RSHUFFLE(0, 0, 0, 0));
	rl += y * _mm_shuffle_ps(m.z, m.z, SSE_RSHUFFLE(1, 1, 1, 1));
	rl += z * _mm_shuffle_ps(m.z, m.z, SSE_RSHUFFLE(2, 2, 2, 2));
	rl += t * _mm_shuffle_ps(m.z, m.z, SSE_RSHUFFLE(3, 3, 3, 3));
	r.z = rl;
	
	rl = x * _mm_shuffle_ps(m.t, m.t, SSE_RSHUFFLE(0, 0, 0, 0));
	rl += y * _mm_shuffle_ps(m.t, m.t, SSE_RSHUFFLE(1, 1, 1, 1));
	rl += z * _mm_shuffle_ps(m.t, m.t, SSE_RSHUFFLE(2, 2, 2, 2));
	rl += t * _mm_shuffle_ps(m.t, m.t, SSE_RSHUFFLE(3, 3, 3, 3));
	r.t = rl;

	return r;
}

inline matrix matrix::operator+(const matrix &m) const {
	return matrix(x + m.x, y + m.y, z + m.z, t + m.t);
}

inline matrix matrix::operator-(const matrix &m) const {
	return matrix(x - m.x, y - m.y, z - m.z, t - m.t);
}

inline vec matrix::operator*(const vec &v) const {
	vec r;
	r = vec(_mm_shuffle_ps(v, v, SSE_RSHUFFLE(0, 0, 0, 0))) * x;
	r += vec(_mm_shuffle_ps(v, v, SSE_RSHUFFLE(1, 1, 1, 1))) * y;
	r += vec(_mm_shuffle_ps(v, v, SSE_RSHUFFLE(2, 2, 2, 2))) * z;
	r += vec(_mm_shuffle_ps(v, v, SSE_RSHUFFLE(3, 3, 3, 3))) * t;
	return r;
}

inline matrix &matrix::translate(const vec &_t) {
	matrix r;
	r.identity();
	r.setTranslation(_t);
	return *this *= r;
}

inline matrix &matrix::scale(const vec &s) {
	matrix r;
	r.identity();
	r.setScale(s);
	return *this *= r;
}
inline matrix &matrix::rotate(const vec &axis, float angle) {
	matrix r;
	r.identity();
	r.setRotation(axis, angle);
	return *this *= r;
}

inline matrix &matrix::setTranslation(const vec &_t) {
	// 12, 13, 14 - (t)
	static const __m128 maskz = SSE_RMASK(~0, ~0, ~0, 0);
	static const __m128 maskxyz = SSE_RMASK(0, 0, 0, ~0);
	t = _mm_or_ps(_mm_and_ps(t, maskxyz), _mm_and_ps(_t, maskz));
	return *this;
}

inline matrix &matrix::setScale(const vec &s) {
	x = _mm_or_ps(_mm_and_ps(s, SSE_RMASK(~0, 0, 0, 0)), _mm_and_ps(x, SSE_RMASK(0, ~0, ~0, ~0)));
	y = _mm_or_ps(_mm_and_ps(s, SSE_RMASK(0, ~0, 0, 0)), _mm_and_ps(y, SSE_RMASK(~0, 0, ~0, ~0)));
	z = _mm_or_ps(_mm_and_ps(s, SSE_RMASK(0, 0, ~0, 0)), _mm_and_ps(z, SSE_RMASK(~0, ~0, 0, ~0)));
	return *this;
}

inline matrix &matrix::setRotation(const vec &axis, float angle) {
	// axis w must be 0, angle in radians
	const float tc = cosf(angle);
	const vec xmmtc = _mm_set_ss(tc);
	const vec co = _mm_set1_ps(1.f - tc); // 1 - cos, 1 - cos, 1 - cos, 1 - cos
	const vec sA = vec(_mm_set1_ps(sinf(angle))) * axis + _mm_shuffle_ps(xmmtc, xmmtc, SSE_RSHUFFLE(0, 1, 2, 3)); // axisX * sin, axisY * sin, axisZ * sin, cos
	
	vec rx = axis * _mm_shuffle_ps(axis, axis, SSE_RSHUFFLE(0, 0, 0, 0)) * co +
		_mm_shuffle_ps(sA, _mm_or_ps(sA, _mm_setr_ps(0.f, -0.f, 0.f, 0.f)), SSE_RSHUFFLE(3, 2, 1, 0));
	vec ry = vec(_mm_shuffle_ps(axis, axis, SSE_RSHUFFLE(0, 1, 1, 3))) * _mm_shuffle_ps(axis, axis, SSE_RSHUFFLE(1, 1, 2, 3)) * co +
		_mm_shuffle_ps(_mm_or_ps(sA, _mm_setr_ps(0.f, 0.f, -0.f, 0.f)), sA, SSE_RSHUFFLE(2, 3, 0, 1));
	vec rz = vec(_mm_shuffle_ps(axis, axis, SSE_RSHUFFLE(0, 1, 2, 3))) * _mm_shuffle_ps(axis, axis, SSE_RSHUFFLE(2, 2, 2, 3)) * co +
		_mm_shuffle_ps(_mm_or_ps(sA, _mm_setr_ps(-0.f, 0.f, 0.f, 0.f)), sA, SSE_RSHUFFLE(1, 0, 3, 2));

	static const __m128 wMask = SSE_RMASK(~0, ~0, ~0, 0);
	static const __m128 rwMask = SSE_RMASK(0, 0, 0, ~0);
	x = _mm_or_ps(_mm_and_ps(x, rwMask), _mm_and_ps(rx, wMask));
	y = _mm_or_ps(_mm_and_ps(y, rwMask), _mm_and_ps(ry, wMask));
	z = _mm_or_ps(_mm_and_ps(z, rwMask), _mm_and_ps(rz, wMask));
	return *this;
}

inline matrix &matrix::identity() {
	x(1.f, 0.f, 0.f, 0.f);
	y(0.f, 1.f, 0.f, 0.f);
	z(0.f, 0.f, 1.f, 0.f);
	t(0.f, 0.f, 0.f, 1.f);
	return *this;
}

inline matrix &matrix::transpose() {
	_MM_TRANSPOSE4_PS(x, y, z, t);
	return *this;
}

inline matrix &matrix::setShadow(const plane &p, const vec &lightPos) {
	const vec planeND = p.get(); // plane.normalX, plane.normalY, plane.normalZ, plane.D
	const vec dot = _mm_and_ps(SSE_RMASK(~0, 0, 0, 0), lightPos.xmmDot(planeND)); // dot, 0, 0, 0

	x = dot - lightPos * _mm_shuffle_ps(planeND, planeND, SSE_RSHUFFLE(0, 0, 0, 0));
	y = vec(_mm_shuffle_ps(dot, dot, SSE_RSHUFFLE(1, 0, 1, 1))) - lightPos * _mm_shuffle_ps(planeND, planeND, SSE_RSHUFFLE(1, 1, 1, 1));
	z = vec(_mm_shuffle_ps(dot, dot, SSE_RSHUFFLE(1, 1, 0, 1))) - lightPos * _mm_shuffle_ps(planeND, planeND, SSE_RSHUFFLE(2, 2, 2, 2));
	t = vec(_mm_shuffle_ps(dot, dot, SSE_RSHUFFLE(1, 1, 1, 0))) - lightPos * _mm_shuffle_ps(planeND, planeND, SSE_RSHUFFLE(3, 3, 3, 3));
	return *this;
}

inline matrix &matrix::setReflect(const plane &p) {
	const vec planeND = p.get();
	const vec mtwo = _mm_set1_ps(-2.f);
	
	x = mtwo * _mm_shuffle_ps(planeND, planeND, SSE_RSHUFFLE(0, 0, 0, 0)) * planeND + _mm_set_ss(1.f);
	x = mtwo * _mm_shuffle_ps(planeND, planeND, SSE_RSHUFFLE(1, 1, 1, 1)) * planeND + _mm_setr_ps(0.f, 1.f, 0.f, 0.f);
	x = mtwo * _mm_shuffle_ps(planeND, planeND, SSE_RSHUFFLE(2, 2, 2, 2)) * planeND + _mm_setr_ps(0.f, 0.f, 1.f, 0.f);
	t(0.f, 0.f, 0.f, 1.f);
	return *this;
}

inline matrix &matrix::lookAt(const vec &eye, const vec &iforward, const vec &iup) {
	const vec forward = (iforward - eye).normalized();
	const vec side = forward.cross(iup).normalize();
	const vec up = side.cross(forward); // real up

	x = _mm_set_ss(side.dot(eye));
	x = _mm_shuffle_ps(x, x, SSE_RSHUFFLE(1, 1, 1, 0));
	x = _mm_or_ps(side, x);
	
	y = _mm_set_ss(up.dot(eye));
	y = _mm_shuffle_ps(y, y, SSE_RSHUFFLE(1, 1, 1, 0));
	y = _mm_or_ps(up, y);
	
	z = _mm_set_ss(forward.dot(eye));
	z = _mm_shuffle_ps(z, z, SSE_RSHUFFLE(1, 1, 1, 0));
	z = _mm_or_ps(forward, z);

	t(0.f, 0.f, 0.f, 1.f);

	_MM_TRANSPOSE4_PS(x, y, z, t);
	return *this;
}

//inline matrix &matrix::setFrustum(const frustum &f) {
//	return setFrustum(f.fov, f.aspect, f.near, f.far);
//}

inline matrix &matrix::setFrustum(float left, float right, float bottom, float top, float znear, float zfar) {
	x(2.f * znear / (right - left), 0.f, 0.f, 0.f);
	y(0.f, 2.f * znear / (top - bottom), 0.f, 0.f);
	z((right + left) / (right - left), (top + bottom) / (top - bottom), -(zfar + znear) / (zfar - znear), -1.f);
	t(0.f, 0.f, -(2.f * zfar * znear) / (zfar - znear), 0.f);
	return *this;
}

inline matrix &matrix::setFrustum(float fovy, float aspect, float znear, float zfar) {
	float ymax = znear * tanf(degToRad(fovy));
    float ymin = -ymax;
    float xmin = ymin * aspect;
    float xmax = ymax * aspect;
	return setFrustum(xmin, xmax, ymin, ymax, znear, zfar);
}

inline matrix &matrix::setOrtho(float left, float right, float bottom, float top, float znear, float zfar) {
	x(2.f / (right - left), 0.f, 0.f, 0.f);
	y(0.f, 2.f / (top - bottom), 0.f, 0.f);
	z(0.f, 0.f, -2.f / (zfar - znear), 0.f);
	t(-(right + left) / (right - left), -(top + bottom) / (top - bottom), -(zfar + znear) / (zfar - znear), 1.f);
	return *this;
}

inline matrix &matrix::inverse() {
	// taken from intel manual 44-24504301
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;

	// transpose
	_MM_TRANSPOSE4_PS(x, y, z, t);
	row0 = x;
	row1 = y;
	row2 = z;
	row3 = t;
		
	tmp1 = _mm_mul_ps(row2, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_mul_ps(row1, tmp1);
	minor1 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
	minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
	minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
	
	tmp1 = _mm_mul_ps(row1, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
	minor3 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
	minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
	
	tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	row2 = _mm_shuffle_ps(row2, row2, 0x4E);
	minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
	minor2 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
	minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
	
	tmp1 = _mm_mul_ps(row0, row1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
	
	tmp1 = _mm_mul_ps(row0, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
	minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
	
	tmp1 = _mm_mul_ps(row0, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
	
	det = _mm_mul_ps(row0, minor0);
	det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
	det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
	tmp1 = _mm_rcp_ss(det); // change this to _mm_div_ss(_mm_set_ss(1.f), det)?
	det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
	det = _mm_shuffle_ps(det, det, 0x00);
	
	x = _mm_mul_ps(det, minor0);
	y = _mm_mul_ps(det, minor1);
	z = _mm_mul_ps(det, minor2);
	t = _mm_mul_ps(det, minor3);

	return *this;
}

inline matrix &matrix::inverseSimple() {
	static const vec wMask = SSE_RMASK(~0, ~0, ~0, 0);
	static const vec signMask(-0.f, -0.f, -0.f, -0.f);
	static const vec w1Mask(0.f, 0.f, 0.f, 1.f);
	
	// inverse 3x3
	vec x1 = _mm_movelh_ps(_mm_unpacklo_ps(x, y), z); // 0, 4, 8, -
	vec y1 = _mm_movehl_ps(_mm_movehdup_ps(z), _mm_unpacklo_ps(x, y)); // 1, 5, 9, -
	vec z1 = _mm_movelh_ps(_mm_unpackhi_ps(x, y), _mm_movehl_ps(z, z)); // 2, 6, 10, -

	x = _mm_and_ps(x1, wMask);
	y = _mm_and_ps(y1, wMask);
	z = _mm_and_ps(z1, wMask);
	t = x1 * _mm_xor_ps(signMask, _mm_shuffle_ps(t, t, SSE_RSHUFFLE(0, 0, 0, 0)))
		+ y1 * _mm_xor_ps(signMask, _mm_shuffle_ps(t, t, SSE_RSHUFFLE(1, 1, 1, 1)))
		+ z1 * _mm_xor_ps(signMask, _mm_shuffle_ps(t, t, SSE_RSHUFFLE(2, 2, 2, 2)));
	
	// set w to 1.f
	t = _mm_or_ps(w1Mask, _mm_and_ps(wMask, t));
	return *this;
}

inline matrix &matrix::textureProjection(const matrix &lightProjection, const matrix &lightView, const matrix &invModelView) {
	static const matrix bias(
		vec(.5f, 0.f, 0.f, 0.f),
		vec(0.f, .5f, 0.f, 0.f),
		vec(0.f, 0.f, .5f, 0.f),
		vec(.5f, .5f, .5f, 1.f));
	return *this = bias * lightProjection * lightView * invModelView;
}

inline vec matrix::transform(const vec &v) const {
	return *this * v;
}

inline bool matrix::isIdentity() const {
	__m128 t = _mm_cmpneq_ps(x, vec(1.f, 0.f, 0.f, 0.f));
	t = _mm_or_ps(t, _mm_cmpneq_ps(y, vec(0.f, 1.f, 0.f, 0.f)));
	t = _mm_or_ps(t, _mm_cmpneq_ps(z, vec(0.f, 0.f, 1.f, 0.f)));
	t = _mm_or_ps(t, _mm_cmpneq_ps(t, vec(0.f, 0.f, 0.f, 1.f)));
	return 0 != _mm_movemask_ps(t);
}

inline vec matrix::getTranslate() const {
	return t.xmmVec3();
}

inline vec matrix::getScale() const {
	vec s = _mm_and_ps(SSE_RMASK(~0, 0, 0, 0), x);
	s = _mm_or_ps(s, _mm_and_ps(SSE_RMASK(0, ~0, 0, 0), y));
	s = _mm_or_ps(s, _mm_and_ps(SSE_RMASK(0, 0, ~0, 0), z));
	return s;
}

inline vec matrix::getAxisX() const {
	return x.xmmVec3();
}

inline vec matrix::getAxisY() const {
	return y.xmmVec3();
}

inline vec matrix::getAxisZ() const {
	return z.xmmVec3();
}

//~
