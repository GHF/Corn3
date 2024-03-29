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

#include "driver/servo_input.h"

#include <algorithm>

#include "config.h"
#include "base/integer.h"
#include "base/log.h"
#include "base/utility.h"
#include "motor/commutator_six_step.h"

ServoInput::ServoInput(ICUDriver *icu_driver)
    : icu_driver_(icu_driver),
      commutator_six_step_(nullptr),
      num_overflows_(0),
      last_amplitude_(0) {
}

void ServoInput::Start() {
  icu_driver_->self = this;
  LogDebug("Configuring servo input capture at %u Hz...", SERVO_INPUT_ICU_FREQ);
  icuStart(icu_driver_, &kServoIcuConfig);
  icuEnable(icu_driver_);
  LogInfo("Started servo input capture.");
}

constexpr int ServoInput::kInputLow;
constexpr int ServoInput::kInputHigh;
constexpr int ServoInput::kInputDeadband;
constexpr int ServoInput::kInputMargin;
constexpr int ServoInput::kInputSlewLimit;

const ICUConfig ServoInput::kServoIcuConfig = { ICU_INPUT_ACTIVE_HIGH,
                                                SERVO_INPUT_ICU_FREQ,
                                                IcuWidthCallback,
                                                IcuPeriodCallback,
                                                IcuOverflowCallback,
                                                ICU_CHANNEL_1,
                                                0,
                                                ICU_RESET_ON_ACTIVE,
                                                ICU_CHANNEL_1_INPUT_1,
                                                ICU_FILTER_F_1_N_8 };

void ServoInput::HandlePulse(int width, int period, bool valid) {
  if (commutator_six_step_ != nullptr) {
    if ((width < (kInputLow - kInputMargin)) ||
        (width > (kInputHigh + kInputMargin))) {
      valid = false;
    }
    if (valid) {
      const int bounded_command = Clamp(width, kInputLow, kInputHigh);
      const Width16 period_2 = commutator_six_step_->GetMaxAmplitude();
      const Width16Diff amplitude = MapRange(kInputLow,
                                             kInputHigh,
                                             bounded_command,
                                             -period_2,
                                             period_2,
                                             kInputDeadband);
      // Greatest change allowed this period based on slew rate limits.
      const int slew_margin = (kInputSlewLimit * period) /
                              (SERVO_INPUT_ICU_FREQ / 1000);
      const Width16Diff slew_limited_amplitude =
          Clamp<int>(amplitude,
                     last_amplitude_ - slew_margin,
                     last_amplitude_ + slew_margin);
      commutator_six_step_->WriteAmplitude(slew_limited_amplitude);
      last_amplitude_ = slew_limited_amplitude;
      chSysLockFromIsr();
      commutator_six_step_->SetEnable(true);
      commutator_six_step_->SignalChange();
      chSysUnlockFromIsr();
    } else {
      last_amplitude_ = 0;
      chSysLockFromIsr();
      commutator_six_step_->SetEnable(false);
      commutator_six_step_->SignalChange();
      chSysUnlockFromIsr();
    }
  }
}

// Gets width of pulse captured. Disables drive if time between edges overflowed
// the timer counter.
void ServoInput::IcuWidthCallback(ICUDriver *icu_driver) {
  const uint16_t pulse_width = icuGetWidth(icu_driver);
  const uint16_t pulse_period = icuGetPeriod(icu_driver);
  ServoInput * const servo_input = static_cast<ServoInput *>(icu_driver->self);
  if (servo_input->num_overflows_ > 0) {
    servo_input->HandlePulse(-1, -1, false);
  } else {
    servo_input->HandlePulse(pulse_width, pulse_period, true);
  }
}

// Resets counter for timer overflows since pulse positive edge.
void ServoInput::IcuPeriodCallback(ICUDriver *icu_driver) {
  static_cast<ServoInput *>(icu_driver->self)->num_overflows_ = 0;
}

// Disables commutation if more than one overflow has ocurred.
void ServoInput::IcuOverflowCallback(ICUDriver *icu_driver) {
  ServoInput * const servo_input = static_cast<ServoInput *>(icu_driver->self);
  // Increment with saturation.
  servo_input->num_overflows_ =
      std::max(servo_input->num_overflows_, servo_input->num_overflows_ + 1);
  if (servo_input->num_overflows_ > 0) {
    servo_input->HandlePulse(-1, -1, false);
  }
}

int32_t ServoInput::MapRange(int32_t in_low, int32_t in_high, int32_t value,
                             int32_t out_low, int32_t out_high,
                             int32_t deadband) {
  // Center the input range on zero.
  const int32_t in_center = Average(in_low, in_high);
  int32_t centered_input = value - in_center;

  // Cut away the deadband from the centered input.
  if (Nabs(centered_input) > -deadband) {
    centered_input = 0;
  } else {
    centered_input -= SignOf(centered_input) * deadband;
  }

  // Compute the length of input and output ranges.
  const int32_t inScale = in_high - in_low - 2 * deadband;
  const int32_t outScale = out_high - out_low;
  const int32_t outCenter = Average(out_low, out_high);

  // Scale by output to input ratio, then shift from zero into range.
  // WARNING: this computation can easily overflow!
  const int32_t outValue = centered_input * outScale / inScale + outCenter;

  // Clamp output within range.
  return std::max(out_low, std::min(out_high, outValue));
}
