Corn3
=====
Copyright (C) 2014 Xo Wang

Corn3 is firmware for Corntroller version 3.

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
make TRGT=~/gcc-arm-none-eabi-4_7-2013q3/bin/arm-none-eabi-
```

Hardware
--------
Corntroller is a small and efficient brushless motor controller.

Version 3 features:

  - 1.0 in by 1.9 in
  - STM32F3 microcontroller (ARM Cortex-M4 @ 72MHz)
  - TI DRV8303 gate driver and current sense amplifier
  - Three-phase inverter with SO8 N-FETs
  - Dual shunt current sense
  - Hall sensor inputs
  - Up to 40 V input voltage

No power figures available, but should be similar to hobby controllers of
similar size.

License
-------
Corn3 is licensed under the MIT license with non-advertisement clause.

In compliance with the Apache license that ChibiOS template configuration files
are licensed under, the Apache License version 2.0 is included.

