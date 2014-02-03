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

#ifndef DRIVER_DRV8303_H_
#define DRIVER_DRV8303_H_

#include "hal.h"
#include <cstdint>

/**
 * @brief Communicates with the DRV8303 gate driver and current sense amplifier.
 *
 * @note Communications are done using the ChibiOS SPI module.
 */
class DRV8303 {
 public:
  /**
   * @brief Creates a driver control structure.
   *
   * @param spi_driver SPI driver used to communicate with the IC.
   */
  DRV8303(SPIDriver *spi_driver);

  /**
   * @brief Initializes driver functionality and clears faults.
   */
  void Start();

  /**
   * @brief Resets gate driver to normal operation and clears all faults in
   *        status registers (except for GVDD Over Voltage).
   *
   * @note This requires the driver be active before being called.
   */
  void ResetSoft();

  /**
   * @brief Checks for any faults latched in the driver and log them as errors.
   *
   * @note Reading faults clears any latched faults that are no longer present.
   *
   * @return True if any faults were found, or if communication failed.
   */
  bool CheckFaults();

  /**
   * @brief Enables gate driver and current sense amplifier.
   */
  void Activate();

  /**
   * @brief Disables gate driver and current sense amplifier.
   */
  void Deactivate();

 protected:
  /**
   * @brief Set of addressable registers in the gate driver IC.
   */
  enum Register {
    kRegisterStatus1, //!< kRegisterStatus1
    kRegisterStatus2, //!< kRegisterStatus2
    kRegisterControl1,//!< kRegisterControl1
    kRegisterControl2,//!< kRegisterControl2
    kNumRegisters     //!< kNumRegisters
  };

  // Bit offsets for register contents.
  static const int kRegisterDataOffset       = 0;
  static const int kRegisterAddressOffset    = 11;
  static const int kRegisterReadWriteOffset  = 15;
  static const int kRegisterFrameErrorOffset = 15;

  static const int kStatus1FaultOffset               = 10;  // Bit 10.
  static const int kStatus1GVddUnderVoltageOffset    = 9;   // Bit 9.
  static const int kStatus1PVddUnderVoltageOffset    = 8;   // Bit 8.
  static const int kStatus1OverTempShutDownOffset    = 7;   // Bit 7.
  static const int kStatus1OverTempWarningOffset     = 6;   // Bit 6.
  static const int kStatus1FetHighAOverCurrentOffset = 5;   // Bit 5.
  static const int kStatus1FetLowAOverCurrentOffset  = 4;   // Bit 4.
  static const int kStatus1FetHighBOverCurrentOffset = 3;   // Bit 3.
  static const int kStatus1FetLowBOverCurrentOffset  = 2;   // Bit 2.
  static const int kStatus1FetHighCOverCurrentOffset = 1;   // Bit 1.
  static const int kStatus1FetLowCOverCurrentOffset  = 0;   // Bit 0.

  static const int kStatus2GVddOverVoltageOffset     = 7;   // Bit 7.
  static const int kStatus2DeviceIdOffset            = 0;   // Bits [0..3].

  static const int kControl1OverCurrentAdjustOffset  = 6;   // Bits [6..10].
  static const int kControl1OverCurrentModeOffset    = 4;   // Bits [4..5].
  static const int kControl1PwmModeOffset            = 3;   // Bit 3.
  static const int kControl1GateResetOffset          = 2;   // Bit 2.
  static const int kControl1GateCurrentOffset        = 0;   // Bits [0..1].

  static const int kControl2OverCurrentTimeOffOffset = 6;   // Bit 6.
  static const int kControl2DcCalChannel2Offset      = 5;   // Bit 5.
  static const int kControl2DcCalChannel1Offset      = 4;   // Bit 4.
  static const int kControl2GainOffset               = 2;   // Bits [2..3].
  static const int kControl2OctwPinSettingOffset     = 0;   // Bits [0..1].

  /**
   * @brief Configuration options for OS driver.
   */
  static const SPIConfig kSpiConfig;

  /**
   * @brief Reads data from an driver internal register.
   *
   * @param register_address Register address to read from.
   * @param data Pointer to where read data will be stored.
   * @return True if read is successful and valid.
   */
  bool Read(Register register_address, uint16_t *data);

  /**
   * @brief Writes data to an driver internal register.
   *
   * @note Because write responds with Status register 1, this function will
   *       call @c CheckFaults with its value. It will also read Status register
   *       2 if any fault is latched, to check for GVDD Over Voltage.
   *
   * @param register_address Register address to write to.
   * @param data Data to write. Must be within 10 bits.
   * @return True if write is successful.
   */
  bool Write(Register register_address, uint16_t data);

  /**
   * @brief Same as the zero-parameter @c CheckFaults (which uses this
   *        internally), only this version takes the status register values it
   *        checks and logs rather than reading them itself.
   *
   * @note Used by @c Write, which gets the value of Status register 1 in
   *       response to its write operation.
   *
   * @param status1 Value of the Status register 1.
   * @param status2 Value of the Status register 2.
   * @return True if any faults were found.
   */
  bool CheckFaults(uint16_t status1, uint16_t status2);

  SPIDriver * const spi_driver_;
};

#endif  /* DRIVER_DRV8303_H_ */
