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

#ifndef MOTOR_COMMUTATOR_SIX_STEP_H_
#define MOTOR_COMMUTATOR_SIX_STEP_H_

#include "ch.h"

#include "common.h"
#include "base/utility.h"

class RotorInterface;
class InverterInterface;

/**
 * @brief Configures inverter based on rotor angle quantized to six steps and
 *        desired power level (amplitude).
 *
 * @note For six-step commutation, only two inverter phases are driven at a
 *       time to generate a flux vector that is 90 degrees ahead of the six-
 *       quantized rotor angle.
 *       TODO(Xo): Fix the preceding techno-gibberish.
 *
 * @note This class has controllable amplitude, which is the "scale" of the
 *       voltage pushed into the motor phases.
 */
class CommutatorSixStep {
 public:
  /**
   * @brief Creates commutator structure with references to necessary motor
   *        interfaces.
   *
   * @param rotor Rotor angle approximation interface.
   * @param inverter Power stage output interface.
   */
  CommutatorSixStep(RotorInterface *rotor, InverterInterface *inverter);

  /**
   * @brief Performs an update of inverter state and configuration whenever
   *        signaled through @c SignalAngleChange.
   */
  NORETURN void CommutationLoop();

  /**
   * @brief Notifies the commutation logic that amplitude or rotor angle has
   *        changed.
   *
   * @note Must be called under a ChibiOS lock.
   */
  void SignalChange();

  /**
   * @brief Writes the voltage that each driven phase deviates from zero.
   *
   * @param semi_amplitude Amount of bias applied to the driven phase. Absolute
   *                       value must be less than or equal to half of the PWM
   *                       period.
   */
  void WriteAmplitude(Width16Diff semi_amplitude);

 protected:
  RotorInterface * const rotor_;
  InverterInterface * const inverter_;
  Width16 semi_amplitude_;  ///< Width by which driven phases are biased.
  Semaphore semaphore_;  ///< Synchronization for commutation updates.
};

#endif  /* MOTOR_COMMUTATOR_SIX_STEP_H_ */
