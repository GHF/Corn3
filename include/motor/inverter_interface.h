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
 * @file Declares InverterInterface, an interface that sets the state of the
 *       three phase inverter.
 */

#ifndef MOTOR_INVERTER_INTERFACE_H_
#define MOTOR_INVERTER_INTERFACE_H_

#include "motor/common.h"

/**
 * @brief Loads and synchronizes the pulse widths output by a three phase
 *        inverter.
 */
class InverterInterface {
 public:
  enum Channel {
    kChannelA,
    kChannelB,
    kChannelC,
    kNumChannels
  };

  virtual ~InverterInterface() {
  }

  /**
   * @brief Gets the pulse width modulation (PWM) generator period.
   *
   * @note This is one more than the highest value that the PWM counter will
   *       count to.
   *
   * @note The PWM clock and PWM counter refer to different concepts and they
   *       do not necessarily have the same frequency.
   *
   * @return Period of PWM, as counts of PWM clocks.
   */
  virtual Width16 GetPeriod() = 0;

  /**
   * @brief Loads the state (output or high impedance) and active time (pulse
   *        width) into an inverter channel.
   *
   * @note When disabled, a channel will be in high impedance. When enabled
   *
   * @note This does not necessarily immediately change the status a channel.
   *       Instead the new widths and states may be loaded at a later time, e.g.
   *       to simultaneous configure all channels, and not necessarily together.
   *       So, even a disabled channel should have some valid width, as it may
   *       continue to generate a pulse of that width until its state takes
   *       effect.
   *
   * @param channel Channel to write to.
   * @param width Duration of each PWM period that channel is driven high.
   * @param enable True if channel is driven (i.e. not high impedance).
   */
  virtual void WriteChannel(Channel channel,
                            Width16 width,
                            bool enable = true) = 0;

  /**
   * @brief Forces mode changes to channels to take effect immediately.
   */
  virtual void SyncModes() = 0;
};

#endif  /* MOTOR_INVERTER_INTERFACE_H_ */
