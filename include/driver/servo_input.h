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

#ifndef DRIVER_SERVO_INPUT_H_
#define DRIVER_SERVO_INPUT_H_

#include "hal.h"

#include "config.h"

class CommutatorSixStep;

/**
 * @brief Driver for reading and processing servo PWM signals into throttle
 *        commands, and optionally sending them to the motor commutator.
 */
class ServoInput {
 public:
  /**
   * @brief Creates a driver structure for handling RC servo pulse input as
   *        a throttle for the commutation.
   *
   * @param icu_driver OS driver for capturing servo input edges.
   */
  ServoInput(ICUDriver *icu_driver);

  /**
   * @brief Starts capturing and processing servo PWM input.
   */
  void Start();

  /**
   * @brief Connects a commutator as an output for the servo signals being read.
   *
   * @param commutator_six_step Motor driving sequencer.
   */
  void SetCommutatorSixStep(CommutatorSixStep *commutator_six_step) {
    commutator_six_step_ = commutator_six_step;
  }

 protected:
  /// Lower bound of pulse width.
  static constexpr int kInputLow = SERVO_INPUT_MIN_COMMAND;
  /// Upper bound of pulse width.
  static constexpr int kInputHigh = SERVO_INPUT_MAX_COMMAND;
  /// Deadband of pulse width.
  static constexpr int kInputDeadband = SERVO_INPUT_DEADBAND;
  /// Margin past the bounds for which pulse widths are rejected.
  static constexpr int kInputMargin = SERVO_INPUT_MARGIN;
  /// Servo input capture settings.
  static const ICUConfig kServoIcuConfig;

  /**
   * @brief Handles servo pulse event.
   *
   * @param width Width of the pulse captured. Not used if @p valid is false.
   * @param valid True if @p width is valid. Invalid can mean a pulse glitch,
   *              timeout, etc.
   */
  void HandlePulse(int width, bool valid);

  /**
   * @brief Handles servo pulse input falling edges.
   *
   * @param icu_driver Input capture OS driver that originated the callback.
   */
  static void IcuWidthCallback(ICUDriver *icu_driver);

  /**
   * @brief Handles servo pulse input rising edges.
   *
   * @param icu_driver Input capture OS driver that originated the callback.
   */
  static void IcuPeriodCallback(ICUDriver *icu_driver);

  /**
   * @brief Handles servo pulse input unit timer overflows.
   *
   * @param icu_driver Input capture OS driver that originated the callback.
   */
  static void IcuOverflowCallback(ICUDriver *icu_driver);

  /**
   * @brief Maps one range of values linearly to another, with deadband.
   *
   * @note This does not check if inputs are valid.
   *
   * @note Deadband is a region at the center of the input range that is mapped
   *       to the center of the output range. Also, the rest of the input range
   *       will start at the extents of the deadband.
   *
   * @note Care must be taken to not exceed integer limits in this computation!
   *
   * TODO(Xo): Describe conditions that cause integer overflow errors.
   *
   * @param in_low Input range lower bound. @p value can not be less than this.
   * @param in_high Input range upper bound. @p value can not exceed this. Must
   *                be greater than @p in_low by twice @p deadband.
   * @param value Input value to map.
   * @param out_low Output range lower bound.
   * @param out_high Output range upper bound. Must be greater than @p out_low.
   * @param deadband Distance from center of input range for which all input
   *  values are mapped to "zero" (center of the output range).
   * @return Mapped input value after adjusting for deadband.
   */
  static int32_t MapRange(int32_t in_low, int32_t in_high, int32_t value,
                          int32_t out_low, int32_t out_high, int32_t deadband);

  ICUDriver * const icu_driver_;  ///< Timer input capture driver.
  CommutatorSixStep *commutator_six_step_;  ///< Servo commands signal sink.
  int num_overflows_;  ///< Times the timer overflowed since last edge.
};

#endif  /* DRIVER_SERVO_INPUT_H_ */
