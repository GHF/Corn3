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

#include <algorithm>

#include "ch.h"
#include "hal.h"

#include "config.h"
#include "base/logging.h"
#include "base/utility.h"
#include "driver/DRV8303.h"
#include "version/version.h"
#include "motor/rotor_hall.h"
#include "motor/inverter_pwm.h"

// Pointer to the normally-inlined system reset so its definition is linked.
// This may still be optimized out unless referenced somewhere.
void (*g_system_reset_function)(void) = NVIC_SystemReset;

// Reset thread working area and function.
static WORKING_AREA(wa_reset, 128);

// Resets the system if two ^C characters are received in succession.
NORETURN static msg_t ThreadReset(void *arg) {
  (void) arg;

  chRegSetThreadName("reset");

  // Non-trivially reference the reset function pointer so it isn't elided by
  // the linker.
  std::swap(g_system_reset_function, g_system_reset_function);

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
  }

  NVIC_SystemReset();
  UNREACHABLE();
}

// Heartbeat thread working area.
static WORKING_AREA(wa_heartbeat, 128);

// Blinks an LED at 1 Hz at low priority to show system is functional and has
// available idle time.
NORETURN static msg_t ThreadHeartbeat(void *arg) {
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

void InitializeCorn() {
  // Clear error LED to show start.
  INVOKE(palClearPad, GPIO_LED_ERROR);

  // Setup debug serial driver; configuration static so valid when out of scope.
  static const SerialConfig debug_serial_config = { DEBUG_BAUDRATE,
                                                    0,
                                                    USART_CR2_STOP1_BITS,
                                                    0 };
  sdStart(&DEBUG_SERIAL, &debug_serial_config);

  // Start reset handler thread.
  chThdCreateStatic(wa_reset,
                    sizeof(wa_reset),
                    NORMALPRIO + 1,
                    ThreadReset,
                    nullptr);

  // Print welcome with OS mechanism, so there is output even if logging breaks.
  const uint8_t welcome_msg[] = ANSI_RESET "\r\n" BOARD_NAME "\r\n";
  chSequentialStreamWrite(&DEBUG_SERIAL, welcome_msg, sizeof(welcome_msg));

  // Print startup message.
  LogInfo("Firmware version %s built %s", g_build_version, g_build_time);

  // Start heartbeat thread.
  chThdCreateStatic(wa_heartbeat,
                    sizeof(wa_heartbeat),
                    LOWPRIO,
                    ThreadHeartbeat,
                    nullptr);

  // Start hall sensor rotor angle driver.
  static WORKING_AREA(wa_hall, 1024);
  static RotorHall rotor_hall(&HALL_ICU, &wa_hall, sizeof(wa_hall));

  // Start three-phase PWM driver.
  static InverterPWM inverter_pwm(&INVERTER_PWM);

  // Start gate driver and current sense amplifiers driver.
  static DRV8303 drv8303(&DRV_SPI);
  drv8303.ResetSoft();

  // Signal end of initialization.
  LogInfo("Initialized in %lu ms.", chTimeNow() * 1000 / CH_FREQUENCY);
  INVOKE(palClearPad, GPIO_LED_INIT);
}
