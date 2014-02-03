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

#include "driver/DRV8303.h"

#include "ch.h"
#include "hal.h"
#include <strings.h>

#include "config.h"
#include "base/logging.h"
#include "base/utility.h"

DRV8303::DRV8303(SPIDriver *spi_driver)
    : spi_driver_(spi_driver) {
}

// Performs a hard reset of the driver and current sense amplifier.
void DRV8303::Start() {
  spiStart(spi_driver_, &kSpiConfig);
  do {
    Deactivate();
    Activate();
  } while (CheckFaults());
  // TODO: Calibrate analog inputs for amplifier offsets.
  LogInfo("Started gate driver and current sense amplifiers.");
}

// Sets the GATE_RESET bit in Status register 2 and waits for it to be cleared.
void DRV8303::ResetSoft() {
  const uint16_t control1_gate_reset_mask = 1 << kControl1GateResetOffset;
  while(!Write(kRegisterControl1, control1_gate_reset_mask))
    ;
  uint16_t control1;
  do {
    if (!Read(kRegisterControl1, &control1)) {
      LogWarning("No response from device.");
      chThdSleepMilliseconds(50);
      continue;
    }
  } while ((control1 & control1_gate_reset_mask) != 0);
}

// Looks for any set faults in Status register 1, and if so, read Status
// register 2 for its fault(s).
bool DRV8303::CheckFaults() {
  uint16_t status1;
  if (!Read(kRegisterStatus1, &status1)) {
    return true;
  }

  if (status1 == 0) {
    return false;
  }

  uint16_t status2 = 0;
  Read(kRegisterStatus2, &status2);
  return CheckFaults(status1, status2);
}

// Asserts the driver enable line and waits for the start up sequence time.
void DRV8303::Activate() {
  // TODO: Make GPIO parameters instance-specific.
  INVOKE(palSetPad, GPIO_DRV_EN);
  chThdSleepMilliseconds(10);
}

// Deasserts the driver enable line and waits for the shut down sequence time.
void DRV8303::Deactivate() {
  // TODO: Make GPIO parameters instance-specific.
  INVOKE(palClearPad, GPIO_DRV_EN);
  chThdSleepMicroseconds(10);
}

// Configures the SPI driver to communicate with the DRV8303.
//
// Note that the data sheet specifies that data is latched on clock falling
// edges, and the maximum clock frequency is 10 MHz. A lower frequency is chosen
// for better noise immunity.
const SPIConfig DRV8303::kSpiConfig = { nullptr,        // No callback.
                                        GPIO_DRV_NSS,   // Slave select pin.
                                        SPI_CR1_BR_1 |  // Scale /8 (4.5 MHz).
                                            SPI_CR1_CPHA,  // Falling edges.
                                        SPI_CR2_DS_3 | SPI_CR2_DS_2 |  // 16b.
                                        SPI_CR2_DS_1 | SPI_CR2_DS_0 };

// Reads an DRV8303 internal register over SPI.
//
// Note that the slave select line is deasserted between the read command and
// the driver's response.
bool DRV8303::Read(Register register_address, uint16_t *data) {
  if (register_address >= kNumRegisters) {
    LogError("Invalid register address (%d).", register_address);
    return false;
  }

  const uint16_t read_command = (1 << kRegisterReadWriteOffset) |
                                (register_address << kRegisterAddressOffset);

  LogDebug("DRV send read command %x.", read_command);
  spiSelect(spi_driver_);
  spiSend(spi_driver_, 1, &read_command);
  spiUnselect(spi_driver_);

  // The response is in a different 16-bit "frame" from the command.
  uint16_t receive_buffer;
  spiSelect(spi_driver_);
  spiReceive(spi_driver_, 1, &receive_buffer);
  spiUnselect(spi_driver_);
  LogDebug("DRV response %x.", receive_buffer);

  if (receive_buffer & (1 << kRegisterFrameErrorOffset)) {
    LogError("Frame error reading from %x.", register_address);
    return false;
  }

  const uint16_t response_address = (receive_buffer >> kRegisterAddressOffset) &
                                    0xf;
  if (response_address != register_address) {
    LogError("Address in response (%x) != address in read command (%x).",
             response_address, register_address);
    return false;
  }

  *data = receive_buffer & ((1 << kRegisterAddressOffset) - 1);
  return true;
}

// Writes to an DRV8303 internal register over SPI.
//
// This doesn't check whether a register is valid for write access, and probably
// won't return an error if you write to a read-only register.
//
// Also, this may take a long time if any faults are on the chip, as it will
// read Status register 2 and log all faults.
bool DRV8303::Write(Register register_address, uint16_t data) {
  if (register_address >= kNumRegisters) {
    LogError("Invalid register address (%x).", register_address);
    return false;
  }

  if (data >= (1 << kRegisterAddressOffset)) {
    LogWarning("TX data has more bits than register field (%x).", data);
  }

  const uint16_t write_command = (0 << kRegisterReadWriteOffset) |
                                 (register_address << kRegisterAddressOffset) |
                                 (data & ((1 << kRegisterAddressOffset) - 1) <<
                                     kRegisterDataOffset);

  LogDebug("DRV send write command %x.", write_command);
  spiSelect(spi_driver_);
  spiSend(spi_driver_, 1, &write_command);
  spiUnselect(spi_driver_);

  // Writes are responded to with the status1 register.
  uint16_t status1;
  spiSelect(spi_driver_);
  spiReceive(spi_driver_, 1, &status1);
  spiUnselect(spi_driver_);
  LogDebug("DRV response %x.", status1);

  if (status1 & (1 << kRegisterFrameErrorOffset)) {
    LogError("Frame error reading from %x.", register_address);
    return false;
  }

  if (status1 & (1 << kStatus1FaultOffset)) {
    uint16_t status2;
    Read(kRegisterStatus2, &status2);
    CheckFaults(status1, status2);
  }

  return true;
}

// Checks the bits in status registers for faults and logs them as errors.
bool DRV8303::CheckFaults(uint16_t status1, uint16_t status2) {
  if (status1 == 0) {
    return false;
  }
  do {
    const int set_bit_index = ::ffs(status1) - 1;

    switch (set_bit_index) {
      case kStatus1FaultOffset:
        LogError("Driver fault.");
        break;
      case kStatus1GVddUnderVoltageOffset:
        LogError("GVDD under voltage.");
        break;
      case kStatus1PVddUnderVoltageOffset:
        LogError("PVDD under voltage.");
        break;
      case kStatus1OverTempShutDownOffset:
        LogError("Over temperature shut down.");
        break;
      case kStatus1OverTempWarningOffset:
        LogError("Over temperature warning.");
        break;
      case kStatus1FetHighAOverCurrentOffset:
        LogError("Overcurrent in half-bridge A, high side.");
        break;
      case kStatus1FetLowAOverCurrentOffset:
        LogError("Overcurrent in half-bridge A, low side.");
        break;
      case kStatus1FetHighBOverCurrentOffset:
        LogError("Overcurrent in half-bridge B, high side.");
        break;
      case kStatus1FetLowBOverCurrentOffset:
        LogError("Overcurrent in half-bridge B, low side.");
        break;
      case kStatus1FetHighCOverCurrentOffset:
        LogError("Overcurrent in half-bridge C, high side.");
        break;
      case kStatus1FetLowCOverCurrentOffset:
        LogError("Overcurrent in half-bridge C, low side.");
        break;
      default:
        break;
    }

    // Clear this fault bit;
    status1 &= ~(1 << set_bit_index);
  } while (status1 != 0);

  if (status2 & (1 << kStatus2GVddOverVoltageOffset)) {
    LogError("GVDD overvoltage.");
  }

  return true;
}
