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

namespace granite { namespace base {

typedef std::vector<uint8> image;

bool toImage(const stream &s, image &i);
image toImage(const stream &s);

#include "image.inc.hpp"

}}
