/*
 * Copyright (C) 2014 Xo Wang
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

#include "base/utility.h"

#include "ch.h"
#include "hal.h"

#include "base/logging.h"

NORETURN void _CriticalHalt(const char *func, const char *format, ...) {
  va_list args;
  va_start(args, format);
  vLogAtLevel(LOGGING_CRITICAL, func, format, args);
  va_end(args);
  // Wait for output queue to clear. The chOQIsEmptyI call should be under
  // system lock, but it's read only so this will work.
  const systime_t timeout_end = chTimeNow() + MS2ST(100);
  while (!chOQIsEmptyI(&DEBUG_SERIAL.oqueue) && chTimeNow() < timeout_end) {
    chThdYield();
  }
  // Set error LED after disabling interrupts so ISRs can't clear it.
  chSysLock();
  INVOKE(palSetPad, GPIO_LED_ERROR);
  chSysHalt();
  UNREACHABLE();
}