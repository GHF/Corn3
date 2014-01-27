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

#ifndef MOTOR_ROTOR_HALL_H_
#define MOTOR_ROTOR_HALL_H_

#include "ch.h"
#include "hal.h"

#include "motor/rotor_interface.h"

/**
 * @brief Hall sensor rotor angle sensor driver.
 *
 * @note Timing is implemented using the ChibiOS input capture unit (ICU)
 *       driver, which uses the STM32 timers' compare/capture functionality.
 *       Hall sensors are read with GPIO.
 */
class RotorHall: public RotorInterface {
 public:
  /**
   * @brief Initializes the hall sensor driver, setting up interrupts and and a
   *        thread.
   *
   * @param icu_driver ICU driver used to capture hall state transitions.
   * @param wa_update Working area for the hall transition processing thread.
   * @param wa_size Size of @p wa_update.
   */
  RotorHall(ICUDriver *icu_driver, void *wa_update, size_t wa_size);

  /**
   * @brief Computes the current rotor angle based on its hall state.
   *
   * @param angle Output; angle from hall state.
   * @return True if hall state is valid and the corresponding rotor angle was
   *         written to @p angle.
   */
  bool ComputeAngle(Angle16 *angle);

  /**
   * @brief Computes the current rotor velocity based on time between hall state
   *        transitions and direction of rotation.
   *
   * @param velocity Output; velocity of rotor.
   * @return True if computed angular velocity is valid and was written to
   *         @p velocity.
   */
  bool ComputeVelocity(Velocity32 *velocity);

 protected:
  /**
   * @brief Possible hall states based on a 3-bit field of [HALL_A..HALL_C].
   *
   * @note Consecutive hall state values do not necessarily represent continuous
   *       motion, nor vice versa.
   */
  enum HallState {
    kHallInvalid000 = 0x0,  ///< kHallInvalid000 Null state of three clear bits.
    kHall0Deg       = 0x1,  ///< kHall0Deg   Rotor at 330 to 30 degrees.
    kHall60Deg      = 0x5,  ///< kHall60Deg  Rotor at 30 to 90 degrees.
    kHall120Deg     = 0x4,  ///< kHall120Deg Rotor at 90 to 150 degrees.
    kHall180Deg     = 0x6,  ///< kHall180Deg Rotor at 150 to 210 degrees.
    kHall240Deg     = 0x2,  ///< kHall240Deg Rotor at 210 to 270 degrees.
    kHall300Deg     = 0x3,  ///< kHall300Deg Rotor at 270 to 330 degrees.
    kHallInvalid111 = 0x7,  ///< kHallInvalid111 Null state of three set bits.
    kHallNumStates          ///< kHallNumStates Number of hall states.
  };

  /**
   * @brief Configuration options for OS driver.
   */
  static const ICUConfig kHallICUConfig;

  /**
   * @brief Relates hall state to angular position.
   */
  static const Angle16 kHallAngles[kHallNumStates];

  /**
   * @brief Looks up hall state to next hall state (i.e., hall sensor output
   *        after rotor has rotated 60 degrees).
   */
  static const HallState kNextHallStates[kHallNumStates];

  /**
   * @brief Reads the hall state from GPIO.
   *
   * @return 3-bit field hall state with sensor A as LSB, sensor B as bit 1, and
   *         sensor C as MSB.
   */
  HallState ReadHallState();

  /**
   * @brief Computes time between hall state transitions.
   *
   * @return Counts of the ICU timer between the last two hall state changes or
   *         between the last state change and now, whichever is greater. Will
   *         be saturated to maximum value if
   */
  icucnt_t TransitionDeltaT();

  /**
   * @brief Processes a hall state transition and wakes up the update thread.
   *
   * @note Can only be called from an ISR.
   *
   * @param count Number of ICU timer counts.
   */
  void HandleEdge(icucnt_t count);

  /**
   * @brief Computes and updates observed state using information passed by ISR.
   */
  void ThreadHall();

  /**
   * @brief Invokes @c ThreadHall; used as a thread function.
   *
   * @param rotor_hall Pointer to an instance of this class.
   * @return Should never return.
   */
  static msg_t ThreadHallWrapper(void *rotor_hall);

  /**
   * @brief Checks if a hall state is valid.
   *
   * @param hall_state 3-bit hall state bitfield.
   * @return True if @p hall_state can be converted to an rotor angle.
   */
  static bool HallStateValid(HallState hall_state) {
    return hall_state != kHallInvalid000 &&
           hall_state != kHallInvalid111 &&
           hall_state < kHallNumStates;
  }

  /**
   * @brief Compute angular speed from the timer counts elapsed between hall
   *        state transitions.
   *
   * @param counts_elapsed Timer counts taken to rotate 60 degrees.
   * @return Angular speed in floating point format. Always positive.
   */
  static Velocity32 ComputeSpeed(icucnt_t counts_elapsed) {
    return Velocity32(kHallICUConfig.frequency / 6) * Velocity32(1 << 16) /
           counts_elapsed;
  }

  /**
   * @brief Reads the ICU capture value and passes it to the edge handler.
   *
   * @param icup Pointer to ICU driver that originated the capture event.
   */
  static void ICUWidthCallback(ICUDriver *icup);

  /**
   * @brief Reads the ICU capture value and passes it to the edge handler.
   *
   * @param icup Pointer to ICU driver that originated the capture event.
   */
  static void ICUPeriodCallback(ICUDriver *icup);

  /**
   * @brief Sets the state as having overflowed the timer since last hall edge.
   *
   * @param icup Pointer to ICU driver that originated the overflow event.
   */
  static void ICUOverflowCallback(ICUDriver *icup);

  ICUDriver * const icu_driver_;  ///< Points to OS capture driver.
  Semaphore semaphore_update_;  ///< Synchronizes update thread to ISR.
  Thread * const thread_hall_;  ///< Points to state update thread.

  bool timer_overflowed_;  ///< True if timer overflowed since last edge.
  HallState hall_state_;  ///< Current hall state bitfield.
  HallState last_hall_state_;  ///< Previous hall state bitfield.
  icucnt_t counts_elapsed_;  ///< ICU timer ticks between last two edges.
  Velocity32 velocity_;  ///< Angular velocity of rotor.
  int direction_;  ///< Positive for CCW, negative for CW, and 0 for fault.
};

#endif  /* MOTOR_ROTOR_HALL_H_ */
