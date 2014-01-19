/*
 * Copyright (C) 2014 Xo Wang
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#ifndef UTILITY_H_
#define UTILITY_H_

/* Compiler-specific attribute definitions. */
#ifdef __GNUC__
#define NORETURN __attribute__((noreturn))
#define ALIGNED(x) __attribute__((aligned(x)))
#else
#define NORETURN
#define ALIGNED(x)
#endif

/**
 * @brief  Expands arguments, then passes them to a macro or function.
 *
 * @note   Useful for passing multiple arguments to a macro as a single macro,
 *         e.g.
 *
 *         #define FN(a, b)  (a + b)
 *         #define ARGS      1, 2
 *
 *         FN(ARGS);          // this doesn't work
 *         INVOKE(FN, ARGS);  // this works
 *
 * @param[in] invokable  macro or function with zero or more arguments
 * @param[in] ...        arguments to pass to invokable
 */
#define INVOKE(invokable, ...) invokable(__VA_ARGS__)

#endif  /* UTILITY_H_ */
