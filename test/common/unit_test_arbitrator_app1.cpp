/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
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

#include "unit_test.h"

#include "erpc_arbitrated_client_setup.h"
#include "erpc_mbf_setup.h"
#include "erpc_server_setup.h"
#include "erpc_transport_setup.h"

#include "test_arbitrator_firstInterface_server.h"
#include "test_arbitrator_secondInterface.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "app_core1.h"
#include "board.h"
#include "mcmgr.h"
#include "rpmsg_lite.h"
#ifdef __cplusplus
}
#endif
using namespace erpc;

int testClient();

SemaphoreHandle_t g_waitQuitMutex;
TaskHandle_t g_serverTask;
TaskHandle_t g_clientTask;

int waitQuit = 0;
int waitClient = 0;
int isTestPassing = 0;
uint32_t startupData;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
void increaseWaitQuit()
{
    xSemaphoreTake(g_waitQuitMutex, portMAX_DELAY);
    waitQuit++;
    xSemaphoreGive(g_waitQuitMutex);
}

void runServer(void *arg)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    erpc_status_t err;
    err = erpc_server_run();
    increaseWaitQuit();

    if (err != kErpcStatus_Success)
    {
        // server error
        while (1)
        {
        }
    }
    vTaskSuspend(NULL);
}

void runClient(void *arg)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // wait until ERPC first (client) app will announce that it is ready.
    while (waitClient == 0)
    {
        vTaskDelay(10);
    }

    // wait until ERPC first (client) app will announce ready to quit state
    while (true)
    {
        isTestPassing = testClient();

        if (waitQuit != 0 || isTestPassing != 0)
        {
            break;
        }
        vTaskDelay(10);
    }

    while (true)
    {
        if (waitQuit != 0)
        {
            break;
        }
        vTaskDelay(100);
    }

    // send to ERPC first (client) app ready to quit state
    quitSecondInterfaceServer();
    increaseWaitQuit();
    vTaskSuspend(NULL);
}

static void SignalReady(void)
{
    // Signal the other core we are ready */
    MCMGR_SignalReady(kMCMGR_Core1);
}

void runInit(void *arg)
{
    // Initialize MCMGR before calling its API
    MCMGR_Init();

    // Get the startup data
    MCMGR_GetStartupData(kMCMGR_Core1, &startupData);

    // RPMsg-Lite transport layer initialization
    erpc_transport_t transportClient;
    erpc_transport_t transportServer;
    transportClient = erpc_transport_rpmsg_lite_rtos_remote_init(101, 100, (void *)startupData,
                                                                 0, SignalReady);

    // MessageBufferFactory initialization
    erpc_mbf_t message_buffer_factory;
    message_buffer_factory = erpc_mbf_rpmsg_zc_init(transportClient);

    // eRPC client side initialization
    transportServer = erpc_arbitrated_client_init(transportClient, message_buffer_factory);

    // eRPC server side initialization
    erpc_server_init(transportServer, message_buffer_factory);

    // adding the service to the server
    erpc_add_service_to_server(create_FirstInterface_service());

    // unblock server and client task
    xTaskNotifyGive(g_serverTask);
    xTaskNotifyGive(g_clientTask);

    // Wait until client side will stop.
    while (true)
    {
        if (waitQuit >= 3)
        {
            break;
        }
        vTaskDelay(100);
    }

    vTaskSuspend(NULL);
}

int main(int argc, char **argv)
{
    BOARD_InitHardware();

    g_waitQuitMutex = xSemaphoreCreateMutex();
    xTaskCreate(runInit, "runInit", 256, NULL, 1, NULL);
    xTaskCreate(runServer, "runServer", 512, NULL, 3, &g_serverTask);
    xTaskCreate(runClient, "runClient", 512, NULL, 2, &g_clientTask);

    vTaskStartScheduler();

    while (1)
    {
    }
}

int32_t getResultFromSecondSide()
{
    increaseWaitQuit();
    return isTestPassing;
}

void quitFirstInterfaceServer()
{
    // Stop server part
    erpc_server_stop();
}

void whenReady()
{
    waitClient++;
}

int testClient()
{
    int number = 15;
    for (int i = 0; i < number; i++)
    {
        secondSendInt(i + number);
    }
    for (int i = number - 1; i >= 0; i--)
    {
        if (i + number != secondReceiveInt())
        {
            return -1;
        }
    }
    return 0;
}
