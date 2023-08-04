/*
 *  Copyright (c) 2018, Vit Holasek
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include <openthread/config.h>

#include <openthread/mqttsn.h>
#include "common/code_utils.hpp"
#include "mqttsn/mqttsn_gateway_list.hpp"

#include "test_platform.h"
#include "test_util.h"

using namespace ot::Mqttsn;

#define TIMER_STARTUP_TIME 1000000

static uint32_t sNow;

static uint32_t testTimerAlarmGetNow(void)
{
    return sNow;
}

static void SetNow(uint32_t millis)
{
    sNow = millis;
}

void InitTestTimer(void)
{
    g_testPlatAlarmGetNow  = testTimerAlarmGetNow;
}

template <typename ItemType>
static StaticListEntry<ItemType> *ListFind(StaticArrayList<ItemType> &list, const ItemType &value)
{
    if (list.IsEmpty() || list.GetHead() == NULL)
    {
        return NULL;
    }
    StaticListEntry<ItemType> *entry = list.GetHead();
    do
    {
        if (value == entry->GetValue())
        {
            return entry;
        }
        entry = entry->GetNext();
    } while (entry != NULL);
    return NULL;
}

template <typename ItemType>
static bool ListContains(StaticArrayList<ItemType> &list, const ItemType &value)
{
    return ListFind<ItemType>(list, value) != NULL;
}

static GatewayInfo CreateGatewayInfo1()
{
    ot::Ip6::Address address = ot::Ip6::Address();
    address.FromString("fd11:22:0:0:6648:fb52:b05:43f5");
    return GatewayInfo(2, address, 123456, 900000);
}

static GatewayInfo CreateGatewayInfo2()
{
    ot::Ip6::Address address = ot::Ip6::Address();
    address.FromString("2018:ff9b::ac12:8");
    return GatewayInfo(3, address, 654321, 1000);
}

static GatewayInfo CreateGatewayInfo3()
{
    ot::Ip6::Address address = ot::Ip6::Address();
    address.FromString("2018:ff9b::c0a8:2d");
    return GatewayInfo(3, address, 10000, 2000);
}

void TestListAddMultipleItems(void)
{
    GatewayInfo gateway1 = CreateGatewayInfo1();
    GatewayInfo gateway2 = CreateGatewayInfo2();
    StaticListEntry<GatewayInfo> itemArray[5];
    StaticArrayList<GatewayInfo> list(itemArray, 5);

    printf("\nTest 1: Test adding multiple items to StaticArrayList\n");

    SuccessOrQuit(list.Add(gateway1), "StaticArrayList::Add() failed");
    SuccessOrQuit(list.Add(gateway2), "StaticArrayList::Add() failed");

    VerifyOrQuit(ListContains<GatewayInfo>(list, gateway1), "List does not contain gateway1");
    VerifyOrQuit(ListContains<GatewayInfo>(list, gateway2), "List does not contain gateway2");

    printf(" -- PASS\n");
}

void TestListSize(void)
{
    GatewayInfo gateway1 = CreateGatewayInfo1();
    GatewayInfo gateway2 = CreateGatewayInfo2();
    StaticListEntry<GatewayInfo> itemArray[5];
    StaticArrayList<GatewayInfo> list(itemArray, 5);

    printf("\nTest 2: Test StaticArrayList Size\n");

    VerifyOrQuit(list.Size() == 0, "Size() must be 0");

    SuccessOrQuit(list.Add(gateway1), "StaticArrayList::Add() failed");
    VerifyOrQuit(list.Size() == 1, "Size() has wrong value");

    SuccessOrQuit(list.Add(gateway2), "StaticArrayList::Add() failed");
    VerifyOrQuit(list.Size() == 2, "Size() has wrong value");

    uint16_t i = 0;
    StaticListEntry<GatewayInfo> *entry = list.GetHead();
    while (entry != NULL)
    {
        i++;
        entry = entry->GetNext();
    }
    VerifyOrQuit(i == 2, "Wrong number of StaticListEntry items");

    printf(" -- PASS\n");
}

void TestListRemoveHead(void)
{
    GatewayInfo gateway1 = CreateGatewayInfo1();
    StaticListEntry<GatewayInfo> itemArray[5];
    StaticArrayList<GatewayInfo> list(itemArray, 5);

    printf("\nTest 3: Test add single item to StatisArrayList and remove it\n");

    SuccessOrQuit(list.Add(gateway1), "StaticArrayList::Add() failed");
    StaticListEntry<GatewayInfo> *entry = list.GetHead();
    SuccessOrQuit(list.Remove(*entry), "StaticArrayList::Remove() failed");

    VerifyOrQuit(list.Size() == 0, "Size() must be 0");
    VerifyOrQuit(list.IsEmpty(), "IsEmpty() must be true");

    printf(" -- PASS\n");
}

void TestListRemoveItemFirst(void)
{
    GatewayInfo gateway1 = CreateGatewayInfo1();
    GatewayInfo gateway2 = CreateGatewayInfo2();
    StaticListEntry<GatewayInfo> itemArray[5];
    StaticArrayList<GatewayInfo> list(itemArray, 5);

    printf("\nTest 4: Test add two items to StatisArrayList and remove the last one\n");

    SuccessOrQuit(list.Add(gateway1), "StaticArrayList::Add() failed");
    SuccessOrQuit(list.Add(gateway2), "StaticArrayList::Add() failed");

    StaticListEntry<GatewayInfo> *entry = ListFind<GatewayInfo>(list, gateway1);
    SuccessOrQuit(list.Remove(*entry), "StaticArrayList::Remove() failed");

    VerifyOrQuit(ListContains<GatewayInfo>(list, gateway2), "List does not contain gateway2");
    VerifyOrQuit(!ListContains<GatewayInfo>(list, gateway1), "List still contain gateway1");	
	VerifyOrQuit(list.Size() == 1, "Size() has wrong value");

    printf(" -- PASS\n");
}

void TestListRemoveItemLast(void)
{
    GatewayInfo gateway1 = CreateGatewayInfo1();
    GatewayInfo gateway2 = CreateGatewayInfo2();
    StaticListEntry<GatewayInfo> itemArray[5];
    StaticArrayList<GatewayInfo> list(itemArray, 5);

    printf("\nTest 5: Test add two items to StatisArrayList and remove the last one\n");

    SuccessOrQuit(list.Add(gateway1), "StaticArrayList::Add() failed");
    SuccessOrQuit(list.Add(gateway2), "StaticArrayList::Add() failed");

    StaticListEntry<GatewayInfo> *entry = ListFind<GatewayInfo>(list, gateway2);
    SuccessOrQuit(list.Remove(*entry), "StaticArrayList::Remove() failed");

    VerifyOrQuit(ListContains<GatewayInfo>(list, gateway1), "List does not contain gateway1");
    VerifyOrQuit(!ListContains<GatewayInfo>(list, gateway2), "List still contain gateway2");	
	VerifyOrQuit(list.Size() == 1, "Size() has wrong value");

    printf(" -- PASS\n");
}

void TestListAddToFullList(void)
{
    GatewayInfo gateway1 = CreateGatewayInfo1();
    GatewayInfo gateway2 = CreateGatewayInfo2();
    GatewayInfo gateway3 = CreateGatewayInfo3();
    StaticListEntry<GatewayInfo> itemArray[2];
    StaticArrayList<GatewayInfo> list(itemArray, 2);

    printf("\nTest 6: Test add to full list fails\n");

    SuccessOrQuit(list.Add(gateway1), "StaticArrayList::Add() failed");
    SuccessOrQuit(list.Add(gateway2), "StaticArrayList::Add() failed");

    VerifyOrQuit(list.Add(gateway3) == OT_ERROR_NO_BUFS, "Add() should fail");
    VerifyOrQuit(list.Size() == 2, "Size() has wrong value");

    printf(" -- PASS\n");
}

void TestListAddToFullListAfterRemove(void)
{
    GatewayInfo gateway1 = CreateGatewayInfo1();
    GatewayInfo gateway2 = CreateGatewayInfo2();
    GatewayInfo gateway3 = CreateGatewayInfo3();
    StaticListEntry<GatewayInfo> itemArray[2];
    StaticArrayList<GatewayInfo> list(itemArray, 2);

    printf("\nTest 7: Test remove and add to full list fails\n");

    SuccessOrQuit(list.Add(gateway1), "StaticArrayList::Add() failed");
    SuccessOrQuit(list.Add(gateway2), "StaticArrayList::Add() failed");

    StaticListEntry<GatewayInfo> *entry = ListFind<GatewayInfo>(list, gateway2);
    SuccessOrQuit(list.Remove(*entry), "StaticArrayList::Remove() failed");

    SuccessOrQuit(list.Add(gateway3), "StaticArrayList::Add() failed");
    VerifyOrQuit(ListContains<GatewayInfo>(list, gateway3), "List does not contain gateway3");

    printf(" -- PASS\n");
}

void TestActiveGatewayListInfoNotRemovedBeforeKeepaliveTimeout(void)
{
    GatewayInfo gateway1 = CreateGatewayInfo1();
    uint32_t duration = 900000;
    ActiveGatewayList list = ActiveGatewayList();

    printf("\nTest 8: Test GatewayInfo is not removed from ActiveGatewayList before keepalive timeout\n");
    SetNow(TIMER_STARTUP_TIME);
    list.Add(gateway1.GetGatewayId(), gateway1.GetGatewayAddress(), duration);
    SetNow(TIMER_STARTUP_TIME + 1000);
    SuccessOrQuit(list.HandleTimer(), "ActiveGatewayList::HandleTimer() failed");
    VerifyOrQuit(list.GetList().Size() == 1, "List is empty");
}

void TestActiveGatewayListInfoRemovedAfterKeepaliveTimeout(void)
{
    GatewayInfo gateway1 = CreateGatewayInfo1();
    uint32_t duration = 900000;
    ActiveGatewayList list = ActiveGatewayList();

    printf("\nTest 9: Test GatewayInfo is removed from ActiveGatewayList after keepalive timeout\n");
    SetNow(TIMER_STARTUP_TIME);
    list.Add(gateway1.GetGatewayId(), gateway1.GetGatewayAddress(), duration);	
    SetNow(TIMER_STARTUP_TIME + 901000);	
    SuccessOrQuit(list.HandleTimer(), "ActiveGatewayList::HandleTimer() failed");
    VerifyOrQuit(list.GetList().Size() == 0, "GatewayInfo not removed");
}

int main(void)
{
    InitTestTimer();
    TestListAddMultipleItems();
    TestListSize();
    TestListRemoveHead();
    TestListRemoveItemFirst();
	TestListRemoveItemLast();
    TestListAddToFullList();
    TestListAddToFullListAfterRemove();
    TestActiveGatewayListInfoNotRemovedBeforeKeepaliveTimeout();
    TestActiveGatewayListInfoRemovedAfterKeepaliveTimeout();
    printf("\nAll tests passed.\n");
    return 0;
}
