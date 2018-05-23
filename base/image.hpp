/*
 * granite engine 1.0 | 2006-2015 | Jakub Duracz | jakubduracz@gmail.com | http://jakubduracz.com
 * file: image
 * created: 10-09-2006
 *
 * description: image manipulation / loading utils
 *
 * changelog:
 * - 10-09-2006: file create (1st version)
 * - 22-01-2009: 2nd rewrite
 * - 31-07-2015: 3rd rewrite
 */

#pragma once
#include "includes.hpp"
#include "stream.hpp"
#include "simd_vector.hpp"

namespace granite { namespace base {

enum imageCodec {
    imageCodecJPEG,
    imageCodecPNG,
    imageCodecBMP,
    imageCodecTGA
};

struct image {
    simd_vector<uint8, 3 * 16> data;
    int channels;
    int width;
    int height;

    // conversion operator for const stream
    inline operator const_stream();
};

bool toImage(const_stream s, image &i);
inline image toImage(const_stream s);
bool fromImage(const image &i, stream &s, imageCodec codec);
inline stream fromImage(const image &i, imageCodec);

#include "image.inc.hpp"

}}
