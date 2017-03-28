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

#include "erpc_server_setup.h"
#include "test_enums_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

struct saveVariables
{
    enumColor enumColorA;
    enumColor enumColorB;
};

struct saveVariables saveVariables1;

void test_enumColor_in(enumColor a)
{
    saveVariables1.enumColorA = a;
}

void test_enumColor_in2(enumColor b)
{
    saveVariables1.enumColorB = b;
}

void test_enumColor_out(enumColor *c)
{
    *c = saveVariables1.enumColorA;
}

void test_enumColor_outbyref(enumColor *d)
{
    *d = saveVariables1.enumColorB;
}

void test_enumColor_inout(enumColor *e)
{
    *e = (enumColor)((int32_t)*e - 9);
}

enumColor test_enumColor_return()
{
    return (enumColor)((int32_t)saveVariables1.enumColorB + 9);
}

enumColor test_enumColor_allDirection(enumColor a, enumColor b, enumColor *c, enumColor *d, enumColor *e)
{
    *c = a;
    *d = b;
    *e = b;
    return a;
}

enumColor2 test_enumColor2_allDirection(enumColor2 a, enumColor2 b, enumColor2 *c, enumColor2 *d, enumColor2 *e)
{
    *c = a;
    *d = b;
    *e = b;
    return a;
}

AnonymousEnum getAnonymousEnum(AnonymousEnum a)
{
    return (AnonymousEnum)((int32_t)a + 7);
}

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    // NOTE: possible memory leak? not ever deleting
    EnumsService_service *svc = new EnumsService_service();

    // add services
    server->addService(svc);
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server()
{
    erpc_add_service_to_server(create_EnumsService_service());
}
#ifdef __cplusplus
}
#endif
