/*
 * Copyright (c) 2018-2019, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stdint.h"

#include "USBCustomDevice.h"
#include "usb_phy_api.h"

#define REPORT_KEY_PRESS 1
#define REPORT_LED_STATUS 2

using namespace arduino;


USBCustomDevice::USBCustomDevice(bool connect, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
    USBHID(get_usb_phy(), 0, 0, vendor_id, product_id, product_release)
{
}

USBCustomDevice::USBCustomDevice(USBPhy *phy, uint16_t vendor_id, uint16_t product_id, uint16_t product_release):
    USBHID(phy, 0, 0, vendor_id, product_id, product_release)
{
    // User or child responsible for calling connect or init
}

USBCustomDevice::~USBCustomDevice()
{
}

const uint8_t *USBCustomDevice::report_desc()
{
    static const uint8_t reportDescriptor[] = {
        USAGE_PAGE(2), 0x00, 0xFF,                  // Vendor Defined Page 1
        USAGE(1), 0x01,                             // Vendor Usage 1
        COLLECTION(1), 0x01,                        // Application
            LOGICAL_MINIMUM(1), 0x00,               // 0 to
            LOGICAL_MAXIMUM(2), 0xFF, 0x00,         // 255
            REPORT_SIZE(1), 0x08,                   // Report 8 bits
            REPORT_COUNT(1), 32,                    // Count 32 Bytes
            USAGE(1), 0x01,                         // Vendor Usage 1
            INPUT(2), 0x02, 0x01,                   // Data, Variable, Absolute, Buffer
            USAGE(1), 0x01,                         // Vendor Usage 1
            OUTPUT(2), 0x02, 0x01,                  // Data, Variable, Absolute, Buffer
        END_COLLECTION(0),
    };
    reportLength = sizeof(reportDescriptor);
    return reportDescriptor;
}


void USBCustomDevice::report_rx()
{
    assert_locked();

    HID_REPORT report;
    read_nb(&report);

    if (report.data[0] == REPORT_LED_STATUS)
    {
        _led_status_updated = true;
        _led_status = report.data[1] & 0b111;
    }
}

uint8_t USBCustomDevice::led_status()
{
    return _led_status;
}

bool USBCustomDevice::led_status_changed()
{
    bool status = _led_status_updated;
    _led_status_updated = false;

    return status;
}

bool USBCustomDevice::key_press(int key)
{
    _mutex.lock();

    HID_REPORT report;

    memset(&report, 0, sizeof(HID_REPORT));

    report.length = 32;
    report.data[0] = REPORT_KEY_PRESS;
    report.data[1] = key;

    if (!send_nb(&report)) {
        _mutex.unlock();
        return false;
    }

    _mutex.unlock();
    return true;

}

#define DEFAULT_CONFIGURATION (1)
#define TOTAL_DESCRIPTOR_LENGTH ((1 * CONFIGURATION_DESCRIPTOR_LENGTH) \
                               + (1 * INTERFACE_DESCRIPTOR_LENGTH) \
                               + (1 * HID_DESCRIPTOR_LENGTH) \
                               + (2 * ENDPOINT_DESCRIPTOR_LENGTH))

const uint8_t *USBCustomDevice::configuration_desc(uint8_t index)
{
    if (index != 0) {
        return NULL;
    }
    uint8_t configuration_descriptor_temp[] = {
        CONFIGURATION_DESCRIPTOR_LENGTH,    // bLength
        CONFIGURATION_DESCRIPTOR,           // bDescriptorType
        LSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (LSB)
        MSB(TOTAL_DESCRIPTOR_LENGTH),       // wTotalLength (MSB)
        0x01,                               // bNumInterfaces
        DEFAULT_CONFIGURATION,              // bConfigurationValue
        0x00,                               // iConfiguration
        C_RESERVED | C_SELF_POWERED,        // bmAttributes
        C_POWER(0),                         // bMaxPower

        INTERFACE_DESCRIPTOR_LENGTH,        // bLength
        INTERFACE_DESCRIPTOR,               // bDescriptorType
        0x00,                               // bInterfaceNumber
        0x00,                               // bAlternateSetting
        0x02,                               // bNumEndpoints
        HID_CLASS,                          // bInterfaceClass
        HID_SUBCLASS_BOOT,                  // bInterfaceSubClass
        HID_PROTOCOL_NONE,                  // bInterfaceProtocol
        0x00,                               // iInterface

        HID_DESCRIPTOR_LENGTH,              // bLength
        HID_DESCRIPTOR,                     // bDescriptorType
        LSB(HID_VERSION_1_11),              // bcdHID (LSB)
        MSB(HID_VERSION_1_11),              // bcdHID (MSB)
        0x00,                               // bCountryCode
        0x01,                               // bNumDescriptors
        REPORT_DESCRIPTOR,                  // bDescriptorType
        (uint8_t)(LSB(report_desc_length())), // wDescriptorLength (LSB)
        (uint8_t)(MSB(report_desc_length())), // wDescriptorLength (MSB)

        ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
        ENDPOINT_DESCRIPTOR,                // bDescriptorType
        _int_in,                            // bEndpointAddress
        E_INTERRUPT,                        // bmAttributes
        LSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (LSB)
        MSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (MSB)
        1,                                  // bInterval (milliseconds)

        ENDPOINT_DESCRIPTOR_LENGTH,         // bLength
        ENDPOINT_DESCRIPTOR,                // bDescriptorType
        _int_out,                           // bEndpointAddress
        E_INTERRUPT,                        // bmAttributes
        LSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (LSB)
        MSB(MAX_HID_REPORT_SIZE),           // wMaxPacketSize (MSB)1
        1,                                  // bInterval (milliseconds)
    };
    MBED_ASSERT(sizeof(configuration_descriptor_temp) == sizeof(_configuration_descriptor));
    memcpy(_configuration_descriptor, configuration_descriptor_temp, sizeof(_configuration_descriptor));
    return _configuration_descriptor;
}