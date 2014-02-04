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

#include "motor/rotor_hall.h"

#include <cstdlib>
#include <limits>

#include "ch.h"
#include "hal.h"

#include "config.h"
#include "base/logging.h"
#include "base/utility.h"
#include "motor/commutator_six_step.h"

// Sets up rotor state, and launches thread that computes state from hall sensor
// signal changes.
RotorHall::RotorHall(ICUDriver *icu_driver, void *wa_update, size_t wa_size)
    : icu_driver_(icu_driver),
      semaphore_update_(_SEMAPHORE_DATA(semaphore_update_, 0)),
      thread_hall_(chThdCreateStatic(wa_update,
                                     wa_size,
                                     NORMALPRIO,
                                     ThreadHallWrapper,
                                     this)),
      commutator_six_step_(nullptr),
      timer_overflowed_(true),
      hall_state_(kHallNumStates),
      last_hall_state_(kHallNumStates),
      counts_elapsed_(0) {
}

// Initializes ICU driver, which enables hall sensor signal edge interrupts.
void RotorHall::Start() {
  // Setup hall sensor input capture.
  icu_driver_->rotor_hall = this;
  LogDebug("Configuring hall input capture at %u Hz...", HALL_ICU_FREQ);
  icuStart(icu_driver_, &kHallICUConfig);
  icuEnable(icu_driver_);
  LogInfo("Started hall input capture.");
}

// Converts current hall state to angle by lookup table. If the stored state is
// invalid, then make an attempt to read the current state before aborting.
// TODO(Xo): Extrapolate angle using velocity.
bool RotorHall::ComputeAngle(Angle16 *angle) {
  if (!HallStateValid(hall_state_)) {
    hall_state_ = ReadHallState();
    if (!HallStateValid(hall_state_)) {
      LogDebug("hall_state_ invalid (%x)", hall_state_);
      return false;
    }
  }

  *angle = kHallAngles[hall_state_];
  return true;
}

// Retrieves currently precomputed velocity; if the value is no longer fresh,
// recompute it and update the cached velocity.
bool RotorHall::ComputeVelocity(Velocity32 *velocity) {
  if (counts_elapsed_ == 0) {
    LogDebug("counts_elapsed_ = 0");
    return false;
  }

  if (direction_ == 0) {
    LogDebug("direction_ = 0");
    return false;
  }

  const icucnt_t delta_t = TransitionDeltaT();
  if (delta_t != counts_elapsed_) {
    // Recompute velocity if time delta is no longer up to date.
    const Velocity32 velocity_magnitude = ComputeSpeed(delta_t);
    velocity_ = direction_ > 0 ? velocity_magnitude : -velocity_magnitude;
  }
  *velocity = velocity_;
  return true;
}

// Configures the ICU for capturing the edges on channel 1, which is set to be
// the XOR of the three hall sensor signals. So any normal hall transition (only
// a single bit change) creates an interrupt.
//
// Note the custom fields in here for Corn modified version of the driver as
// well as a pointer to this driver in @c ICUDriver.
const ICUConfig RotorHall::kHallICUConfig = { ICU_INPUT_ACTIVE_HIGH,
                                              HALL_ICU_FREQ,
                                              ICUWidthCallback,
                                              ICUPeriodCallback,
                                              nullptr,
                                              ICU_CHANNEL_1,
                                              0,
                                              ICU_RESET_ON_CH1_EDGE,
                                              ICU_CHANNEL_1_XOR_123 };

// Lookup table from hall state bitfield to fixed-point angle.
// Note that they are out of angular order because the hall states are in
// integer order.
const Angle16 RotorHall::kHallAngles[kHallNumStates] = { 0,
                                                         DegreesToAngle16(180),
                                                         DegreesToAngle16(60),
                                                         DegreesToAngle16(120),
                                                         DegreesToAngle16(300),
                                                         DegreesToAngle16(240),
                                                         DegreesToAngle16(0),
                                                         0};

// Lookup table from hall state to the following hall state, assuming positive
// direction of rotation (i.e. counter-clockwise).
const RotorHall::HallState RotorHall::kNextHallStates[kHallNumStates] = {
    kHallInvalid000,
    kHall240Deg,
    kHall120Deg,
    kHall180Deg,
    kHall0Deg,
    kHall300Deg,
    kHall60Deg,
    kHallInvalid111 };

// Reads the hall bitfield from the GPIO pads.
// TODO(Xo): Make this instance-specific by storing GPIO parameters.
RotorHall::HallState RotorHall::ReadHallState() {
  // Use a look up table to reverse the order of the hall bits.
  // TODO(Xo): Change the other look up tables so this isn't necessary.
  static const uint8_t kBitReverses[8] = { 0, 4, 2, 6, 1, 5, 3, 7 };
  const uint8_t reversed_hall_signals = INVOKE(palReadGroup, GPIO_GROUP_HALL);
  return static_cast<RotorHall::HallState>(kBitReverses[reversed_hall_signals]);
}

// Computes the most up-to-date estimate of time between hall states.
//
// If the time since the last edge is less than the time between the last two
// edges, then return the latter. Otherwise, return former, but saturate to max
// value possible if the timer has overflowed.
icucnt_t RotorHall::TransitionDeltaT() {
  if (timer_overflowed_) {
    return std::numeric_limits<icucnt_t>::max();
  } else {
    const icucnt_t current_count = icu_driver_->tim->CNT;
    if (current_count > counts_elapsed_) {
      return current_count;
    } else {
      return counts_elapsed_;
    }
  }
}

// Handles the interrupt generated by ICU detecting an edge. The critical parts
// of state are advanced under a system lock to avoid corruption and to be able
// to signal the update thread.
void RotorHall::HandleEdge(icucnt_t count) {
  // Filter very short input spikes, which generate edges in rapid succession.
  // TODO(Xo): Use hardware filtering.
  const HallState new_hall_state = ReadHallState();
  if (new_hall_state == hall_state_) {
    return;
  }

  chSysLockFromIsr();
  // Atomically update the state variables using the latest hall signal edge.
  last_hall_state_ = hall_state_;
  hall_state_ = new_hall_state;
  counts_elapsed_ = count;
  timer_overflowed_ = false;
  // Signals the update thread that state variables have changed.
  chSemSignalI(&semaphore_update_);
  // Signal change to commutator.
  if (commutator_six_step_ != nullptr) {
    commutator_six_step_->SignalChange();
  }
  chSysUnlockFromIsr();

  INVOKE(palTogglePad, GPIO_LED_HALL);
}

// Runs as needed to process events signaled by the edge ISR. So, heavy
// computation is deferred to this thread and don't lock up the system. This
// reduces interrupt jitter and also allows logging (since logging isn't
// possible from an ISR).
NORETURN void RotorHall::ThreadHall() {
  while (true) {
    // Wait for a hall event to wake this thread.
    chSysLock();
    chSemWaitS(&semaphore_update_);
    // Woken by ICU ISR. A hall edge has changed sensor state.
    chSysUnlock();

    Velocity32 velocity_magnitude = 0.f;
    if (counts_elapsed_ != 0) {
      velocity_magnitude = ComputeSpeed(counts_elapsed_);
    }

    if (HallStateValid(hall_state_) && HallStateValid(last_hall_state_)) {
      if (hall_state_ == kNextHallStates[last_hall_state_]) {
        velocity_ = velocity_magnitude;
        direction_ = 1;
      } else if (last_hall_state_ == kNextHallStates[hall_state_]) {
        velocity_ = -velocity_magnitude;
        direction_ = -1;
      } else {
        direction_ = 0;
        velocity_ = 0.f;
        LogError("Glitch transition (%x -> %x).",
                 last_hall_state_, hall_state_);
      }
    } else {
      direction_ = 0;
      velocity_ = 0.f;
      LogWarning("Invalid transition (%x -> %x).",
                 last_hall_state_, hall_state_);
    }

    LogDebug("New state: %3u degrees @ %ld RPM.",
             Angle16ToDegrees(kHallAngles[hall_state_]),
             Velocity32ToRPM(velocity_));
  }
}

// Non-member function to pass to thread creation.
NORETURN msg_t RotorHall::ThreadHallWrapper(void *rotor_hall) {
  chRegSetThreadName("hall");
  static_cast<RotorHall *>(rotor_hall)->ThreadHall();
  chThdExit(0);
}

// The ICU driver has different callbacks for rising and falling edges, which
// are stored in different registers. This redirects rising edge interrupts to
// the edge handler.
void RotorHall::ICUWidthCallback(ICUDriver *icup) {
  const icucnt_t count = icuGetWidth(icup);
  static_cast<RotorHall *>(icup->rotor_hall)->HandleEdge(count);
}

// Redirects rising edge interrupts to the edge handler.
void RotorHall::ICUPeriodCallback(ICUDriver *icup) {
  const icucnt_t count = icuGetPeriod(icup);
  static_cast<RotorHall *>(icup->rotor_hall)->HandleEdge(count);
}

void RotorHall::ICUOverflowCallback(ICUDriver *icup) {
  static_cast<RotorHall *>(icup->rotor_hall)->timer_overflowed_ = true;
}
