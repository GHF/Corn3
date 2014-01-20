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

#include "ch.h"
#include "hal.h"

#include "config.h"
#include "base/logging.h"
#include "base/utility.h"
#include "version/version.h"

#include <cstdlib>

// heartbeat thread
static WORKING_AREA(waHeartbeat, 128);
NORETURN static msg_t threadHeartbeat(void *arg) {
  (void) arg;

  chRegSetThreadName("heartbeat");

  while (true) {
    INVOKE(palClearPad, GPIO_LEDY);
    chThdSleepMilliseconds(100);
    INVOKE(palSetPad, GPIO_LEDY);
    chThdSleepMilliseconds(900);
  }

  chThdExit(0);
}

int main(void) {
  halInit();
  chSysInit();

  // Setup debug serial port.
  const SerialConfig dbgSerialConfig = { DEBUG_BAUDRATE,
                                         0,
                                         USART_CR2_STOP1_BITS,
                                         USART_CR3_CTSE | USART_CR3_RTSE };
  sdStart(&DEBUG_SERIAL, &dbgSerialConfig);

  // Print using base OS mechanism, so there is output even if printf breaks.
  const uint8_t welcome_msg[] = BOARD_NAME "\r\n";
  chSequentialStreamWrite(&DEBUG_SERIAL, welcome_msg, sizeof(welcome_msg));

  // Print startup message.
  LogInfo("Firmware version %s built %s", g_build_version, g_build_time);

  chThdCreateStatic(waHeartbeat,
                    sizeof(waHeartbeat),
                    LOWPRIO,
                    threadHeartbeat,
                    nullptr);

  while (true) {
    chThdSleepMilliseconds(1000);
  }

  return EXIT_SUCCESS;
}
