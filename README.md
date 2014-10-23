Corn3
=====
Copyright (C) 2014 Xo Wang

Corn3 is firmware for Corntroller version 3. It is a total rewrite of previous
Corntroller code, with a focus on writing code and documentation that may serve
as an introduction to three phase motor control for hobbyists.

Build
-----
Corn3 is written for use with GNU Tools for ARM Embedded Processors
(https://launchpad.net/gcc-arm-embedded).

To build, run make:

```
make
```

If you don't have arm-none-eabi in your PATH, specify its prefix in TRGT, e.g.:

```
make TRGT=~/gcc-arm-none-eabi-4_8-2014q3/bin/arm-none-eabi-
```

Hardware
--------
Corntroller is a small and efficient brushless motor controller.

Version 3.2 features:

  - 0.8 in by 1.6 in
  - STM32F3 microcontroller (ARM Cortex-M4 @ 72MHz)
  - TI DRV8301 gate driver, current sense amplifier, and buck converter
  - Three-phase inverter with SO8 N-FETs
  - Dual shunt current sense
  - Hall sensor inputs
  - Up to 30 V input voltage

No power figures available, but should be similar to hobby controllers of
similar size.

Hacking
-------
Corn3 and Corntroller are developed using the Black Magic Probe debugger.

See http://www.blacksphere.co.nz/main/blackmagic

Corntroller v3 has a 10-pin "mini" keyed JTAG header and a PicoBlade 4-pin
serial debug connector. The firmware does not yet support USB firmware
flashing.

License
-------
Corn3 is licensed under the MIT license with non-advertisement clause.

In compliance with the Apache license that ChibiOS configuration and low level
drivers are licensed under, the Apache License version 2.0 is included.

