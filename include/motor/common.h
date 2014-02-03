/*
 * Corn3 - Copyright (C) 2014 Xo Wang
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

#ifndef MOTOR_COMMON_H_
#define MOTOR_COMMON_H_

#include <cstdint>

/**
 * @brief Data type to represent electrical angle.
 *
 * @note The Corntroller angle convention: the fixed reference frame is aligned
 *       to phase A. That is, the rotor is at angle 0 when it is locked by
 *       current going into motor lead A and leaving leads B and C.
 *
 * @note This is an fixed-point format where bit values 0 to 2^16 represent 0 to
 *       2 * pi. Of course 2^16 would overflow to 0, a convenient feature for
 *       angles.
 */
typedef uint16_t Angle16;

/**
 * @brief Data type to represent the difference between two fixed-point angles.
 */
typedef int16_t Angle16Diff;

/**
 * @brief Data type to represent angular velocity.
 *
 * @note The Corntroller direction convention: positive direction is counter-
 *       clockwise.
 */
typedef float Velocity32;

/**
 * @brief Data type to represent width of inverter output. A value of this type
 *        is the number of counts by a PWM generator clock, and is meaningful
 *        only if it does not exceed the period of the clock.
 */
typedef uint16_t Width16;

/**
 * @brief Convert degrees to fixed-point, with rounding.
 *
 * @param degrees Angle in degrees.
 * @return Angle in fixed-point angular format.
 */
static inline constexpr Angle16 DegreesToAngle16(unsigned degrees) {
  return (degrees * (1 << 16) + 180) / 360;
}

/**
 * @brief Convert fixed-point to degrees, with rounding.
 *
 * @param angle Angle in fixed-point angular format.
 * @return Angle in degrees from 0 to 360 (inclusive).
 */
static inline constexpr unsigned Angle16ToDegrees(Angle16 angle) {
  return (unsigned(angle) * 360U + (1 << 15)) >> 16;
}

/**
 * @brief Convert angular velocity to revolutions per minute (RPM), with
 *        rounding.
 *
 * @param velocity Angular velocity in floating point format.
 * @return Angular velocity in RPM.
 */
static inline constexpr long Velocity32ToRPM(Velocity32 velocity) {
  return (60 * static_cast<long>(velocity) + (1 << 15)) >> 16;
}

#endif  /* MOTOR_COMMON_H_ */
