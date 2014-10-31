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

#include "driver/usb_device.h"

#include "base/utility.h"
#include "config.h"

void UsbDevice::Start() {
  ReadSerialNumber(kSerialNumberDigits * 2,
                   &string_descriptor_data_serial_number_[2]);
  sduObjectInit(&sdud_);
  sduStart(&sdud_, &kSerialUsbConfig);

  USBDriver * const usb_driver = kSerialUsbConfig.usbp;
  usbDisconnectBus(usb_driver);
  chThdSleepMilliseconds(500);
  usbStart(usb_driver, &kUsbConfig);
  usbConnectBus(usb_driver);
}

SerialUSBDriver *UsbDevice::GetSerial() {
  return &sdud_;
}

const SerialUSBConfig UsbDevice::kSerialUsbConfig = {
  &USB_DRIVER,               // usbp
  kDataRequestEndpoint,      // bulk_in
  kDataAvailableEndpoint,    // bulk_out
  kInterruptRequestEndpoint  // int_in
};

const USBConfig UsbDevice::kUsbConfig = {
  HandleEvent,     // event_cb
  GetDescriptor,   // get_descriptor_cb
  HandleRequests,  // requests_hook_cb
  nullptr          // sof_cb
};

const uint8_t UsbDevice::kDeviceDescriptorData[18] = {
    USB_DESC_DEVICE(0x0200,  // bcdUSB (2.0).
                    0x02,    // bDeviceClass (CDC).
                    0x00,    // bDeviceSubClass.
                    0x00,    // bDeviceProtocol.
                    0x40,    // bMaxPacketSize.
                    0x6666,  // idVendor (prototype).
                    0xc000,  // idProduct.
                    (CORN_MAJOR << 8) + (CORN_MINOR),  // bcdDevice.
                    1,       // iManufacturer.
                    2,       // iProduct.
                    3,       // iSerialNumber.
                    1)       // bNumConfigurations.
};

const USBDescriptor UsbDevice::kDeviceDescriptor = {
    sizeof(kDeviceDescriptorData), kDeviceDescriptorData
};

const uint8_t UsbDevice::kConfigurationDescriptorData[67] = {
    // Configuration Descriptor.
    USB_DESC_CONFIGURATION(67,    // wTotalLength.
                           0x02,  // bNumInterfaces.
                           0x01,  // bConfigurationValue.
                           0,     // iConfiguration.
                           0xC0,  // bmAttributes (self powered).
                           250),  // bMaxPower (500mA).
    // Interface Descriptor.
    USB_DESC_INTERFACE(0x00,  // bInterfaceNumber.
                       0x00,  // bAlternateSetting.
                       0x01,  // bNumEndpoints.
                       0x02,  // bInterfaceClass (Communications Interface
                              // Class, CDC section 4.2).
                       0x02,  // bInterfaceSubClass (Abstract Control Model, CDC
                              // section 4.3).
                       0x01,  // bInterfaceProtocol (AT commands, CDC section
                              // 4.4).
                       0),    // iInterface.
    // Header Functional Descriptor (CDC section 5.2.3).
    USB_DESC_BYTE(5),      // bLength.
    USB_DESC_BYTE(0x24),   // bDescriptorType (CS_INTERFACE).
    USB_DESC_BYTE(0x00),   // bDescriptorSubtype (Header Functional Descriptor).
    USB_DESC_BCD(0x0110),  // bcdCDC.
    // Call Management Functional Descriptor.
    USB_DESC_BYTE(5),     // bFunctionLength.
    USB_DESC_BYTE(0x24),  // bDescriptorType (CS_INTERFACE).
    USB_DESC_BYTE(0x01),  // bDescriptorSubtype (Call Management Functional
                          // Descriptor).
    USB_DESC_BYTE(0x00),  // bmCapabilities (D0+D1).
    USB_DESC_BYTE(0x01),  // bDataInterface.
    // ACM Functional Descriptor.
    USB_DESC_BYTE(4),     // bFunctionLength.
    USB_DESC_BYTE(0x24),  // bDescriptorType (CS_INTERFACE).
    USB_DESC_BYTE(0x02),  // bDescriptorSubtype (Abstract Control Management
                          // Descriptor).
    USB_DESC_BYTE(0x02),  // bmCapabilities.
    // Union Functional Descriptor.
    USB_DESC_BYTE(5),     // bFunctionLength.
    USB_DESC_BYTE(0x24),  // bDescriptorType (CS_INTERFACE).
    USB_DESC_BYTE(0x06),  // bDescriptorSubtype (Union Functional Descriptor).
    USB_DESC_BYTE(0x00),  // bMasterInterface (Communication Class Interface).
    USB_DESC_BYTE(0x01),  // bSlaveInterface0 (Data Class Interface).
    // Endpoint 2 Descriptor.
    USB_DESC_ENDPOINT(kInterruptRequestEndpoint | 0x80,  // bEndpointAddress.

                      0x03,    // bmAttributes (Interrupt).
                      0x0008,  // wMaxPacketSize.
                      0xFF),   // bInterval.
    // Interface Descriptor.
    USB_DESC_INTERFACE(0x01,   // bInterfaceNumber.
                       0x00,   // bAlternateSetting.
                       0x02,   // bNumEndpoints.
                       0x0A,   // bInterfaceClass (Data Class Interface, CDC
                               // section 4.5).
                       0x00,   // bInterfaceSubClass (CDC section 4.6).
                       0x00,   // bInterfaceProtocol (CDC section 4.7).
                       0x00),  // iInterface.
    // Endpoint 3 Descriptor.
    USB_DESC_ENDPOINT(kDataAvailableEndpoint,  // bEndpointAddress.
                      0x02,    // bmAttributes (Bulk).
                      0x0040,  // wMaxPacketSize.
                      0x00),   // bInterval.
    // Endpoint 1 Descriptor.
    USB_DESC_ENDPOINT(kDataRequestEndpoint | 0x80,  // bEndpointAddress.
                      0x02,    // bmAttributes (Bulk).
                      0x0040,  // wMaxPacketSize.
                      0x00)    // bInterval.
};

const USBDescriptor UsbDevice::kConfigurationDescriptor = {
    sizeof(kConfigurationDescriptorData), kConfigurationDescriptorData
};

const uint8_t UsbDevice::kStringDescriptorDataLanguage[] = {
    USB_DESC_BYTE(4),                      // bLength.
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING),  // bDescriptorType.
    USB_DESC_WORD(0x0409)                  // wLANGID (U.S. English).
};

const uint8_t UsbDevice::kStringDescriptorDataManufacturer[] = {
    USB_DESC_BYTE(20),                     // bLength
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING),  // bDescriptorType
    'h', 0, 'a', 0, 'c', 0, 'k', 0, 's', 0, 'h', 0, 'a', 0, 'c', 0, 'k', 0
};

const uint8_t UsbDevice::kStringDescriptorDataProduct[] = {
    USB_DESC_BYTE(34),                     // bLength.
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING),  // bDescriptorType.
    'C', 0, 'o', 0, 'r', 0, 'n', 0, 't', 0, 'r', 0, 'o', 0, 'l', 0, 'l', 0,
    'e', 0, 'r', 0, ' ', 0, 'v', 0, '0' + CORN_MAJOR, 0, '.', 0,
    '0' + CORN_MINOR, 0
};

// This is defined earlier than per style to make it a complete type for the
// string descriptors array.
uint8_t UsbDevice::string_descriptor_data_serial_number_
    [2 + kSerialNumberDigits * 2] = {
    USB_DESC_BYTE(2 + kSerialNumberDigits * 2),  // bLength.
    USB_DESC_BYTE(USB_DESCRIPTOR_STRING),        // bDescriptorType.
};

const USBDescriptor UsbDevice::kStringDescriptors[] = {
    {sizeof(kStringDescriptorDataLanguage),
     kStringDescriptorDataLanguage},
    {sizeof(kStringDescriptorDataManufacturer),
     kStringDescriptorDataManufacturer},
    {sizeof(kStringDescriptorDataProduct),
     kStringDescriptorDataProduct},
    {sizeof(string_descriptor_data_serial_number_),
     string_descriptor_data_serial_number_},
};

const USBEndpointConfig UsbDevice::kEndpoint1Config = {
    USB_EP_MODE_TYPE_BULK,  // ep_mode
    nullptr,                // setup_cb
    sduDataTransmitted,     // in_cb
    sduDataReceived,        // out_cb
    64,                     // in_maxsize
    64,                     // out_maxsize
    &endpoint1_in_state_,   // in_state
    &endpoint1_out_state_,  // out_state
    1,                      // ep_buffers
    nullptr                 // setup_buf
};

const USBEndpointConfig UsbDevice::kEndpoint2Config = {
    USB_EP_MODE_TYPE_INTR,    // ep_mode
    nullptr,                  // setup_cb
    sduInterruptTransmitted,  // in_cb
    nullptr,                  // out_cb
    16,                       // in_maxsize
    0,                        // out_maxsize
    &endpoint2_in_state_,     // in_state
    nullptr,                  // out_state
    1,                        // ep_buffers
    nullptr                   // setup_buf
};

const usbep_t UsbDevice::kDataRequestEndpoint = 1;
const usbep_t UsbDevice::kDataAvailableEndpoint = 1;
const usbep_t UsbDevice::kInterruptRequestEndpoint = 2;

void UsbDevice::ReadSerialNumber(size_t capacity, uint8_t *usb_string) {
  // TODO(Xo): Read the STM32 unique device ID for this.
  for (size_t i = 0; i < capacity; i += 2) {
    usb_string[i] = '0';
    usb_string[i + 1] = 0;
  }
}

void UsbDevice::HandleEvent(USBDriver *usb, usbevent_t event) {
  switch (event) {
    case USB_EVENT_RESET:
      return;
    case USB_EVENT_ADDRESS:
      return;
    case USB_EVENT_CONFIGURED:
      chSysLockFromIsr();

      // Enables the endpoints specified into the configuration.
      // Note: this callback is invoked from an ISR so I-Class functions must be
      // used.
      usbInitEndpointI(usb, kDataRequestEndpoint, &kEndpoint1Config);
      usbInitEndpointI(usb, kInterruptRequestEndpoint, &kEndpoint2Config);

      // Reset the state of the CDC subsystem.
      sduConfigureHookI(&sdud_);

      chSysUnlockFromIsr();
      return;
    case USB_EVENT_SUSPEND:
      return;
    case USB_EVENT_WAKEUP:
      return;
    case USB_EVENT_STALLED:
      return;
    default:
      break;
  }
}

const USBDescriptor *UsbDevice::GetDescriptor(USBDriver *usb,
                                              uint8_t descriptor_type,
                                              uint8_t descriptor_index,
                                              uint16_t language) {
  (void) usb;
  (void) language;
  switch (descriptor_type) {
    case USB_DESCRIPTOR_DEVICE:
      return &kDeviceDescriptor;
    case USB_DESCRIPTOR_CONFIGURATION:
      return &kConfigurationDescriptor;
    case USB_DESCRIPTOR_STRING:
      if (descriptor_index < ARRAY_SIZE(kStringDescriptors))
        return &kStringDescriptors[descriptor_index];
      break;
  }
  return nullptr;
}

bool UsbDevice::HandleRequests(USBDriver *usb_driver) {
  return sduRequestsHook(usb_driver);
}

SerialUSBDriver UsbDevice::sdud_;

USBInEndpointState UsbDevice::endpoint1_in_state_;
USBOutEndpointState UsbDevice::endpoint1_out_state_;
USBInEndpointState UsbDevice::endpoint2_in_state_;
