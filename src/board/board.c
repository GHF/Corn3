/*
 * Corn3 - Copyright (C) 2014 Xo Wang
 * ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio (MODIFIED BY ABOVE)
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

#if HAL_USE_PAL || defined(__DOXYGEN__)
/**
 * @brief   PAL setup.
 * @details Digital I/O ports static configuration as defined in @p board.h.
 *          This variable is used by the HAL when initializing the PAL driver.
 */
const PALConfig pal_default_config = { { VAL_GPIOA_MODER,
                                         VAL_GPIOA_OTYPER,
                                         VAL_GPIOA_OSPEEDR,
                                         VAL_GPIOA_PUPDR,
                                         VAL_GPIOA_ODR,
                                         VAL_GPIOA_AFRL,
                                         VAL_GPIOA_AFRH },
                                       { VAL_GPIOB_MODER,
                                         VAL_GPIOB_OTYPER,
                                         VAL_GPIOB_OSPEEDR,
                                         VAL_GPIOB_PUPDR,
                                         VAL_GPIOB_ODR,
                                         VAL_GPIOB_AFRL,
                                         VAL_GPIOB_AFRH },
                                       { VAL_GPIOC_MODER,
                                         VAL_GPIOC_OTYPER,
                                         VAL_GPIOC_OSPEEDR,
                                         VAL_GPIOC_PUPDR,
                                         VAL_GPIOC_ODR,
                                         VAL_GPIOC_AFRL,
                                         VAL_GPIOC_AFRH },
                                       { VAL_GPIOD_MODER,
                                         VAL_GPIOD_OTYPER,
                                         VAL_GPIOD_OSPEEDR,
                                         VAL_GPIOD_PUPDR,
                                         VAL_GPIOD_ODR,
                                         VAL_GPIOD_AFRL,
                                         VAL_GPIOD_AFRH },
                                       { VAL_GPIOE_MODER,
                                         VAL_GPIOE_OTYPER,
                                         VAL_GPIOE_OSPEEDR,
                                         VAL_GPIOE_PUPDR,
                                         VAL_GPIOE_ODR,
                                         VAL_GPIOE_AFRL,
                                         VAL_GPIOE_AFRH },
                                       { VAL_GPIOF_MODER,
                                         VAL_GPIOF_OTYPER,
                                         VAL_GPIOF_OSPEEDR,
                                         VAL_GPIOF_PUPDR,
                                         VAL_GPIOF_ODR,
                                         VAL_GPIOF_AFRL,
                                         VAL_GPIOF_AFRH } };
#endif

/**
 * @brief   Early initialization code.
 * @details This initialization must be performed just after stack setup
 *          and before any other initialization.
 */
void __early_init(void) {
  stm32_clock_init();
}

#if HAL_USE_SDC || defined(__DOXYGEN__)
/**
 * @brief   SDC card detection.
 */
bool_t sdc_lld_is_card_inserted(SDCDriver *sdcp) {
  (void) sdcp;
  /* TODO: Fill the implementation.*/
  return TRUE;
}

/**
 * @brief   SDC card write protection detection.
 */
bool_t sdc_lld_is_write_protected(SDCDriver *sdcp) {
  (void) sdcp;
  /* TODO: Fill the implementation.*/
  return FALSE;
}
#endif /* HAL_USE_SDC */

#if HAL_USE_MMC_SPI || defined(__DOXYGEN__)
/**
 * @brief   MMC_SPI card detection.
 */
bool_t mmc_lld_is_card_inserted(MMCDriver *mmcp) {
  (void) mmcp;
  /* TODO: Fill the implementation.*/
  return TRUE;
}

/**
 * @brief   MMC_SPI card write protection detection.
 */
bool_t mmc_lld_is_write_protected(MMCDriver *mmcp) {
  (void) mmcp;
  /* TODO: Fill the implementation.*/
  return FALSE;
}
#endif

/**
 * @brief   Board-specific initialization code.
 * @todo    Add your board-specific code, if any.
 */
void boardInit(void) {
}
