/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * Copyright 2020-2021 ACRIOS Systems s.r.o.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_arbitrated_client_setup.h"

#include "erpc_arbitrated_client_manager.h"
#include "erpc_basic_codec.h"
#include "erpc_manually_constructed.h"
#include "erpc_message_buffer.h"
#include "erpc_transport_arbitrator.h"
#if ERPC_NESTED_CALLS
#include "erpc_threading.h"
#endif

#if ERPC_THREADS_IS(NONE)
#error "Arbitrator code does not work in no-threading configuration."
#endif

using namespace erpc;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////

// global client variables
ERPC_MANUALLY_CONSTRUCTED(ArbitratedClientManager, s_client);
ClientManager *g_client;
#pragma weak g_client

ERPC_MANUALLY_CONSTRUCTED(BasicCodecFactory, s_codecFactory);
ERPC_MANUALLY_CONSTRUCTED(TransportArbitrator, s_arbitrator);
ERPC_MANUALLY_CONSTRUCTED(BasicCodec, s_codec);
ERPC_MANUALLY_CONSTRUCTED(Crc16, s_crc16);

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

erpc_transport_t erpc_arbitrated_client_init(erpc_transport_t transport, erpc_mbf_t message_buffer_factory)
{
    erpc_assert(transport);

    Transport *castedTransport;

    // Init factories.
    s_codecFactory.construct();

    // Create codec used by the arbitrator.
    s_codec.construct();

    // Init the arbitrator using the passed in transport.
    s_arbitrator.construct();
    castedTransport = reinterpret_cast<Transport *>(transport);
    s_crc16.construct();
    castedTransport->setCrc16(s_crc16.get());
    s_arbitrator->setSharedTransport(castedTransport);
    s_arbitrator->setCodec(s_codec);

    // Init the client manager.
    s_client.construct();
    s_client->setArbitrator(s_arbitrator);
    s_client->setCodecFactory(s_codecFactory);
    s_client->setMessageBufferFactory(reinterpret_cast<MessageBufferFactory *>(message_buffer_factory));
    g_client = s_client;

    return reinterpret_cast<erpc_transport_t>(s_arbitrator.get());
}

void erpc_arbitrated_client_set_error_handler(client_error_handler_t error_handler)
{
    if (g_client != NULL)
    {
        g_client->setErrorHandler(error_handler);
    }
}

void erpc_arbitrated_client_set_crc(uint32_t crcStart)
{
    s_crc16->setCrcStart(crcStart);
}

#if ERPC_NESTED_CALLS
void erpc_arbitrated_client_set_server(erpc_server_t server)
{
    if (g_client != NULL)
    {
        g_client->setServer(reinterpret_cast<Server *>(server));
    }
}

void erpc_arbitrated_client_set_server_thread_id(void *serverThreadId)
{
    if (g_client != NULL)
    {
        g_client->setServerThreadId(reinterpret_cast<Thread::thread_id_t *>(serverThreadId));
    }
}
#endif

#if ERPC_MESSAGE_LOGGING
bool erpc_arbitrated_client_add_message_logger(erpc_transport_t transport)
{
    bool retVal;

    if (g_client == NULL)
    {
        retVal = false;
    }
    else
    {
        retVal = g_client->addMessageLogger(reinterpret_cast<Transport *>(transport));
    }

    return retVal;
}
#endif

#if ERPC_PRE_POST_ACTION
void erpc_arbitrated_client_add_pre_cb_action(pre_post_action_cb preCB)
{
    erpc_assert(g_client);

    g_client->addPreCB(preCB);
}

void erpc_arbitrated_client_add_post_cb_action(pre_post_action_cb postCB)
{
    erpc_assert(g_client);

    g_client->addPostCB(postCB);
}
#endif

void erpc_arbitrated_client_deinit(void)
{
    s_client.destroy();
    s_codecFactory.destroy();
    s_codec.destroy();
    s_arbitrator.destroy();
    g_client = NULL;
}
