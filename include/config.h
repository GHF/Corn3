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

#ifndef CONFIG_H_
#define CONFIG_H_

/* Set a compile-time logging level. */
#define STATIC_LOGGING_LEVEL LOGGING_INFO

/* Option to remove floating point support from printf (saves code space). */
#define DISABLE_FLOAT_TO_STRING TRUE

/* Macros for GPIO pins that expand to their GPIO port and pad number. */
#define GPIO_LEDZ       GPIOB, GPIOB_LEDZ
#define GPIO_DRV_NSS    GPIOB, GPIOB_DRV_NSS
#define GPIO_LEDX       GPIOB, GPIOB_LEDX
#define GPIO_LEDY       GPIOB, GPIOB_LEDY

/* These GPIO group macros are of the form "port, mask, offset." */
#define GPIO_GROUP_HALL GPIOA,                           \
                        PAL_PORT_BIT(GPIOA_HALL_A) |     \
                            PAL_PORT_BIT(GPIOA_HALL_B) | \
                            PAL_PORT_BIT(GPIOA_HALL_C),  \
                        GPIOA_HALL_A

/* Options for LED functionality. */
#define GPIO_LED_ISR        GPIO_LEDX  /* Turned on while in important ISR.   */
#define GPIO_LED_ERROR      GPIO_LEDX  /* Latched on when error encountered.  */
#define GPIO_LED_HEARTBEAT  GPIO_LEDY  /* Blinks at 1 Hz for system healthy.  */
#define GPIO_LED_INIT       GPIO_LEDZ  /* Cleared upon system initialization. */
#define GPIO_LED_HALL       GPIO_LEDZ  /* Toggled upon hall state change.     */

/* Debug serial options. */
#define DEBUG_SERIAL    (SD3)
#define DEBUG_BAUDRATE  115200

/* Hall sensor input options. */
#define HALL_ICU              (ICUD2)
#define HALL_ICU_FREQ         (720000)
#define HALL_THREAD_PRIORITY  NORMALPRIO

#endif  /* CONFIG_H_ */
