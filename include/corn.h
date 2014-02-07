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

#ifndef CORN_H_
#define CORN_H_

#include <cstddef>

#include "hal.h"

#include "base/utility.h"
#include "driver/DRV8303.h"
#include "driver/servo_input.h"
#include "motor/rotor_hall.h"
#include "motor/inverter_pwm.h"
#include "motor/commutator_six_step.h"

/**
 * @brief Entry point, initialization, and main loop for all functionality.
 *
 * @note ONLY CONSTRUCT ONE OF THESE, AND NEVER DESTROY IT. This is not enforced
 *       with a singleton mechanism to keep the code simple.
 */
class Corn {
 public:
  /**
   * @brief Creates system variables and structures.
   */
  Corn();

  /**
   * @brief Initializes subsystems into known states.
   */
  void Start();

  /**
   * @brief Runs Corntroller main functionality.
   */
  NORETURN void MainLoop();

 protected:
  static const SerialConfig kDebugSerialConfig;  ///< Serial port configuration.
  static void (* const system_reset_function)(void);  ///< NVIC_SystemReset.

  /**
   * @brief Resets the system upon command through the debug serial channel.
   */
  NORETURN static msg_t ThreadReset(void *arg);

  /**
   * @brief Blinks an LED at 1 Hz at low priority to show system is functional
   *        and has idle time.
   */
  NORETURN static msg_t ThreadHeartbeat(void *arg);

  static WORKING_AREA(wa_reset_, 128);      ///< Reset thread working area.
  static WORKING_AREA(wa_heartbeat_, 128);  ///< Heartbeat thread working area.
  static WORKING_AREA(wa_hall_, 1024);      ///< Hall thread working area.

  RotorHall rotor_hall_;  ///< Hall sensor signal handling driver.
  InverterPWM inverter_pwm_;  ///< 3-phase inverter driver.
  DRV8303 drv8303_;  ///< Gate driver and current sense amplifier driver.
  CommutatorSixStep commutator_six_step_;  ///< Motor output sequencer.
  ServoInput servo_input_;  ///< Servo pulse input from R/C receiver.
};

#endif  /* CORN_H_ */
