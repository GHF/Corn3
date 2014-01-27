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

/**
 * @file Declares RotorInterface, an interface that reports the state of the
 *       rotor.
 */

#ifndef MOTOR_ROTOR_INTERFACE_H_
#define MOTOR_ROTOR_INTERFACE_H_

#include "motor/common.h"

/**
 * @brief Reports on the angular position and velocity of the rotor.
 */
class RotorInterface {
 public:
  virtual ~RotorInterface() {}

  /**
   * @brief Computes and retrieves the rotor angle.
   *
   * @param angle Pointer that the current rotor position (if valid) is written
   *              to.
   * @return True if the angle is valid and was written to the output param.
   */
  virtual bool ComputeAngle(Angle16 *angle) = 0;

  /**
   * @brief Computes and retrieves the rotor angular velocity.
   *
   * @param angle Pointer that the current rotor angular velocity (if valid) is
   *              written to.
   * @return True if the velocity is valid and was written to the output param.
   */
  virtual bool ComputeVelocity(Velocity32 *velocity) = 0;
};

#endif  /* MOTOR_ROTOR_INTERFACE_H_ */
