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

#ifndef DRIVER_USB_DEVICE_H_
#define DRIVER_USB_DEVICE_H_

#include <cstdint>

#include "ch.h"
#include "hal.h"

/**
 * @brief Configuration, initialization routines, and event handlers for USB
 *        serial driver.
 */
class UsbDevice {
 public:
  /**
   * @brief Initializes USB peripheral and prepares USB serial driver for data.
   */
  static void Start();

  /**
   * @brief Retrieves the USB serial channel. Pointer will not change during
   *        initialization.
   * @return USB serial channel, castable to OS IO channel.
   */
  static SerialUSBDriver *GetSerial();

 protected:
  static const SerialUSBConfig kSerialUsbConfig;
  static const USBConfig kUsbConfig;

  static const uint8_t kDeviceDescriptorData[];
  static const USBDescriptor kDeviceDescriptor;

  static const uint8_t kConfigurationDescriptorData[];
  static const USBDescriptor kConfigurationDescriptor;

  static const uint8_t kStringDescriptorDataLanguage[];
  static const uint8_t kStringDescriptorDataManufacturer[];
  static const uint8_t kStringDescriptorDataProduct[];
  static const size_t kSerialNumberDigits = 17;
  static const USBDescriptor kStringDescriptors[];

  static const USBEndpointConfig kEndpoint1Config;
  static const USBEndpointConfig kEndpoint2Config;

  static const usbep_t kDataRequestEndpoint;
  static const usbep_t kDataAvailableEndpoint;
  static const usbep_t kInterruptRequestEndpoint;

  static void ReadSerialNumber(size_t capacity, uint8_t *usb_string);

  static void HandleEvent(USBDriver *usb, usbevent_t event);

  static const USBDescriptor *GetDescriptor(USBDriver *usb,
                                            uint8_t descriptor_type,
                                            uint8_t descriptor_index,
                                            uint16_t language);

  static bool HandleRequests(USBDriver *usb_driver);

  // DP resistor control is not possible on Cornflake; using stubs for the
  // connection macros.
  static void usb_lld_connect_bus(USBDriver *) {
  }

  static void usb_lld_disconnect_bus(USBDriver *) {
  }

  static uint8_t string_descriptor_data_serial_number_[];

  static SerialUSBDriver sdud_;

  static USBInEndpointState endpoint1_in_state_;
  static USBOutEndpointState endpoint1_out_state_;
  static USBInEndpointState endpoint2_in_state_;
};

#endif  /* DRIVER_USB_DEVICE_H_ */
