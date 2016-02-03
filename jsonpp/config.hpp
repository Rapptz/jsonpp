// The MIT License (MIT)

// Copyright (c) 2014-2016 Danny Y.

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef JSONPP_CONFIG_HPP
#define JSONPP_CONFIG_HPP

// MSVC 2013 and lower don't have noexcept
#if !defined(JSONPP_NOEXCEPT)
#if defined(_MSC_VER) && _MSC_VER <= 1800
#define JSONPP_NOEXCEPT throw()
#else
#define JSONPP_NOEXCEPT noexcept
#endif
#endif

// MSVC doesn't conform to alternative tokens as keywords
#if defined(_MSC_VER)
#include <ciso646>
#endif

#if !defined(JSONPP_ASSERT)
#include <cassert>
#define JSONPP_ASSERT(condition, message) assert((condition) && (message))
#endif // JSONPP_ASSERT

#endif // JSONPP_CONFIG_HPP
