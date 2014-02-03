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

#ifndef MOTOR_INVERTER_PWM_H_
#define MOTOR_INVERTER_PWM_H_

#include "hal.h"

#include "inverter_interface.h"

/**
 * @brief Drives a three phase inverter using three pulse width modulation (PWM)
 *        channels.
 *
 * @note Each PWM channel has two complementary outputs that are used as signals
 *       to drive the high- and low-side transistors in a half bridge.
 *
 * @note InverterInterface is used to control the output of the three phase
 *       power stage. The implementation of that using a six-output PWM
 *       generator is a different logical concept, even if the terms "channel"
 *       and "output" are used in both and are closely linked. Be aware that
 *       e.g. disabling a PWM generator channel is not the same as enabling the
 *       inverter phase it corresponds to.
 *
 * @note The actual fraction of a PWM cycle that a channel is driven high or low
 *       is subject to both
 *         1) Deadtime: inactive time inserted between transitions to prevent
 *            both transistors in a half bridge from being on simultaneously.
 *            This gives time to turn a transistor off before its complementary
 *            transistor turns on.
 *       and
 *         2) Maximum on time: the high side transistor can not be permanently
 *            driven on with some gate drivers (i.e. its gate-to-source voltage
 *            leaks and can't be "topped off" by the driver). Also, current
 *            sensing may occur in the off-time of the inverter, which sets a
 *            minimum duration to allow analog to digital converter (ADC)
 *            sampling.
 */
class InverterPWM: public InverterInterface {
 public:
  /**
   * @brief Creates inverter driver structure.
   *
   * @param pwm_driver PWM driver used to generate gate driver input signals.
   */
  InverterPWM(PWMDriver *pwm_driver);

  /**
   * @brief Initializes the PWM driver and configures each channel to put out an
   *        inactive (low) signal, which puts each inverter channel in high
   *        impedance.
   */
  void Start();

  /**
   * @brief Reads PWM period. This is the denominator of the fraction of time
   *        that this phase is on.
   *
   * @note The PWM generator is configured for center-aligned, so that each
   *       cycle is generated twice: once counting up (normal) and once counting
   *       down, during which the signal is "mirrored" in time. So, the active
   *       time for each cycle is adjacent to active time of either the previous
   *       or next cycle (and vice versa for the inactive time). The end result
   *       is that the true generated period is double of the return value.
   *       Pulse widths are also doubled for the same reasons.
   *
   * @return Max number of timer ticks that a pulse width may span.
   */
  Width16 GetPeriod() {
    return pwm_driver_->period;
  }

  /**
   * @brief Loads a channel configuration into hardware registers.
   *
   * @note Neither the state (enabled/disabled) nor the pulse width for a
   *       channel will take effect immediately, as both are "preloaded" by the
   *       hardware. The pulse width will be effective at the next PWM cycle
   *       and the state will be effective when and only if @c SyncModes is
   *       called.
   *
   * @param channel Channel to configure.
   * @param width Duration of the PWM period that this inverter phase is driven
   *              high (if enabled). For the rest of each period, the phase is
   *              driven low. This is subject to inserted deadtime and maximum
   *              on times.
   * @param enable True if the inverter phase is driven; false to put it into a
   *               high impedance state (default).
   */
  void WriteChannel(Channel channel, Width16 width, bool enable = true);

  /**
   * @brief Synchronizes the hardware to the PWM channel states previously
   *        written.
   */
  void SyncModes();

 protected:
  static const PWMConfig kPwmConfig;

  PWMDriver * const pwm_driver_;
};

#endif  /* MOTOR_INVERTER_PWM_H_ */
