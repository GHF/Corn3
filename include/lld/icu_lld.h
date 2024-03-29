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
/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio (MODIFIED BY ABOVE)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    STM32/icu_lld.h
 * @brief   STM32 ICU subsystem low level driver header.
 *
 * @note    This has been modified to support only TIM2 and TIM4 on STM32F30xxx.
 *
 * @note    This driver has been extended to support channel 1 as a XOR of
 *          channels 1, 2, and 3.
 *
 * @note    This driver has been extended for different reset triggers,
 *          including the channel 1 edge detector.
 *
 * @note    This driver has been modified to clear its timer when enabled, and
 *          to always invoke callbacks, even when exiting WAITING. So, the first
 *          edge will always invoke the callback with the time since enabling.
 *
 * @addtogroup ICU
 * @{
 */

#ifndef _ICU_LLD_H_
#define _ICU_LLD_H_

#include "stm32_tim.h"
#include "chcore.h"

#if HAL_USE_ICU || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   ICUD2 driver enable switch.
 * @details If set to @p TRUE the support for ICUD2 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_ICU_USE_TIM2) || defined(__DOXYGEN__)
#define STM32_ICU_USE_TIM2                  FALSE
#endif

/**
 * @brief   ICUD4 driver enable switch.
 * @details If set to @p TRUE the support for ICUD4 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_ICU_USE_TIM4) || defined(__DOXYGEN__)
#define STM32_ICU_USE_TIM4                  FALSE
#endif

/**
 * @brief   ICUD2 interrupt priority level setting.
 */
#if !defined(STM32_ICU_TIM2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_ICU_TIM2_IRQ_PRIORITY         7
#endif
/** @} */

/**
 * @brief   ICUD4 interrupt priority level setting.
 */
#if !defined(STM32_ICU_TIM4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_ICU_TIM4_IRQ_PRIORITY         7
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_ICU_USE_TIM1
#error "TIM1 not compatible with this driver"
#endif

#if STM32_ICU_USE_TIM2 && !STM32_HAS_TIM2
#error "TIM2 not present in the selected device"
#endif

#if STM32_ICU_USE_TIM3
#error "TIM3 not compatible with this driver"
#endif

#if STM32_ICU_USE_TIM4 && !STM32_HAS_TIM4
#error "TIM4 not present in the selected device"
#endif

#if STM32_ICU_USE_TIM5
#error "TIM5 not compatible with this driver"
#endif

#if STM32_ICU_USE_TIM8
#error "TIM8 not compatible with this driver"
#endif

#if STM32_ICU_USE_TIM9
#error "TIM9 not compatible with this driver"
#endif

#if !STM32_ICU_USE_TIM2 && !STM32_ICU_USE_TIM4
#error "ICU driver activated but no TIM peripheral assigned"
#endif

#if STM32_ICU_USE_TIM2 &&                                                   \
    !CORTEX_IS_VALID_KERNEL_PRIORITY(STM32_ICU_TIM2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM2"
#endif

#if STM32_ICU_USE_TIM4 &&                                                   \
    !CORTEX_IS_VALID_KERNEL_PRIORITY(STM32_ICU_TIM4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM4"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   ICU driver mode.
 */
typedef enum {
  ICU_INPUT_ACTIVE_HIGH = 0,        /**< Trigger on rising edge.            */
  ICU_INPUT_ACTIVE_LOW = 1,         /**< Trigger on falling edge.           */
} icumode_t;

/**
 * @brief   ICU frequency type.
 */
typedef uint32_t icufreq_t;

/**
 * @brief   ICU channel type.
 */
typedef enum {
  ICU_CHANNEL_1 = 0,              /**< Use TIMxCH1.      */
  ICU_CHANNEL_2 = 1,              /**< Use TIMxCH2.      */
  ICU_CHANNEL_3 = 2,              /**< Use TIMxCH3.      */
  ICU_CHANNEL_4 = 3,              /**< Use TIMxCH4.      */
} icuchannel_t;

/**
 * @brief   ICU counting configuration.
 */
typedef enum {
  ICU_RESET_NEVER       = 0,    /**< Timer starts enabled and never resets.  */
  ICU_RESET_ON_ACTIVE   = 1,    /**< Timer resets when driver enters ACTIVE. */
  ICU_RESET_ON_CH1_EDGE = 2,    /**< Timer resets when CH1 detects edge.     */
} icuresetmode_t;

/**
 * @brief   ICU channel 1 configuration.
 */
typedef enum {
  ICU_CHANNEL_1_INPUT_1 = 0,        /**< Channel 1 connected input 1.        */
  ICU_CHANNEL_1_XOR_123 = 1,        /**< Channel 1 is XORed inputs 1, 2, 3.  */
} icuxormode_t;

/**
 * @brief   ICU sample rate and filter configuration.
 */
typedef enum {
  ICU_FILTER_F_1_N_1 = 0,    /**< Full sample frequency, 1 sample filter. */
  ICU_FILTER_F_1_N_2 = 1,    /**< Full sample frequency, 2 sample filter. */
  ICU_FILTER_F_1_N_4 = 2,    /**< Full sample frequency, 2 sample filter. */
  ICU_FILTER_F_1_N_8 = 3,    /**< Full sample frequency, 8 sample filter. */
  ICU_FILTER_F_2_N_6 = 4,    /**< 1/2 sample frequency, 6 sample filter. */
  ICU_FILTER_F_2_N_8 = 5,    /**< 1/2 sample frequency, 8 sample filter. */
  ICU_FILTER_F_4_N_6 = 6,    /**< 1/4 sample frequency, 6 sample filter. */
  ICU_FILTER_F_4_N_8 = 7,    /**< 1/4 sample frequency, 8 sample filter. */
  ICU_FILTER_F_8_N_6 = 8,    /**< 1/8 sample frequency, 6 sample filter. */
  ICU_FILTER_F_8_N_8 = 9,    /**< 1/8 sample frequency, 8 sample filter. */
  ICU_FILTER_F_16_N_5 = 10,  /**< 1/16 sample frequency, 5 sample filter. */
  ICU_FILTER_F_16_N_6 = 11,  /**< 1/16 sample frequency, 6 sample filter. */
  ICU_FILTER_F_16_N_8 = 12,  /**< 1/16 sample frequency, 8 sample filter. */
  ICU_FILTER_F_32_N_5 = 13,  /**< 1/16 sample frequency, 5 sample filter. */
  ICU_FILTER_F_32_N_6 = 14,  /**< 1/16 sample frequency, 6 sample filter. */
  ICU_FILTER_F_32_N_8 = 15,  /**< 1/16 sample frequency, 8 sample filter. */
} icufilter_t;

/**
 * @brief   ICU counter type.
 */
typedef uint32_t icucnt_t;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Driver mode.
   */
  icumode_t                 mode;
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  icufreq_t                 frequency;
  /**
   * @brief   Callback for pulse width measurement.
   */
  icucallback_t             width_cb;
  /**
   * @brief   Callback for cycle period measurement.
   */
  icucallback_t             period_cb;
  /**
   * @brief   Callback for timer overflow.
   */
  icucallback_t             overflow_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief   Timer input channel to be used.
   * @note    Only inputs TIMx 1 and 2 are supported.
   */
  icuchannel_t              channel;
  /**
   * @brief TIM DIER register initialization data.
   * @note  The value of this field should normally be equal to zero.
   * @note  Only the DMA-related bits can be specified in this field.
   */
  uint32_t                  dier;
  /**
   * @brief   Reset mode of underlying timer.
   */
  icuresetmode_t            resetmode;
  /**
   * @brief   Configuration of channel 1.
   */
  icuxormode_t              xormode;
  /**
   * @brief   Hardware filter configuration.
   */
  icufilter_t               filter;
} ICUConfig;

/**
 * @brief   Structure representing an ICU driver.
 */
struct ICUDriver {
  /**
   * @brief Driver state.
   */
  icustate_t                state;
  /**
   * @brief Current configuration data.
   */
  const ICUConfig           *config;
#if defined(ICU_DRIVER_EXT_FIELDS)
  ICU_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Timer base clock.
   */
  uint32_t                  clock;
  /**
   * @brief Pointer to the TIMx registers block.
   */
  stm32_tim_t               *tim;
  /**
   * @brief CCR register used for width capture.
   */
  volatile uint32_t         *wccrp;
  /**
   * @brief CCR register used for period capture.
   */
  volatile uint32_t         *pccrp;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the width of the latest pulse.
 * @details The pulse width is defined as number of ticks between the start
 *          edge and the stop edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_width(icup) (*((icup)->wccrp) + 1)

/**
 * @brief   Returns the width of the latest cycle.
 * @details The cycle width is defined as number of ticks between a start
 *          edge and the next start edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_period(icup) (*((icup)->pccrp) + 1)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_ICU_USE_TIM2 && !defined(__DOXYGEN__)
extern ICUDriver ICUD2;
#endif

#if STM32_ICU_USE_TIM4 && !defined(__DOXYGEN__)
extern ICUDriver ICUD4;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void icu_lld_init(void);
  void icu_lld_start(ICUDriver *icup);
  void icu_lld_stop(ICUDriver *icup);
  void icu_lld_enable(ICUDriver *icup);
  void icu_lld_disable(ICUDriver *icup);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ICU */

#endif /* _ICU_LLD_H_ */

/** @} */
