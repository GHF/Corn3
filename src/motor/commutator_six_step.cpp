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

#include "motor/commutator_six_step.h"

#include <cstdlib>
#include <algorithm>

#include "base/logging.h"
#include "motor/rotor_interface.h"
#include "motor/inverter_interface.h"

CommutatorSixStep::CommutatorSixStep(RotorInterface *rotor,
                                     InverterInterface *inverter)
    : rotor_(rotor),
      inverter_(inverter),
      semi_amplitude_(0),
      semaphore_(_SEMAPHORE_DATA(semaphore_, 0)) {
}

// Classifies the rotor angle into one of six buckets, then computes the channel
// output modes and widths to generate a flux vector perpendicular to the center
// of the bucket. Finally, repeat when a "state updated" signal is received.
NORETURN void CommutatorSixStep::CommutationLoop() {
  while (true) {
    Angle16 rotor_angle;
    if (!rotor_->ComputeAngle(&rotor_angle)) {
      LogError("Rotor angle could not be read.");
      // Disable inverter.
      inverter_->WriteChannel(InverterInterface::kChannelA, 0, false);
      inverter_->WriteChannel(InverterInterface::kChannelB, 0, false);
      inverter_->WriteChannel(InverterInterface::kChannelC, 0, false);
    } else if (semi_amplitude_ == 0) {
      LogDebug("Zero throttle; maximum braking.");
      // Switch all low-side transistors on to short out phases (full braking).
      inverter_->WriteChannel(InverterInterface::kChannelA, 0, true);
      inverter_->WriteChannel(InverterInterface::kChannelB, 0, true);
      inverter_->WriteChannel(InverterInterface::kChannelC, 0, true);
    } else {
      // Advance by a half step so that the six steps are split along the 0 to
      // 180 degrees axis. This makes the following arithmetic simpler.
      rotor_angle += DegreesToAngle16(30);

      Width16 period_2 = inverter_->GetPeriod() / 2;
      Width16 pos_width = period_2 + semi_amplitude_;
      Width16 neg_width = period_2 - semi_amplitude_;

      // Exploit the symmetry of the commutation: in the range
      // 150 deg <= rotor position < 330 deg, the commutation is similar to the
      // other half circle of rotor positions, except with reversed polarities.
      if (rotor_angle >= DegreesToAngle16(180)) {
        // This reverse the polarities of the two driven phases.
        std::swap(pos_width, neg_width);
        // Map this half of rotor angles to the normal polarity half.
        rotor_angle -= DegreesToAngle16(180);
      }

      // Use angle to search for the correct commutation step.
      if (rotor_angle < DegreesToAngle16(60)) {
        // 330 deg <= rotor position <  30 deg or
        // 150 deg <= rotor position < 210 deg
        inverter_->WriteChannel(InverterInterface::kChannelA, period_2, false);
        inverter_->WriteChannel(InverterInterface::kChannelB, pos_width, true);
        inverter_->WriteChannel(InverterInterface::kChannelC, neg_width, true);
        LogDebug("Aoff B+ C-");
      } else if (rotor_angle < DegreesToAngle16(120)) {
        //  30 deg <= rotor position <  90 deg or
        // 210 deg <= rotor position < 270 deg
        inverter_->WriteChannel(InverterInterface::kChannelA, neg_width, true);
        inverter_->WriteChannel(InverterInterface::kChannelB, pos_width, true);
        inverter_->WriteChannel(InverterInterface::kChannelC, period_2, false);
        LogDebug("A- B+ Coff");
      } else {
        //  90 deg <= rotor position < 150 deg or
        // 270 deg <= rotor position < 330 deg
        inverter_->WriteChannel(InverterInterface::kChannelA, neg_width, true);
        inverter_->WriteChannel(InverterInterface::kChannelB, period_2, false);
        inverter_->WriteChannel(InverterInterface::kChannelC, pos_width, true);
        LogDebug("A- Boff C+");
      }
    }
    inverter_->SyncModes();
    // Wait for rotor angle to be updated.
    chSemWait(&semaphore_);
  }
}

void CommutatorSixStep::SignalChange() {
  chSemSignalI(&semaphore_);
}

void CommutatorSixStep::WriteAmplitude(Width16Diff semi_amplitude) {
  CHECK(std::abs(semi_amplitude) <= inverter_->GetPeriod() / 2);
  semi_amplitude_ = semi_amplitude;
}

Width16Diff CommutatorSixStep::GetMaxAmplitude() {
  return inverter_->GetPeriod() / 2;
}
