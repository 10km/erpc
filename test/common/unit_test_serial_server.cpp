/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "myAlloc.h"
#include "Logging.h"
#include "basic_codec.h"
#include "serial_transport.h"
#include "simple_server.h"
#include "unit_test.h"
#include "test_unit_test_common_server.h"
#include <stdlib.h>

using namespace erpc;

class MyMessageBufferFactory : public MessageBufferFactory
{
public:
    virtual MessageBuffer create()
    {
        uint8_t *buf = new uint8_t[1024];
        return MessageBuffer(buf, 1024);
    }

    virtual void dispose(MessageBuffer *buf)
    {
        assert(buf);
        if (*buf)
        {
            delete[] buf->get();
        }
    }
};

SerialTransport g_transport("/dev/ttyS4", 115200);
MyMessageBufferFactory g_msgFactory;
BasicCodecFactory g_basicCodecFactory;
SimpleServer g_server;

int MyAlloc::allocated_ = 0;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
int main(int argc, const char *argv[])
{
    // create logger instance
    StdoutLogger *m_logger = new StdoutLogger();
    m_logger->setFilterLevel(Logger::kInfo);
    Log::setLogger(m_logger);
    Log::info("Starting ERPC server...\n");

    uint8_t vtime = 0;
    uint8_t vmin = 1;
    while (kErpcStatus_Success != g_transport.init(vtime, vmin))
        ;

    g_server.setMessageBufferFactory(&g_msgFactory);
    g_server.setTransport(&g_transport);
    g_server.setCodecFactory(&g_basicCodecFactory);

    add_services(&g_server);
    add_common_service(&g_server);
    // run server infinitely
    g_server.run();

    //    Thread::sleep(10000000);
    free(m_logger);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// Server helper functions
////////////////////////////////////////////////////////////////////////////////
void add_common_service(SimpleServer *server)
{
    Common_service *svc = new Common_service();

    server->addService(svc);
}

extern "C" void erpc_add_service_to_server(void *service)
{
}
////////////////////////////////////////////////////////////////////////////////
// Common service implementations here
////////////////////////////////////////////////////////////////////////////////
void quit()
{
    exit(0);
}

int32_t getServerAllocated()
{
    int result = MyAlloc::allocated();
    MyAlloc::allocated(0);
    return result;
}
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
