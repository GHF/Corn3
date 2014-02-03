##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb3 -fomit-frame-pointer -falign-functions=16
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti -fno-exceptions -fno-threadsafe-statics -std=c++11
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

# Enables the use of FPU on Cortex-M4.
ifeq ($(USE_FPU),)
  USE_FPU = yes
endif

# Enable this if you really want to use the STM FWLib.
ifeq ($(USE_FWLIB),)
  USE_FWLIB = no
endif

# Enables the use of chprintf instead of system printf for base/logging.
ifeq ($(LOGGING_USE_CHPRINTF),)
  LOGGING_USE_CHPRINTF = yes
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = corn3

# Imported source files and paths
CHIBIOS = ChibiOS
include src/board/board.mk
include src/version/version_vars.mk
include src/lld/platform.mk
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/ports/GCC/ARMCMx/STM32F3xx/port.mk
include $(CHIBIOS)/os/kernel/kernel.mk

# Define linker script file here
LDSCRIPT= $(PORTLD)/STM32F303xC.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(PORTSRC) \
       $(KERNSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(BOARDSRC) \
       $(VERSIONSRC) \
       src/base/logging.c \
       src/base/utility.c \
       src/syscalls.c \

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = src/main.cpp \
		 src/corn.cpp \
		 src/cxx_stubs.cpp \
         src/driver/DRV8303.cpp \
         src/motor/rotor_hall.cpp \
         src/motor/inverter_pwm.cpp \

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC = $(PORTASM)

INCDIR = $(PORTINC) $(KERNINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) \
         $(CHIBIOS)/os/various

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m4

#TRGT = arm-elf-
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
LD   = $(TRGT)gcc
#LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra

#
# Compiler settings
##############################################################################

##############################################################################
# Start of default section
#

# List all default C defines here, like -D_DEBUG=1
DDEFS =

# List all default ASM defines here, like -D_DEBUG=1
DADEFS =

# List all default directories to look for include files here
DINCDIR =

# List the default directory to look for the libraries here
DLIBDIR =

# List all default libraries here
DLIBS =

#
# End of default section
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS =

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR = include

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS = $(VERSIONLIBS)

#
# End of user defines
##############################################################################

ifeq ($(USE_FPU),yes)
  USE_OPT += -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -fsingle-precision-constant
  DDEFS += -DCORTEX_USE_FPU=TRUE
else
  DDEFS += -DCORTEX_USE_FPU=FALSE
endif

ifeq ($(USE_FWLIB),yes)
  include $(CHIBIOS)/ext/stm32lib/stm32lib.mk
  CSRC += $(STM32SRC)
  INCDIR += $(STM32INC)
  USE_OPT += -DUSE_STDPERIPH_DRIVER
endif

ifeq ($(LOGGING_USE_CHPRINTF),yes)
  CSRC += $(CHIBIOS)/os/various/chprintf.c
  DDEFS += -DLOGGING_USE_CHPRINTF=1
else
  DDEFS += -DLOGGING_USE_CHPRINTF=0
endif

# Main make targets and rules.
include $(CHIBIOS)/os/ports/GCC/ARMCMx/rules.mk

# Additional make rules.
include $(VERSIONDIR)/version_rules.mk
