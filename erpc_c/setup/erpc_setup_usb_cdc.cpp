/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_manually_constructed.h"
#include "erpc_transport_setup.h"
#include "erpc_usb_cdc_transport.h"

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

static ManuallyConstructed<UsbCdcTransport> s_usb_transport;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_transport_usb_cdc_init(void *serialHandle, void *serialConfig, void *usbCdcConfig,
                                             uint8_t *usbRingBuffer, uint32_t usbRingBufferLength)
{
    s_usb_transport.construct((serial_handle_t)serialHandle, (serial_manager_config_t *)serialConfig,
                              (serial_port_usb_cdc_config_t *)usbCdcConfig, (uint8_t *)usbRingBuffer,
                              (uint32_t)usbRingBufferLength);
    if (s_usb_transport->init() == kErpcStatus_Success)
    {
        return reinterpret_cast<erpc_transport_t>(s_usb_transport.get());
    }
    return NULL;
}
