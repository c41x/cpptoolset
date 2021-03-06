/*
 * granite engine 1.0 | 2006-2016 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
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

    rl = x * vec(_mm_shuffle_ps(m.x, m.x, SSE_RSHUFFLE(0, 0, 0, 0)));
    rl += y * vec(_mm_shuffle_ps(m.x, m.x, SSE_RSHUFFLE(1, 1, 1, 1)));
    rl += z * vec(_mm_shuffle_ps(m.x, m.x, SSE_RSHUFFLE(2, 2, 2, 2)));
    rl += t * vec(_mm_shuffle_ps(m.x, m.x, SSE_RSHUFFLE(3, 3, 3, 3)));
    r.x = rl;

    rl = x * vec(_mm_shuffle_ps(m.y, m.y, SSE_RSHUFFLE(0, 0, 0, 0)));
    rl += y * vec(_mm_shuffle_ps(m.y, m.y, SSE_RSHUFFLE(1, 1, 1, 1)));
    rl += z * vec(_mm_shuffle_ps(m.y, m.y, SSE_RSHUFFLE(2, 2, 2, 2)));
    rl += t * vec(_mm_shuffle_ps(m.y, m.y, SSE_RSHUFFLE(3, 3, 3, 3)));
    r.y = rl;

    rl = x * vec(_mm_shuffle_ps(m.z, m.z, SSE_RSHUFFLE(0, 0, 0, 0)));
    rl += y * vec(_mm_shuffle_ps(m.z, m.z, SSE_RSHUFFLE(1, 1, 1, 1)));
    rl += z * vec(_mm_shuffle_ps(m.z, m.z, SSE_RSHUFFLE(2, 2, 2, 2)));
    rl += t * vec(_mm_shuffle_ps(m.z, m.z, SSE_RSHUFFLE(3, 3, 3, 3)));
    r.z = rl;

    rl = x * vec(_mm_shuffle_ps(m.t, m.t, SSE_RSHUFFLE(0, 0, 0, 0)));
    rl += y * vec(_mm_shuffle_ps(m.t, m.t, SSE_RSHUFFLE(1, 1, 1, 1)));
    rl += z * vec(_mm_shuffle_ps(m.t, m.t, SSE_RSHUFFLE(2, 2, 2, 2)));
    rl += t * vec(_mm_shuffle_ps(m.t, m.t, SSE_RSHUFFLE(3, 3, 3, 3)));
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
    r += t; // ignore w component (always 1.f) - just apply transform
    return r;
}

inline matrix &matrix::translate(const vec &_t) {
    matrix r;
    r.setTranslation(_t);
    return *this *= r;
}

inline matrix &matrix::scale(const vec &s) {
    matrix r;
    r.setScale(s);
    return *this *= r;
}
inline matrix &matrix::rotate(const vec &axis, float angle) {
    matrix r;
    r.setRotation(axis, angle);
    return *this *= r;
}

inline matrix &matrix::setTranslation(const vec &_t) {
    // 12, 13, 14 - (t)
    static const __m128 maskz = SSE_RMASK(~0, ~0, ~0, 0);
    x(1.f, 0.f, 0.f, 0.f);
    y(0.f, 1.f, 0.f, 0.f);
    z(0.f, 0.f, 1.f, 0.f);
    t = _mm_and_ps(_t, maskz);
    return *this;
}

inline matrix &matrix::setScale(const vec &s) {
    x = _mm_and_ps(s, SSE_RMASK(~0, 0, 0, 0));
    y = _mm_and_ps(s, SSE_RMASK(0, ~0, 0, 0));
    z = _mm_and_ps(s, SSE_RMASK(0, 0, ~0, 0));
    t(0.f, 0.f, 0.f, 1.f);
    return *this;
}

inline matrix &matrix::setRotation(const vec &axis, float angle) {
    // axis w must be 0, angle in radians
    const float tc = cosf(angle);
    const vec xmmtc = _mm_set_ss(tc);
    const vec co = _mm_set1_ps(1.f - tc); // 1 - cos, 1 - cos, 1 - cos, 1 - cos
    const vec sA = vec(_mm_set1_ps(sinf(angle))) * axis +
        _mm_shuffle_ps(xmmtc, xmmtc, SSE_RSHUFFLE(3, 2, 1, 0)); // axisX * sin, axisY * sin, axisZ * sin, cos

    vec rx = axis * _mm_shuffle_ps(axis, axis, SSE_RSHUFFLE(0, 0, 0, 0)) * co +
        _mm_shuffle_ps(sA, _mm_xor_ps(sA, vec(0.f, -0.f, 0.f, 0.f)), SSE_RSHUFFLE(3, 2, 1, 0));
    vec ry = vec(_mm_shuffle_ps(axis, axis, SSE_RSHUFFLE(0, 1, 1, 3))) * _mm_shuffle_ps(axis, axis, SSE_RSHUFFLE(1, 1, 2, 3)) * co +
        _mm_shuffle_ps(_mm_xor_ps(sA, vec(0.f, 0.f, -0.f, 0.f)), sA, SSE_RSHUFFLE(2, 3, 0, 1));
    vec rz = axis * _mm_shuffle_ps(axis, axis, SSE_RSHUFFLE(2, 2, 2, 3)) * co +
        _mm_shuffle_ps(_mm_xor_ps(sA, vec(-0.f, 0.f, 0.f, 0.f)), sA, SSE_RSHUFFLE(1, 0, 3, 2));

    static const __m128 wMask = SSE_RMASK(~0, ~0, ~0, 0);
    x = _mm_and_ps(rx, wMask);
    y = _mm_and_ps(ry, wMask);
    z = _mm_and_ps(rz, wMask);
    t(0.f, 0.f, 0.f, 1.f);
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

// TODO: makes sense? fov, aspect, near and far may not be initialized in frustum
//inline matrix &matrix::setFrustum(const frustum &f) {
//  return setFrustum(f.fov, f.aspect, f.near, f.far);
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
    // taken from XNAMath
    transpose();
    vec V00 = _mm_shuffle_ps(z, z,_MM_SHUFFLE(1,1,0,0));
    vec V10 = _mm_shuffle_ps(t, t,_MM_SHUFFLE(3,2,3,2));
    vec V01 = _mm_shuffle_ps(x, x,_MM_SHUFFLE(1,1,0,0));
    vec V11 = _mm_shuffle_ps(y, y,_MM_SHUFFLE(3,2,3,2));
    vec V02 = _mm_shuffle_ps(z, x,_MM_SHUFFLE(2,0,2,0));
    vec V12 = _mm_shuffle_ps(t, y,_MM_SHUFFLE(3,1,3,1));

    vec D0 = _mm_mul_ps(V00,V10);
    vec D1 = _mm_mul_ps(V01,V11);
    vec D2 = _mm_mul_ps(V02,V12);

    V00 = _mm_shuffle_ps(z,z,_MM_SHUFFLE(3,2,3,2));
    V10 = _mm_shuffle_ps(t,t,_MM_SHUFFLE(1,1,0,0));
    V01 = _mm_shuffle_ps(x,x,_MM_SHUFFLE(3,2,3,2));
    V11 = _mm_shuffle_ps(y,y,_MM_SHUFFLE(1,1,0,0));
    V02 = _mm_shuffle_ps(z,x,_MM_SHUFFLE(3,1,3,1));
    V12 = _mm_shuffle_ps(t,y,_MM_SHUFFLE(2,0,2,0));

    V00 = _mm_mul_ps(V00,V10);
    V01 = _mm_mul_ps(V01,V11);
    V02 = _mm_mul_ps(V02,V12);
    D0 = _mm_sub_ps(D0,V00);
    D1 = _mm_sub_ps(D1,V01);
    D2 = _mm_sub_ps(D2,V02);
    V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,1,3,1)); // V11 = D0Y,D0W,D2Y,D2Y
    V00 = _mm_shuffle_ps(y, y,_MM_SHUFFLE(1,0,2,1));
    V10 = _mm_shuffle_ps(V11,D0,_MM_SHUFFLE(0,3,0,2));
    V01 = _mm_shuffle_ps(x, x,_MM_SHUFFLE(0,1,0,2));
    V11 = _mm_shuffle_ps(V11,D0,_MM_SHUFFLE(2,1,2,1));
    vec V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,3,3,1)); // V13 = D1Y,D1W,D2W,D2W
    V02 = _mm_shuffle_ps(t, t,_MM_SHUFFLE(1,0,2,1));
    V12 = _mm_shuffle_ps(V13,D1,_MM_SHUFFLE(0,3,0,2));
    vec V03 = _mm_shuffle_ps(z, z,_MM_SHUFFLE(0,1,0,2));
    V13 = _mm_shuffle_ps(V13,D1,_MM_SHUFFLE(2,1,2,1));

    vec C0 = _mm_mul_ps(V00,V10);
    vec C2 = _mm_mul_ps(V01,V11);
    vec C4 = _mm_mul_ps(V02,V12);
    vec C6 = _mm_mul_ps(V03,V13);

    V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(0,0,1,0)); // V11 = D0X,D0Y,D2X,D2X
    V00 = _mm_shuffle_ps(y, y,_MM_SHUFFLE(2,1,3,2));
    V10 = _mm_shuffle_ps(D0,V11,_MM_SHUFFLE(2,1,0,3));
    V01 = _mm_shuffle_ps(x, x,_MM_SHUFFLE(1,3,2,3));
    V11 = _mm_shuffle_ps(D0,V11,_MM_SHUFFLE(0,2,1,2));
    V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(2,2,1,0)); // V13 = D1X,D1Y,D2Z,D2Z
    V02 = _mm_shuffle_ps(t, t,_MM_SHUFFLE(2,1,3,2));
    V12 = _mm_shuffle_ps(D1,V13,_MM_SHUFFLE(2,1,0,3));
    V03 = _mm_shuffle_ps(z, z,_MM_SHUFFLE(1,3,2,3));
    V13 = _mm_shuffle_ps(D1,V13,_MM_SHUFFLE(0,2,1,2));

    V00 = _mm_mul_ps(V00,V10);
    V01 = _mm_mul_ps(V01,V11);
    V02 = _mm_mul_ps(V02,V12);
    V03 = _mm_mul_ps(V03,V13);
    C0 = _mm_sub_ps(C0,V00);
    C2 = _mm_sub_ps(C2,V01);
    C4 = _mm_sub_ps(C4,V02);
    C6 = _mm_sub_ps(C6,V03);

    V00 = _mm_shuffle_ps(y,y,_MM_SHUFFLE(0,3,0,3));
    V10 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,0,2,2)); // V10 = D0Z,D0Z,D2X,D2Y
    V10 = _mm_shuffle_ps(V10,V10,_MM_SHUFFLE(0,2,3,0));
    V01 = _mm_shuffle_ps(x,x,_MM_SHUFFLE(2,0,3,1));
    V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,0,3,0)); // V11 = D0X,D0W,D2X,D2Y
    V11 = _mm_shuffle_ps(V11,V11,_MM_SHUFFLE(2,1,0,3));
    V02 = _mm_shuffle_ps(t,t,_MM_SHUFFLE(0,3,0,3));
    V12 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,2,2,2)); // V12 = D1Z,D1Z,D2Z,D2W
    V12 = _mm_shuffle_ps(V12,V12,_MM_SHUFFLE(0,2,3,0));
    V03 = _mm_shuffle_ps(z,z,_MM_SHUFFLE(2,0,3,1));
    V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,2,3,0)); // V13 = D1X,D1W,D2Z,D2W
    V13 = _mm_shuffle_ps(V13,V13,_MM_SHUFFLE(2,1,0,3));

    V00 = _mm_mul_ps(V00,V10);
    V01 = _mm_mul_ps(V01,V11);
    V02 = _mm_mul_ps(V02,V12);
    V03 = _mm_mul_ps(V03,V13);
    vec C1 = _mm_sub_ps(C0,V00);
    C0 = _mm_add_ps(C0,V00);
    vec C3 = _mm_add_ps(C2,V01);
    C2 = _mm_sub_ps(C2,V01);
    vec C5 = _mm_sub_ps(C4,V02);
    C4 = _mm_add_ps(C4,V02);
    vec C7 = _mm_add_ps(C6,V03);
    C6 = _mm_sub_ps(C6,V03);

    C0 = _mm_shuffle_ps(C0,C1,_MM_SHUFFLE(3,1,2,0));
    C2 = _mm_shuffle_ps(C2,C3,_MM_SHUFFLE(3,1,2,0));
    C4 = _mm_shuffle_ps(C4,C5,_MM_SHUFFLE(3,1,2,0));
    C6 = _mm_shuffle_ps(C6,C7,_MM_SHUFFLE(3,1,2,0));
    C0 = _mm_shuffle_ps(C0,C0,_MM_SHUFFLE(3,1,2,0));
    C2 = _mm_shuffle_ps(C2,C2,_MM_SHUFFLE(3,1,2,0));
    C4 = _mm_shuffle_ps(C4,C4,_MM_SHUFFLE(3,1,2,0));
    C6 = _mm_shuffle_ps(C6,C6,_MM_SHUFFLE(3,1,2,0));

    vec vTemp = C0.xmmDot(x);
    vTemp = _mm_div_ps(_mm_set1_ps(1.f),vTemp);

    x = _mm_mul_ps(C0,vTemp);
    y = _mm_mul_ps(C2,vTemp);
    z = _mm_mul_ps(C4,vTemp);
    t = _mm_mul_ps(C6,vTemp);

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
    __m128 tmp = _mm_cmpeq_ps(x, vec(1.f, 0.f, 0.f, 0.f));
    tmp = _mm_and_ps(tmp, _mm_cmpeq_ps(y, vec(0.f, 1.f, 0.f, 0.f)));
    tmp = _mm_and_ps(tmp, _mm_cmpeq_ps(z, vec(0.f, 0.f, 1.f, 0.f)));
    tmp = _mm_and_ps(tmp, _mm_cmpeq_ps(t, vec(0.f, 0.f, 0.f, 1.f)));
    return 0xf == _mm_movemask_ps(tmp);
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

inline frustum::frustum(float _fov, float _aspect, float _znear, float _zfar, const vec &eye, const vec &look, const vec &up) {
    (*this)(_fov, _aspect, _znear, _zfar, eye, look, up);
}

inline frustum::frustum(const matrix &MVP) {
    setMVP(MVP);
}

inline frustum &frustum::operator()(float _fov, float _aspect, float _znear, float _zfar, const vec &eye, const vec &look, const vec &up)
{
    setProjection(_fov, _aspect, _znear, _zfar);
    setModelView(eye, look, up);
    return *this;
}

inline frustum &frustum::setProjection(float _fov, float _aspect, float _znear, float _zfar) {
    fov = _fov;
    aspect = _aspect;
    znear = _znear;
    zfar = _zfar;

    // calculate near/far widths
    const float fovAngle = tanf(degToRad(_fov) * .5f);
    nearHeight = znear * fovAngle;
    nearWidth = nearHeight * aspect;
    farHeight = zfar * fovAngle;
    farWidth = farHeight * aspect;

    return *this;
}

inline frustum &frustum::setModelView(const vec &eye, const vec &look, const vec &up) {
    vec x, y, z, farCenter, nearCenter;

    z = (eye - look).normalize();
    x = up.cross(z).normalize();
    y = z.cross(x);
    nearCenter = eye - z * znear;
    farCenter = eye - z * zfar;

    points[POINT_NLT] = nearCenter + y * nearHeight - x * nearWidth;
    points[POINT_NRT] = nearCenter + y * nearHeight + x * nearWidth;
    points[POINT_NLB] = nearCenter - y * nearHeight - x * nearWidth;
    points[POINT_NRB] = nearCenter - y * nearHeight + x * nearWidth;

    points[POINT_FLT] = farCenter + y * farHeight - x * farWidth;
    points[POINT_FRT] = farCenter + y * farHeight + x * farWidth;
    points[POINT_FLB] = farCenter - y * farHeight - x * farWidth;
    points[POINT_FRB] = farCenter - y * farHeight + x * farWidth;

    planes[PLANE_NEAR](points[POINT_NLT], points[POINT_NRT], points[POINT_NRB]);
    planes[PLANE_FAR](points[POINT_FLT], points[POINT_FLB], points[POINT_FRB]);
    planes[PLANE_LEFT](points[POINT_NLT], points[POINT_NLB], points[POINT_FLB]);
    planes[PLANE_RIGHT](points[POINT_FRT], points[POINT_FRB], points[POINT_NRB]);
    planes[PLANE_TOP](points[POINT_FRT], points[POINT_NRT], points[POINT_NLT]);
    planes[PLANE_BOTTOM](points[POINT_FLB], points[POINT_NLB], points[POINT_NRB]);

    planes[0].normal.normalize();
    planes[1].normal.normalize();
    planes[2].normal.normalize();
    planes[3].normal.normalize();
    planes[4].normal.normalize();
    planes[5].normal.normalize();
    return *this;

}

inline frustum &frustum::setMVP(const matrix &MVP) {
    // taken from some GPG book edition, from chapter about Frusum Culling
    planes[PLANE_LEFT](MVP.t - MVP.x);
    planes[PLANE_RIGHT](MVP.t + MVP.x);
    planes[PLANE_TOP](MVP.t - MVP.y);
    planes[PLANE_BOTTOM](MVP.t + MVP.y);
    planes[PLANE_FAR](MVP.t - MVP.z);
    planes[PLANE_NEAR](MVP.t + MVP.z);
    return *this;
}

inline bool frustum::contains(const vec &p) const {
    for(int i = 0; i < 6; ++i)
        if(planes[i].intersection(p) < 0.f)
            return false;
    return true;
}

inline bool frustum::contains(const sphere &s) const {
    for(int i = 0; i < 6; ++i)
        if(planes[i].intersection(s) < 0.f)
            return false;
}

inline bool frustum::contains(const aabbox &b) const {
    for(int i = 0; i < 6; ++i)
        if(!planes[i].isAnyAbove(b))
            return false;
    return true;
}

inline bool frustum::contains(const obbox &b) const {
    for(int i = 0; i < 6; ++i)
        if(!planes[i].isAnyAbove(b))
            return false;
    return true;
}

inline quaternion::quaternion() {}

inline quaternion::quaternion(float w, float x, float y, float z) {
    xmm = _mm_setr_ps(x, y, z, w);
}

inline quaternion::quaternion(const vec &e) {
    euler(e);
}

inline quaternion::quaternion(const matrix &rotationMatrix) {
    // Ken Shoemake - 1987 SIGGRAPH "Quaternion Calculus and Fast Animation".
    vec4f rx = rotationMatrix.x;
    vec4f ry = rotationMatrix.y;
    vec4f rz = rotationMatrix.z;
    float trace = rx.x + ry.y + rz.z;
    float root;
    vec4f q;

    if (trace > 0.0f) {
        root = sqrtf(trace + 1.0f);
        q.w = 0.5f * root;
        root = 0.5f / root;
        q.x = (ry.z - rz.y) * root;
        q.y = (rz.x - rx.z) * root;
        q.z = (rx.y - ry.x) * root;
    }
    else {
        static size_t next[3] = { 1, 2, 0 };
        vec4f *mat[3] = { &rx, &ry, &rz };
        size_t i = 0;
        if (ry.y > rx.x) i = 1;
        if (rz.z > mat[i]->data[i]) i = 2;
        size_t j = next[i];
        size_t k = next[j];
        root = sqrtf(mat[i]->data[i] - mat[j]->data[j] - mat[k]->data[k] + 1.0f);
        float *qt[3] = { &q.x, &q.y, &q.z };
        *qt[i] = 0.5f * root;
        root = 0.5f / root;
        q.w = (mat[k]->data[j] - mat[j]->data[k]) * root;
        *qt[j] = (mat[j]->data[i] + mat[i]->data[j]) * root;
        *qt[k] = (mat[k]->data[i] + mat[i]->data[k]) * root;
    }

    xmm = q;
}

inline quaternion::quaternion(const vec &ax, const vec &ay, const vec &az)
        : quaternion(matrix(ax, ay, az, vec(0.f, 0.f, 0.f, 1.f))){}

inline quaternion::quaternion(float angle, const vec &axis) {
    angleAxis(angle, axis);
}

inline quaternion::~quaternion() {}

inline quaternion quaternion::operator*(float s) const {
    quaternion r;
    r.xmm = xmm * s;
    return r;
}

inline quaternion quaternion::operator*(const quaternion &q) const {
    __m128 a = _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(3, 3, 3, 3));
    a = _mm_mul_ps(a, _mm_shuffle_ps(q.xmm, q.xmm, SSE_RSHUFFLE(0, 1, 2, 3)));
    __m128 b = _mm_setr_ps(1.0f, 1.0f, 1.0f, -1.0f);
    b = _mm_mul_ps(b, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(0, 1, 2, 0)));
    b = _mm_mul_ps(b, _mm_shuffle_ps(q.xmm, q.xmm, SSE_RSHUFFLE(3, 3, 3, 0)));
    a = _mm_add_ps(a, b);
    b = _mm_setr_ps(1.0f, 1.0f, 1.0f, -1.0f);
    b = _mm_mul_ps(b, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(1, 2, 0, 1)));
    b = _mm_mul_ps(b, _mm_shuffle_ps(q.xmm, q.xmm, SSE_RSHUFFLE(2, 0, 1, 1)));
    a = _mm_add_ps(a, b);
    b = _mm_setr_ps(-1.0f, -1.0f, -1.0f, -1.0f);
    b = _mm_mul_ps(b, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(2, 0, 1, 2)));
    b = _mm_mul_ps(b, _mm_shuffle_ps(q.xmm, q.xmm, SSE_RSHUFFLE(1, 2, 0, 2)));
    quaternion r;
    r.xmm = _mm_add_ps(a, b);
    return r;
}

inline vec quaternion::operator*(const vec &p) const {
    vec uv = xmm.cross(p);
    vec uuv = xmm.cross(uv);
    vec w = _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(3, 3, 3, 3));
    uv *= w * 2.0f;
    uuv *= 2.0f;
    return p + uv + uuv;
}

inline quaternion quaternion::operator+(const quaternion &q) const {
    quaternion r;
    r.xmm = _mm_add_ps(xmm, q.xmm);
    return r;
}

inline quaternion quaternion::operator-(const quaternion &q) const {
    quaternion r;
    r.xmm = _mm_sub_ps(xmm, q.xmm);
    return r;
}

inline quaternion quaternion::operator-() const {
    quaternion q;
    static const vec signMask(-0.f, -0.f, -0.f, -0.f);
    q.xmm = _mm_xor_ps(xmm, signMask);
    return q;
}

inline quaternion::operator matrix() const {
    return toMatrix();
}

inline quaternion &quaternion::operator()(float w, float x, float y, float z) {
    xmm = _mm_setr_ps(x, y, z, w);
    return *this;
}

inline quaternion &quaternion::operator()(const matrix &m) {
    return *this = quaternion(m);
}

inline quaternion &quaternion::operator()(const vec &e) {
    return euler(e);
}

inline quaternion &quaternion::operator()(const vec &ax, const vec &ay, const vec &az) {
    return *this = quaternion(ax, ay, az);
}

inline quaternion &quaternion::operator()(float angle, const vec &axis) {
    angleAxis(angle, axis);
    return *this;
}

inline matrix quaternion::toMatrix() const {
    matrix r;
    static const __m128 maskw = SSE_RMASK(~0, ~0, ~0, 0);
    __m128 q = _mm_and_ps(xmm, maskw);
    q = _mm_add_ps(xmm, q); // x + x, y + y, z + z, w

    __m128 a = _mm_setr_ps(-1.0f, 1.0f, 1.0f, 0.0f);
    a = _mm_mul_ps(a, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(1, 0, 0, 3)));
    a = _mm_mul_ps(a, _mm_shuffle_ps(q, q, SSE_RSHUFFLE(1, 1, 2, 3)));
    __m128 b = _mm_setr_ps(-1.0f, 1.0f, -1.0f, 0.0f);
    b = _mm_mul_ps(b, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(2, 3, 3, 3)));
    b = _mm_mul_ps(b, _mm_shuffle_ps(q, q, SSE_RSHUFFLE(2, 2, 1, 3)));
    b = _mm_add_ps(b, _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f));
    r.x = _mm_add_ps(a, b);

    a = _mm_setr_ps(1.0f, -1.0f, 1.0f, 0.0f);
    a = _mm_mul_ps(a, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(0, 0, 1, 3)));
    a = _mm_mul_ps(a, _mm_shuffle_ps(q, q, SSE_RSHUFFLE(1, 0, 2, 3)));
    b = _mm_setr_ps(-1.0f, -1.0f, 1.0f, 0.0f);
    b = _mm_mul_ps(b, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(3, 2, 3, 3)));
    b = _mm_mul_ps(b, _mm_shuffle_ps(q, q, SSE_RSHUFFLE(2, 2, 0, 3)));
    b = _mm_add_ps(b, _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f));
    r.y = _mm_add_ps(a, b);

    a = _mm_setr_ps(1.0f, 1.0f, -1.0f, 0.0f);
    a = _mm_mul_ps(a, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(0, 1, 0, 3)));
    a = _mm_mul_ps(a, _mm_shuffle_ps(q, q, SSE_RSHUFFLE(2, 2, 0, 3)));
    b = _mm_setr_ps(1.0f, -1.0f, -1.0f, 0.0f);
    b = _mm_mul_ps(b, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(3, 3, 1, 3)));
    b = _mm_mul_ps(b, _mm_shuffle_ps(q, q, SSE_RSHUFFLE(1, 0, 1, 3)));
    b = _mm_add_ps(b, _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f));
    r.z = _mm_add_ps(a, b);
    r.t = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    return r;
}

inline float quaternion::length() const {
    return _mm_cvtss_f32(xmmLength());
}

inline float quaternion::getRoll() const {
    vec4f q = xmm;
    //return atan2(2.0f * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z);  // alternative version (without reprojection)
    // calculate x axis and calculate angle (atan)
    float fTy = 2.0f * q.y;
    float fTz = 2.0f * q.z;
    float fTwz = fTz * q.w;
    float fTxy = fTy * q.x;
    float fTyy = fTy * q.y;
    float fTzz = fTz * q.z;
    return atan2(fTxy + fTwz, 1.0f - (fTyy + fTzz));
}

inline float quaternion::getPitch() const {
    vec4f q = xmm;
    //return atan2(2.0f * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);  // alternative version (without reprojection)
    float fTx = 2.0f * q.x;
    float fTz = 2.0f * q.z;
    float fTwx = fTx * q.w;
    float fTxx = fTx * q.x;
    float fTyz = fTz * q.y;
    float fTzz = fTz * q.z;
    return atan2(fTyz + fTwx, 1.0f - (fTxx + fTzz));
}

inline float quaternion::getYaw() const {
    vec4f q = xmm;
    //return asin(-2.0f * (q.x * q.z - q.w * q.y)); // alternative version (without reprojection)
    float fTx = 2.0f * q.x;
    float fTy = 2.0f * q.y;
    float fTz = 2.0f * q.z;
    float fTwy = fTy * q.w;
    float fTxx = fTx * q.x;
    float fTxz = fTz * q.x;
    float fTyy = fTy * q.y;
    return atan2(fTxz + fTwy, 1.0f - (fTxx + fTyy));
}

inline vec quaternion::getXAxis() const {
    __m128 t = _mm_setr_ps(-2.f, 2.f, 2.f, 0.f);
    t = _mm_mul_ps(t, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(1, 0, 0, 3)));
    t = _mm_mul_ps(t, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(1, 1, 2, 3)));
    __m128 q = _mm_setr_ps(-2.f, 2.f, -2.f, 0.f);
    q = _mm_mul_ps(q, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(2, 3, 3, 3)));
    q = _mm_mul_ps(q, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(2, 2, 1, 3)));
    return _mm_add_ps(_mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f), _mm_add_ps(q, t));

}

inline vec quaternion::getYAxis() const {
    __m128 t = _mm_setr_ps(2.0f, -2.0f, 2.0f, 0.0f);
    t = _mm_mul_ps(t, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(0, 0, 1, 3)));
    t = _mm_mul_ps(t, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(1, 0, 2, 3)));
    __m128 q = _mm_setr_ps(-2.0f, -2.0f, 2.0f, 0.0f);
    q = _mm_mul_ps(q, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(3, 2, 3, 3)));
    q = _mm_mul_ps(q, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(2, 2, 0, 3)));
    return _mm_add_ps(_mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f), _mm_add_ps(t, q));
}

inline vec quaternion::getZAxis() const {
    __m128 t = _mm_setr_ps(2.0f, 2.0f, -2.0f, 0.0f);
    t = _mm_mul_ps(t, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(0, 1, 0, 3)));
    t = _mm_mul_ps(t, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(2, 2, 0, 3)));
    __m128 q = _mm_setr_ps(2.0, -2.0f, -2.0f, 0.0f);
    q = _mm_mul_ps(q, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(3, 3, 1, 3)));
    q = _mm_mul_ps(q, _mm_shuffle_ps(xmm, xmm, SSE_RSHUFFLE(1, 0, 1, 3)));
    return _mm_add_ps(_mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f), _mm_add_ps(t, q));
}

inline vec quaternion::xmmLength() const {
    vec t = xmm * xmm;
    t = _mm_hadd_ps(t, t);
    t = _mm_hadd_ps(t, t); // 4 component dot
    return _mm_sqrt_ps(t);
}

inline vec quaternion::euler() const {
    return vec(getPitch(), getYaw(), getRoll());
}

inline float quaternion::dot(const quaternion &q) const {
    vec t = xmm * q.xmm;
    t = _mm_hadd_ps(t, t);
    t = _mm_hadd_ps(t, t);
    return _mm_cvtss_f32(t);
}

inline quaternion quaternion::normalized() const {
    return quaternion(*this).normalize();
}

inline quaternion quaternion::inversed() const {
    return quaternion(*this).inverse();
}

inline quaternion quaternion::inversedUnit() const {
    return quaternion(*this).inverseUnit();
}

inline quaternion quaternion::inversedAngle() const {
    return quaternion(*this).inverseAngle();
}

inline quaternion &quaternion::identity() {
    xmm =_mm_set_ps(1.f, 0.f, 0.f, 0.f);
    return *this;
}

inline quaternion &quaternion::normalize() {
    xmm /= xmmLength();
    return *this;
}

inline quaternion &quaternion::inverse() {
    return normalize().inverseUnit();
}

inline quaternion &quaternion::inverseUnit() {
    static const vec signMask(-0.f, -0.f, -0.f, 0.f);
    xmm = _mm_xor_ps(xmm, signMask);
    return *this;
}

inline quaternion &quaternion::inverseAngle() {
    static const vec signMask(0.f, 0.f, 0.f, -0.f);
    xmm = _mm_xor_ps(xmm, signMask);
    return *this;
}

inline quaternion &quaternion::fromToRotation(const vec &from, const vec &to) {
    // TODO: -
    return *this;
}

inline quaternion &quaternion::lookRotation(const vec &forward, const vec &up) {
    // TODO: -
    return *this;
}

inline quaternion &quaternion::angleAxis(float angle, const vec &axis) {
    float halfAngle = angle * 0.5f;
    float s = sinf(halfAngle);
    static const __m128 oneW = _mm_setr_ps(0.f, 0.f, 0.f, 1.f);
    __m128 r = _mm_setr_ps(s, s, s, cosf(halfAngle));
    __m128 a = _mm_add_ps(axis.xmmVec3(), oneW); // just set w to 1
    xmm = _mm_mul_ps(r, a);
    return *this;
}

inline quaternion &quaternion::rotateTowards(const quaternion &q, float maxAngle) {
    // TODO: -
    return *this;
}

inline quaternion &quaternion::euler(const vec &euler) {
    vec4f e = euler * 0.5f;
    vec4f s(sinf(e.x), sinf(e.y), sinf(e.z));
    vec4f c(cosf(e.x), cosf(e.y), cosf(e.z));
    xmm = _mm_setr_ps(s.x * c.y * c.z - c.x * s.y * s.z,
                      c.x * s.y * c.z + s.x * c.y * s.z,
                      c.x * c.y * s.z - s.x * s.y * c.z,
                      c.x * c.y * c.z + s.x * s.y * s.z);
    return *this;
}

//~
