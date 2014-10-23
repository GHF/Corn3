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

#include "motor/inverter_pwm.h"

#include "base/log.h"

InverterPWM::InverterPWM(PWMDriver *pwm_driver)
    : pwm_driver_(pwm_driver) {
}

// Note that all the channels are disabled in the OS driver, and then separately
// configured through the WriteChannel function.
void InverterPWM::Start() {
  pwmStart(pwm_driver_, &kPwmConfig);
  // Enable each channel's output, but put them in disable mode. See comment for
  // the OS driver configuration on the distinction.
  WriteChannel(InverterPWM::kChannelA, 0, false);
  WriteChannel(InverterPWM::kChannelB, 0, false);
  WriteChannel(InverterPWM::kChannelC, 0, false);
  SyncModes();
  constexpr int pwm_frequency = INVERTER_COUNTER_FREQ / INVERTER_PWM_PERIOD / 2;
  LogInfo("Started inverter PWM driver at %d.%d kHz.",
          pwm_frequency / 1000, (pwm_frequency % 1000 + 50) / 100);
}

// Writes channel configurations to preload registers. The widths don't take
// effect until the next timer update, and the channel modes don't take effect
// until a call to SyncModes().
void InverterPWM::WriteChannel(InverterPWM::Channel channel,
                               Width16 width,
                               bool enable) {
  CHECK(channel < InverterPWM::kNumChannels);

  uint32_t ccer = pwm_driver_->tim->CCER;
  switch (channel) {
    case InverterPWM::kChannelC: {
      uint32_t ccmr1 = pwm_driver_->tim->CCMR1;
      ccmr1 &= ~STM32_TIM_CCMR1_OC1M_MASK;
      if (enable) {
        // Set the normal PWM mode.
        ccmr1 |= STM32_TIM_CCMR1_OC1M(6);
        // Enable both PWM channels.
        ccer |= STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC1NE;
      } else {
        // Lock the reference signal to inactive. Since OCxM is preloaded
        // together with channel enable and polarity bits, this takes effect
        // synchronously with them. This is better than just setting the pulse
        // width compare register (CCR) to zero, as that takes effect on counter
        // update events.
        ccmr1 |= STM32_TIM_CCMR1_OC1M(4);
        // Disable the high-side output, which sets the low-side (complementary)
        // output as based on the reference signal. See table "Output control
        // bits for complementary OCx and OCxN channels with break feature" in
        // the reference manual (RM0316).
        ccer &= ~STM32_TIM_CCER_CC1E;
        // Enable the low side output. Even with the OSSR bit set in TIMx_BDTR,
        // having both outputs for a channel disabled results in both outputs
        // putting out high-impedance rather than both inactive.
        ccer |= STM32_TIM_CCER_CC1NE;
      }
      pwm_driver_->tim->CCMR1 = ccmr1;
      // Since the modes don't take effect until SyncModes() is called, update
      // the width regardless of this channel being disabled.
      pwm_driver_->tim->CCR[0] = width;
      break;
    }

    case InverterPWM::kChannelB: {
      uint32_t ccmr1 = pwm_driver_->tim->CCMR1;
      ccmr1 &= ~STM32_TIM_CCMR1_OC2M_MASK;
      if (enable) {
        ccmr1 |= STM32_TIM_CCMR1_OC2M(6);
        ccer |= STM32_TIM_CCER_CC2E | STM32_TIM_CCER_CC2NE;
      } else {
        ccmr1 |= STM32_TIM_CCMR1_OC2M(4);
        ccer &= ~STM32_TIM_CCER_CC2E;
        ccer |= STM32_TIM_CCER_CC2NE;
      }
      pwm_driver_->tim->CCMR1 = ccmr1;
      pwm_driver_->tim->CCR[1] = width;
      break;
    }

    case InverterPWM::kChannelA: {
      uint32_t ccmr2 = pwm_driver_->tim->CCMR2;
      ccmr2 &= ~STM32_TIM_CCMR2_OC3M_MASK;
      if (enable) {
        ccmr2 |= STM32_TIM_CCMR2_OC3M(6);
        ccer |= STM32_TIM_CCER_CC3E | STM32_TIM_CCER_CC3NE;
      } else {
        ccmr2 |= STM32_TIM_CCMR2_OC3M(4);
        ccer &= ~STM32_TIM_CCER_CC3E;
        ccer |= STM32_TIM_CCER_CC3NE;
      }
      pwm_driver_->tim->CCMR2 = ccmr2;
      pwm_driver_->tim->CCR[2] = width;
      break;
    }

    default:
      break;
  }
  pwm_driver_->tim->CCER = ccer;
}

// Generates commutation event, loading preloaded channel configurations.
void InverterPWM::SyncModes() {
  pwm_driver_->tim->EGR |= STM32_TIM_EGR_COMG;
}

// All channels are disabled at the beginning. This results in them not driving
// the PWM pins at all (high impedance), which puts in the inverter in an
// unknown state. The true "inverter disabled" mode is to drive all the PWM pins
// low, so that the inverter is in high impedance at all half bridges.
const PWMConfig InverterPWM::kPwmConfig = { INVERTER_COUNTER_FREQ,
                                            INVERTER_PWM_PERIOD,
                                            nullptr,
                                          { { PWM_OUTPUT_DISABLED, nullptr },
                                            { PWM_OUTPUT_DISABLED, nullptr },
                                            { PWM_OUTPUT_DISABLED, nullptr },
                                            { PWM_OUTPUT_DISABLED, nullptr } },
                                            // Center-aligned (count up & down).
                                            STM32_TIM_CR1_CMS(3),
                                            // Preload channel configurations.
                                            STM32_TIM_CR2_CCPC,
                                            // Drive channel even if disabled.
                                            STM32_TIM_BDTR_OSSR |
                                            // Drive channel if generator idle.
                                                STM32_TIM_BDTR_OSSI |
                                            // Deadtime of 4 * 125 ns.
                                                STM32_TIM_BDTR_DTG(4),
                                            0 };
