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

#ifndef BASE_INTEGER_H_
#define BASE_INTEGER_H_

#include <type_traits>

/**
 * @brief Takes the negation of the absolute value of an integer.
 *
 * @note Used to avoid undefined behavior for abs(INT_MIN). See C99 standard
 *       7.20.6.1.2 and footnote 265 for the description of abs/labs/llabs
 *       behavior with INT_MIN.
 *
 * @param i Input value.
 * @return Negative absolute value of i; defined for all values of i.
 */
template<typename T>
static inline T Nabs(T i) {
  static_assert(std::is_integral<T>::value && std::is_signed<T>::value,
                "Nabs is valid only for signed integers.");
  // Check if signed right shift sign extends (i.e. arithmetic right shift).
  if ((static_cast<T>(-1) >> 1) == static_cast<T>(-1)) {
    constexpr int num_bits = sizeof(T) * 8;
    // Splat sign bit into all 32 bits and complement.
    const T inverse_sign = ~(i >> (num_bits - 1));
    // If i is positive (inverse_sign = -1), this will invert i and add 1.
    // Otherwise (inverse_sign = 0) will be unchanged.
    return (i ^ inverse_sign) - inverse_sign;
  } else {
    return i < 0 ? i : -i;
  }
}

/**
 * @brief Averages two signed integers in an overflow-safe way.
 *
 * @note The naive average function is erroneous when the sum of the inputs
 *       overflows integer limits; this average works by summing the halves of
 *       the input values and then correcting the sum for rounding.
 *
 * @param a First value.
 * @param b Second value.
 * @return Signed average of the two values, rounded towards zero.
 */
template<typename T>
static inline T Average(T a, T b) {
  static_assert(std::is_integral<T>::value,
                "Average is valid only for integers.");
  static_assert(std::is_signed<T>::value &&
                    ((static_cast<T>(-1) >> 1) == static_cast<T>(-1)),
                "Arithmetic right shift is not available.");
  // shifts divide by two, rounded towards negative infinity
  const T sum_halves = (a >> 1) + (b >> 1);
  // this has error of magnitude one if both are odd
  const T both_odd = (a & b) & 1;
  // round toward zero; add one if one input is odd and sum is negative
  const T round_to_zero = (sum_halves < 0) & (a ^ b);
  // result is sum of halves corrected for rounding
  return sum_halves + both_odd + round_to_zero;
}

/**
 * @brief Gets the sign of a value.
 *
 * @param i Value.
 * @return 1 if @p i is positive, 0 if @p i is 0, and -1 if @p i is negative.
 */
template<typename T>
static inline T SignOf(T i) {
  static_assert(std::is_integral<T>::value,
                "SignOf is valid only for integers.");
  return (i > 0) - (i < 0);
}

#endif  /* BASE_INTEGER_H_ */
