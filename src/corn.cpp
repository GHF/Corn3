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

#include "corn.h"

#include "ch.h"
#include "hal.h"

#include "config.h"
#include "base/log.h"
#include "base/utility.h"
#include "version/version.h"

// Does whole-system initialization, and uses many static variables as if they
// were instance variables, because this object is constructed under the
// assumption that only one will be made.
Corn::Corn()
    : rotor_hall_(&HALL_ICU, &wa_hall_, sizeof(wa_hall_)),
      inverter_pwm_(&INVERTER_PWM),
      drv8303_(&DRV_SPI),
      commutator_six_step_(&rotor_hall_, &inverter_pwm_),
      servo_input_(&SERVO_INPUT_ICU) {
}

// Sequences bootup. Calls initialization methods of subsystems.
void Corn::Start() {
  // Clear error LED to show start of initialization.
  INVOKE(palClearPad, GPIO_LED_ERROR);

  // Setup debug serial driver.
  sdStart(&DEBUG_SERIAL, &kDebugSerialConfig);

  // Start reset handler thread.
  chThdCreateStatic(wa_reset_,
                    sizeof(wa_reset_),
                    NORMALPRIO + 1,
                    ThreadReset,
                    nullptr);

  // Print welcome with OS mechanism, so there is output even if logging breaks.
  const uint8_t welcome_msg[] = ANSI_RESET "\r\n" BOARD_NAME "\r\n";
  sdWrite(&DEBUG_SERIAL, welcome_msg, sizeof(welcome_msg));

  // Print startup message.
  LogInfo("Firmware version %s built %s.", g_build_version, g_build_time);

  // Start heartbeat thread.
  chThdCreateStatic(wa_heartbeat_,
                    sizeof(wa_heartbeat_),
                    LOWPRIO,
                    ThreadHeartbeat,
                    nullptr);

  // Start three-phase PWM driver.
  inverter_pwm_.Start();

  // Start gate driver and current sense amplifiers driver.
  drv8303_.Start();
  drv8303_.ResetSoft();

  // Start hall sensor rotor angle driver.
  rotor_hall_.SetCommutatorSixStep(&commutator_six_step_);
  rotor_hall_.Start();

  // Start servo pulse input driver.
  servo_input_.SetCommutatorSixStep(&commutator_six_step_);
  servo_input_.Start();

  // Start error polling thread.
  chThdCreateStatic(wa_error_,
                    sizeof(wa_error_),
                    LOWPRIO + 1,
                    ThreadError,
                    &drv8303_);
  LogInfo("Started gate driver error polling.");

  // Signal end of initialization.
  LogInfo("Initialized in %lu ms.", chTimeNow() * 1000 / CH_FREQUENCY);
  INVOKE(palClearPad, GPIO_LED_INIT);
}

NORETURN void Corn::MainLoop() {
  commutator_six_step_.CommutationLoop();
}

// Serial settings for 8N1 at configured baud rate, with no flow control.
const SerialConfig Corn::kDebugSerialConfig = { DEBUG_BAUDRATE,
                                                0,
                                                USART_CR2_STOP1_BITS,
                                                0 };

// Resets the system if two ^C characters are received in succession.
NORETURN msg_t Corn::ThreadReset(void *arg) {
  (void) arg;

  chRegSetThreadName("reset");

  bool etx_received = false;
  while (true) {
    const uint8_t c = chnGetTimeout(&DEBUG_SERIAL, TIME_INFINITE);
    if (c == '\x03') {
      if (etx_received) {
        break;
      } else {
        etx_received = true;
      }
    } else {
      etx_received = false;
    }
    // Echo character back.
    sdPut(&DEBUG_SERIAL, c);
  }
  SystemReset();
}

// Uses delays to achieve a 1 Hz blink rate, so the timing will not be exact.
NORETURN msg_t Corn::ThreadHeartbeat(void *arg) {
  (void) arg;

  chRegSetThreadName("heartbeat");

  while (true) {
    INVOKE(palClearPad, GPIO_LED_HEARTBEAT);
    chThdSleepMilliseconds(900);
    INVOKE(palSetPad, GPIO_LED_HEARTBEAT);
    chThdSleepMilliseconds(100);
  }

  chThdExit(0);
}

// Polls the DRV8303 for errors, which also clears any faults.
NORETURN msg_t Corn::ThreadError(void *drv8303_pointer) {
  chRegSetThreadName("error");

  DRV8303 * const drv8303 = static_cast<DRV8303 *>(drv8303_pointer);
  while (true) {
    drv8303->CheckFaults();
    chThdSleepMilliseconds(100);
  }

  chThdExit(0);
}

// Thread working area definitions.
// TODO(Xo): Define the stack sizes in a single location.
WORKING_AREA(Corn::wa_reset_, 128);
WORKING_AREA(Corn::wa_heartbeat_, 128);
WORKING_AREA(Corn::wa_hall_, 1024);
WORKING_AREA(Corn::wa_error_, 512);
