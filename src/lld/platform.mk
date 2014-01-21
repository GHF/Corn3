# Based on /ChibiOS/os/hal/platforms/STM32F30x/platform.mk with replacements for
# Corn-specific low-level drivers (LLD) in /src/driver and /include/driver

# HACK: The . makes this Corn3's lld dir appear earlier in the sorted VPATH.

# List of all the STM32F30x platform files.
PLATFORMSRC = ${CHIBIOS}/os/hal/platforms/STM32F30x/stm32_dma.c \
              ${CHIBIOS}/os/hal/platforms/STM32F30x/hal_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32F30x/adc_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32F30x/ext_lld_isr.c \
              ${CHIBIOS}/os/hal/platforms/STM32/can_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/ext_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/GPIOv2/pal_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/I2Cv2/i2c_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/RTCv2/rtc_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/SPIv2/spi_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/TIMv1/gpt_lld.c \
              ./src/lld/icu_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/TIMv1/pwm_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/USARTv2/serial_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/USARTv2/uart_lld.c \
              ${CHIBIOS}/os/hal/platforms/STM32/USBv1/usb_lld.c

# Required include directories
PLATFORMINC = ${CHIBIOS}/os/hal/platforms/STM32F30x \
              ${CHIBIOS}/os/hal/platforms/STM32 \
              ${CHIBIOS}/os/hal/platforms/STM32/GPIOv2 \
              ${CHIBIOS}/os/hal/platforms/STM32/I2Cv2 \
              ${CHIBIOS}/os/hal/platforms/STM32/RTCv2 \
              ${CHIBIOS}/os/hal/platforms/STM32/SPIv2 \
              include/lld \
              ${CHIBIOS}/os/hal/platforms/STM32/TIMv1 \
              ${CHIBIOS}/os/hal/platforms/STM32/USARTv2 \
              ${CHIBIOS}/os/hal/platforms/STM32/USBv1
