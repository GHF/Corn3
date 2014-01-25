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
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio  (MODIFIED BY ABOVE)

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
/*
   Concepts and parts of this file have been contributed by Fabio Utzig and
   Xo Wang.
 */

/**
 * @file    STM32/icu_lld.c
 * @brief   STM32 ICU subsystem low level driver header.
 *
 * @addtogroup ICU
 * @{
 */

#include "ch.h"
#include "hal.h"

#if HAL_USE_ICU || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   ICUD2 driver identifier.
 * @note    The driver ICUD1 allocates the timer TIM2 when enabled.
 */
#if STM32_ICU_USE_TIM2 || defined(__DOXYGEN__)
ICUDriver ICUD2;
#endif

/**
 * @brief   ICUD4 driver identifier.
 * @note    The driver ICUD4 allocates the timer TIM4 when enabled.
 */
#if STM32_ICU_USE_TIM4 || defined(__DOXYGEN__)
ICUDriver ICUD4;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Shared IRQ handler.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 */
static void icu_lld_serve_interrupt(ICUDriver *icup) {
  uint16_t sr;

  sr  = icup->tim->SR;
  sr &= icup->tim->DIER & STM32_TIM_DIER_IRQ_MASK;
  icup->tim->SR = ~sr;
  switch (icup->config->channel) {
  case ICU_CHANNEL_1:
    if ((sr & STM32_TIM_SR_CC1IF) != 0)
      _icu_isr_invoke_period_cb(icup);
    if ((sr & STM32_TIM_SR_CC2IF) != 0)
      _icu_isr_invoke_width_cb(icup);
    break;
  case ICU_CHANNEL_2:
    if ((sr & STM32_TIM_SR_CC1IF) != 0)
      _icu_isr_invoke_width_cb(icup);
    if ((sr & STM32_TIM_SR_CC2IF) != 0)
      _icu_isr_invoke_period_cb(icup);
    break;
  case ICU_CHANNEL_3:
  case ICU_CHANNEL_4:
    if ((sr & STM32_TIM_SR_CC3IF) != 0)
      _icu_isr_invoke_period_cb(icup);
    if ((sr & STM32_TIM_SR_CC4IF) != 0)
      _icu_isr_invoke_width_cb(icup);
    break;
  }
  if ((sr & STM32_TIM_SR_UIF) != 0)
    _icu_isr_invoke_overflow_cb(icup);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if STM32_ICU_USE_TIM2
#if !defined(STM32_TIM2_HANDLER)
#error "STM32_TIM2_HANDLER not defined"
#endif
/**
 * @brief   TIM2 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM2_HANDLER) {

  CH_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD2);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_ICU_USE_TIM2 */

#if STM32_ICU_USE_TIM4
#if !defined(STM32_TIM4_HANDLER)
#error "STM32_TIM4_HANDLER not defined"
#endif
/**
 * @brief   TIM4 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM4_HANDLER) {

  CH_IRQ_PROLOGUE();

  icu_lld_serve_interrupt(&ICUD4);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_ICU_USE_TIM4 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level ICU driver initialization.
 *
 * @notapi
 */
void icu_lld_init(void) {

#if STM32_ICU_USE_TIM2
  /* Driver initialization.*/
  icuObjectInit(&ICUD2);
  ICUD2.tim = STM32_TIM2;
#endif

#if STM32_ICU_USE_TIM4
  /* Driver initialization.*/
  icuObjectInit(&ICUD4);
  ICUD4.tim = STM32_TIM4;
#endif
}

/**
 * @brief   Configures and activates the ICU peripheral.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_start(ICUDriver *icup) {
  uint32_t psc;

  chDbgAssert((icup->config->channel == ICU_CHANNEL_1) ||
              (icup->config->channel == ICU_CHANNEL_2) ||
              (icup->config->channel == ICU_CHANNEL_3) ||
              (icup->config->channel == ICU_CHANNEL_4),
              "icu_lld_start(), #1", "invalid input");

  chDbgAssert((icup->config->resetmode != ICU_RESET_ON_CH1_EDGE) ||
              (icup->config->channel == ICU_CHANNEL_1),
              "icu_lld_start(), #2", "reset on CH1 but channel 1 not selected");

  chDbgAssert((icup->config->xormode != ICU_CHANNEL_1_XOR_123) ||
              (icup->config->channel == ICU_CHANNEL_1),
              "icu_lld_start(), #3",
              "channel 1 XOR enabled but channel 1 not selected");

  chDbgAssert((icup->config->resetmode == ICU_RESET_NEVER) ||
              ((icup->config->channel != ICU_CHANNEL_3) &&
               (icup->config->channel != ICU_CHANNEL_4)),
              "icu_lld_start(), #4", "channels 3 and 4 don't support reset");

  if (icup->state == ICU_STOP) {
    /* Clock activation and timer reset.*/
#if STM32_ICU_USE_TIM2
    if (&ICUD2 == icup) {
      rccEnableTIM2(FALSE);
      rccResetTIM2();
      nvicEnableVector(STM32_TIM2_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_ICU_TIM2_IRQ_PRIORITY));
      icup->clock = STM32_TIMCLK1;
    }
#endif
#if STM32_ICU_USE_TIM4
    if (&ICUD4 == icup) {
      rccEnableTIM4(FALSE);
      rccResetTIM4();
      nvicEnableVector(STM32_TIM4_NUMBER,
                       CORTEX_PRIORITY_MASK(STM32_ICU_TIM4_IRQ_PRIORITY));
      icup->clock = STM32_TIMCLK1;
    }
#endif
  }
  else {
    /* Driver re-configuration scenario, it must be stopped first.*/
    icup->tim->CR1    = 0;                  /* Timer disabled.              */
    icup->tim->CR2    = 0;                  /* Clear timer options.         */
    icup->tim->DIER   = icup->config->dier &/* DMA-related DIER settings.   */
                        ~STM32_TIM_DIER_IRQ_MASK;
    icup->tim->SR     = 0;                  /* Clear eventual pending IRQs. */
    icup->tim->CCR[0] = 0;                  /* Comparator 1 disabled.       */
    icup->tim->CCR[1] = 0;                  /* Comparator 2 disabled.       */
    icup->tim->CNT    = 0;                  /* Counter reset to zero.       */
  }

  /* Timer configuration.*/
  psc = (icup->clock / icup->config->frequency) - 1;
  chDbgAssert((psc <= 0xFFFF) &&
              ((psc + 1) * icup->config->frequency) == icup->clock,
              "icu_lld_start(), #5", "invalid frequency");
  icup->tim->PSC  = (uint16_t)psc;
  icup->tim->ARR   = 0xFFFFFFFF;

  switch (icup->config->channel) {
  case ICU_CHANNEL_1:
    /* Selected input 1.
       CCMR1_CC1S = 01 = CH1 Input on TI1.
       CCMR1_CC2S = 10 = CH2 Input on TI1.*/
    icup->tim->CCMR1 = STM32_TIM_CCMR1_CC1S(1) | STM32_TIM_CCMR1_CC2S(2);

    if (icup->config->xormode == ICU_CHANNEL_1_XOR_123) {
      /* TI1 is CH1, CH1, and CH3 XORed together. */
      icup->tim->CR2 |= STM32_TIM_CR2_TI1S;
    }

    if (icup->config->resetmode == ICU_RESET_ON_ACTIVE) {
      /* SMCR_TS  = 101, input is TI1FP1.
         SMCR_SMS = 100, reset on rising edge.*/
      icup->tim->SMCR  = STM32_TIM_SMCR_TS(5) | STM32_TIM_SMCR_SMS(4);
    } else if (icup->config->resetmode == ICU_RESET_ON_CH1_EDGE) {
      /* SMCR_TS  = 100, input is TI1 edge detector.
         SMCR_SMS = 100, reset on both edges.*/
      icup->tim->SMCR  = STM32_TIM_SMCR_TS(4) | STM32_TIM_SMCR_SMS(4);
    }

    /* The CCER settings depend on the selected trigger mode.
       ICU_INPUT_ACTIVE_HIGH: Active on rising edge, idle on falling edge.
       ICU_INPUT_ACTIVE_LOW:  Active on falling edge, idle on rising edge.*/
    if (icup->config->mode == ICU_INPUT_ACTIVE_HIGH)
      icup->tim->CCER = STM32_TIM_CCER_CC1E |
                        STM32_TIM_CCER_CC2E | STM32_TIM_CCER_CC2P;
    else
      icup->tim->CCER = STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC1P |
                        STM32_TIM_CCER_CC2E;

    /* Direct pointers to the capture registers in order to make reading
       data faster from within callbacks.*/
    icup->wccrp = &icup->tim->CCR[1];
    icup->pccrp = &icup->tim->CCR[0];
    break;

  case ICU_CHANNEL_2:
    /* Selected input 2.
       CCMR1_CC1S = 10 = CH1 Input on TI2.
       CCMR1_CC2S = 01 = CH2 Input on TI2.*/
    icup->tim->CCMR1 = STM32_TIM_CCMR1_CC1S(2) | STM32_TIM_CCMR1_CC2S(1);

    if (icup->config->resetmode == ICU_RESET_ON_ACTIVE) {
      /* SMCR_TS  = 110, input is TI2FP2.
         SMCR_SMS = 100, reset on rising edge.*/
      icup->tim->SMCR  = STM32_TIM_SMCR_TS(6) | STM32_TIM_SMCR_SMS(4);
    }

    /* The CCER settings depend on the selected trigger mode.
       ICU_INPUT_ACTIVE_HIGH: Active on rising edge, idle on falling edge.
       ICU_INPUT_ACTIVE_LOW:  Active on falling edge, idle on rising edge.*/
    if (icup->config->mode == ICU_INPUT_ACTIVE_HIGH)
      icup->tim->CCER = STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC1P |
                        STM32_TIM_CCER_CC2E;
    else
      icup->tim->CCER = STM32_TIM_CCER_CC1E |
                        STM32_TIM_CCER_CC2E | STM32_TIM_CCER_CC2P;

    /* Direct pointers to the capture registers in order to make reading
       data faster from within callbacks.*/
    icup->wccrp = &icup->tim->CCR[0];
    icup->pccrp = &icup->tim->CCR[1];
    break;

  case ICU_CHANNEL_3:
  case ICU_CHANNEL_4:
    /* Inputs 3 and 4 can't be used as reset triggers, so they can have the
       same capture channels configuration (ACTIVE on CCR3 and IDLE on CCR4). */
    if (icup->config->channel == ICU_CHANNEL_3) {
      /* Selected input 3.
         CCMR2_CC3S = 01 = CH1 Input on TI3.
         CCMR2_CC4S = 10 = CH2 Input on TI3.*/
      icup->tim->CCMR2 = STM32_TIM_CCMR2_CC3S(1) | STM32_TIM_CCMR2_CC4S(2);
    } else {
      /* Selected input 4.
         CCMR2_CC3S = 10 = CH1 Input on TI4.
         CCMR2_CC4S = 01 = CH2 Input on TI4.*/
      icup->tim->CCMR2 = STM32_TIM_CCMR2_CC3S(2) | STM32_TIM_CCMR2_CC4S(1);
    }

    /* The CCER settings depend on the selected trigger mode.
       ICU_INPUT_ACTIVE_HIGH: Active on rising edge, idle on falling edge.
       ICU_INPUT_ACTIVE_LOW:  Active on falling edge, idle on rising edge.*/
    if (icup->config->mode == ICU_INPUT_ACTIVE_HIGH)
      icup->tim->CCER = STM32_TIM_CCER_CC3E |
                        STM32_TIM_CCER_CC4E | STM32_TIM_CCER_CC4P;
    else
      icup->tim->CCER = STM32_TIM_CCER_CC3E | STM32_TIM_CCER_CC3P |
                        STM32_TIM_CCER_CC4E;
    /* There's no trigger to update registers, so this does it manually. */
    icup->tim->EGR |= STM32_TIM_EGR_UG;

    /* Direct pointers to the capture registers in order to make reading
       data faster from within callbacks.*/
    icup->wccrp = &icup->tim->CCR[4];
    icup->pccrp = &icup->tim->CCR[3];
    break;
  }
}

/**
 * @brief   Deactivates the ICU peripheral.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_stop(ICUDriver *icup) {

  if (icup->state == ICU_READY) {
    /* Clock deactivation.*/
    icup->tim->CR1  = 0;                    /* Timer disabled.              */
    icup->tim->DIER = 0;                    /* All IRQs disabled.           */
    icup->tim->SR   = 0;                    /* Clear eventual pending IRQs. */

#if STM32_ICU_USE_TIM2
    if (&ICUD2 == icup) {
      nvicDisableVector(STM32_TIM2_NUMBER);
      rccDisableTIM2(FALSE);
    }
#endif
#if STM32_ICU_USE_TIM4
    if (&ICUD4 == icup) {
      nvicDisableVector(STM32_TIM4_NUMBER);
      rccDisableTIM4(FALSE);
    }
#endif
  }
}

/**
 * @brief   Enables the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_enable(ICUDriver *icup) {

  icup->tim->SR = 0;                        /* Clear pending IRQs (if any). */
  switch (icup->config->channel) {
  case ICU_CHANNEL_1:
    if (icup->config->period_cb != NULL)
      icup->tim->DIER |= STM32_TIM_DIER_CC1IE;
    if (icup->config->width_cb != NULL)
      icup->tim->DIER |= STM32_TIM_DIER_CC2IE;
    break;

  case ICU_CHANNEL_2:
    if (icup->config->width_cb != NULL)
      icup->tim->DIER |= STM32_TIM_DIER_CC1IE;
    if (icup->config->period_cb != NULL)
      icup->tim->DIER |= STM32_TIM_DIER_CC2IE;
    break;

  /* Both channels 3 and 4 have the same capture polarity setup. */
  case ICU_CHANNEL_3:
  case ICU_CHANNEL_4:
    if (icup->config->width_cb != NULL)
      icup->tim->DIER |= STM32_TIM_DIER_CC3IE;
    if (icup->config->period_cb != NULL)
      icup->tim->DIER |= STM32_TIM_DIER_CC4IE;
    break;
  }
  if (icup->config->overflow_cb != NULL)
    icup->tim->DIER |= STM32_TIM_DIER_UIE;
  icup->tim->CR1 = STM32_TIM_CR1_URS | STM32_TIM_CR1_CEN;
}

/**
 * @brief   Disables the input capture.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 *
 * @notapi
 */
void icu_lld_disable(ICUDriver *icup) {

  icup->tim->CR1   = 0;                     /* Initially stopped.           */
  icup->tim->SR    = 0;                     /* Clear pending IRQs (if any). */

  /* All interrupts disabled.*/
  icup->tim->DIER &= ~STM32_TIM_DIER_IRQ_MASK;
}

#endif /* HAL_USE_ICU */

/** @} */
