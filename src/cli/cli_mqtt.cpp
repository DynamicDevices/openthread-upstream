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

#include "openthread-core-config.h"

#if OPENTHREAD_CONFIG_MQTTSN_ENABLE

#include <ctype.h>

#include "cli/cli.hpp"
#include "common/code_utils.hpp"
#include "utils/parse_cmdline.hpp"

#include <openthread/mqttsn.h>

using ot::Utils::CmdLineParser::ParseAsUint16;

namespace ot {
namespace Cli {

Mqtt::Mqtt(otInstance *aInstance, OutputImplementer &aOutputImplementer)
    : Output(aInstance, aOutputImplementer)
{
    ;
}

template <> otError Mqtt::Process<Cmd("start")>(Arg aArgs[])
{
    otError error;
    uint16_t port = OPENTHREAD_CONFIG_MQTTSN_DEFAULT_PORT;
    
    if (!aArgs[0].IsEmpty())
    {
        SuccessOrExit(error = aArgs[0].ParseAsUint16(port));
    }

    SuccessOrExit(error = otMqttsnSetPublishReceivedHandler(GetInstancePtr(), &Mqtt::HandlePublishReceived, this));
    SuccessOrExit(error = otMqttsnStart(GetInstancePtr(), port));
exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("stop")>(Arg aArgs[])
{
    OT_UNUSED_VARIABLE(aArgs);

    return otMqttsnStop(GetInstancePtr());
}

template <> otError Mqtt::Process<Cmd("connect")>(Arg aArgs[])
{
	otError error;
	otIp6Address destinationIp;
	uint16_t destinationPort;
    bool nat64SynthesizedAddress;

    if (aArgs[1].IsEmpty())
    {
    	ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = Interpreter::ParseToIp6Address(GetInstancePtr(), aArgs[0], destinationIp, nat64SynthesizedAddress));
    SuccessOrExit(error = aArgs[1].ParseAsUint16(destinationPort));
    SuccessOrExit(error = otMqttsnSetConnectedHandler(GetInstancePtr(), &Mqtt::HandleConnected, this));
    SuccessOrExit(error = otMqttsnSetDisconnectedHandler(GetInstancePtr(), &Mqtt::HandleDisconnected, this));
    SuccessOrExit(error = otMqttsnConnectDefault(GetInstancePtr(), &destinationIp, destinationPort));

exit:
	return error;
}

template <> otError Mqtt::Process<Cmd("reconnect")>(Arg aArgs[])
{
    OT_UNUSED_VARIABLE(aArgs);

    return otMqttsnReconnect(GetInstancePtr());
}

template <> otError Mqtt::Process<Cmd("subscribe")>(Arg aArgs[])
{
    otError error;
    otMqttsnQos qos = kQos1;
    otMqttsnTopic topic;
    if (aArgs[0].IsEmpty())
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = ParseTopic(aArgs[0], &topic));
    if (!aArgs[1].IsEmpty())
    {
        SuccessOrExit(error = otMqttsnStringToQos(aArgs[1].GetCString(), &qos));
    }
    SuccessOrExit(error = otMqttsnSubscribe(GetInstancePtr(), &topic, qos, &Mqtt::HandleSubscribed, this));
exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("state")>(Arg aArgs[])
{
    OT_UNUSED_VARIABLE(aArgs);

    otError error;
    otMqttsnClientState clientState;
    const char *clientStateString;
    clientState = otMqttsnGetState(GetInstancePtr());
    SuccessOrExit(error = otMqttsnClientStateToString(clientState, &clientStateString));
    OutputFormat("%s\r\n", clientStateString);
exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("register")>(Arg aArgs[])
{
    otError error;
    char *topicName;
    if (aArgs[0].IsEmpty())
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    topicName = aArgs[0].GetCString();
    SuccessOrExit(error = otMqttsnRegister(GetInstancePtr(), topicName, &HandleRegistered, this));
exit:
    return error;
}

otError Mqtt::ParseTopic(const Arg &aArg, otMqttsnTopic *aTopic)
{
    otError error = OT_ERROR_NONE;
    const char *value = aArg.GetCString();
    uint16_t topicId = 0;

    // Parse topic
    // If string starts with '@' it will be considered as normal topic ID
    // If string starts with '$' it will be considered as predefined topic ID
    // Otherwise it is short topic name
    if (value[0] == '@')
    {
        SuccessOrExit(error = ParseAsUint16(&value[1], topicId));
        *aTopic = otMqttsnCreateTopicId((otMqttsnTopicId)topicId);
    }
    else if (value[0] == '$')
    {
        SuccessOrExit(error = ParseAsUint16(&value[1], topicId));
        *aTopic = otMqttsnCreatePredefinedTopicId((otMqttsnTopicId)topicId);
    }
    else
    {
        *aTopic = otMqttsnCreateTopicName(value);
    }

exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("publish")>(Arg aArgs[])
{
    otError error;
    otMqttsnQos qos = kQos1;
    const char* data = "";
    int32_t length = 0;
    otMqttsnTopic topic;

    if (aArgs[1].IsEmpty())
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = ParseTopic(aArgs[0], &topic));
    SuccessOrExit(error = otMqttsnStringToQos(aArgs[1].GetCString(), &qos));
    if (!aArgs[2].IsEmpty())
    {
        data = aArgs[2].GetCString();
        length = strlen(aArgs[2].GetCString());
    }
    SuccessOrExit(error = otMqttsnPublish(GetInstancePtr(), (uint8_t *)data,
        length, qos, false, &topic, &Mqtt::HandlePublished, this));
exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("publishm1")>(Arg aArgs[])
{
    otError error;
    otIp6Address destinationIp;
    uint16_t destinationPort;
    const char* data = "";
    int32_t length = 0;
    otMqttsnTopic topic;
    bool nat64SynthesizedAddress;

    if (aArgs[2].IsEmpty())
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }

    SuccessOrExit(error = Interpreter::ParseToIp6Address(GetInstancePtr(), aArgs[0], destinationIp, nat64SynthesizedAddress));
    SuccessOrExit(error = aArgs[1].ParseAsUint16(destinationPort));
    SuccessOrExit(error = ParseTopic(aArgs[2], &topic));
    if (!aArgs[3].IsEmpty())
    {
        data = aArgs[3].GetCString();
        length = strlen(aArgs[3].GetCString());
    }

    SuccessOrExit(error = otMqttsnPublishQosm1(GetInstancePtr(), (uint8_t *)data,
        length, false, &topic, &destinationIp, destinationPort));
exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("unsubscribe")>(Arg aArgs[])
{
    otError error;
    otMqttsnTopic topic;

    if (aArgs[0].IsEmpty())
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }

    SuccessOrExit(error = ParseTopic(aArgs[0], &topic));
    SuccessOrExit(error = otMqttsnUnsubscribe(GetInstancePtr(), &topic, &Mqtt::HandleUnsubscribed, this));
exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("disconnect")>(Arg aArgs[])
{
    OT_UNUSED_VARIABLE(aArgs);

    return otMqttsnDisconnect(GetInstancePtr());
}

template <> otError Mqtt::Process<Cmd("sleep")>(Arg aArgs[])
{
    otError error;
    uint16_t duration;

    if (aArgs[0].IsEmpty())
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = aArgs[0].ParseAsUint16(duration));
    SuccessOrExit(error = otMqttsnSleep(GetInstancePtr(), duration));
exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("awake")>(Arg aArgs[])
{
    otError error;
    uint32_t timeout;

    if (aArgs[0].IsEmpty())
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = aArgs[0].ParseAsUint32(timeout));
    SuccessOrExit(error = otMqttsnAwake(GetInstancePtr(), timeout));
exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("searchgw")>(Arg aArgs[])
{
    otError error;
    otIp6Address multicastAddress;
    uint16_t port;
    uint8_t radius;

    if (aArgs[2].IsEmpty())
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = aArgs[0].ParseAsIp6Address(multicastAddress));
    SuccessOrExit(error = aArgs[1].ParseAsUint16(port));
    SuccessOrExit(error = aArgs[2].ParseAsUint8(radius));
    SuccessOrExit(error = otMqttsnSetSearchgwHandler(GetInstancePtr(), &Mqtt::HandleSearchgwResponse, this));
    SuccessOrExit(error = otMqttsnSearchGateway(GetInstancePtr(), &multicastAddress, port, radius));
exit:
    return error;
}

template <> otError Mqtt::Process<Cmd("gateways")>(Arg aArgs[])
{
    OT_UNUSED_VARIABLE(aArgs);

    otMqttsnGatewayInfo gateways[kMaxGatewayInfoCount];
    uint16_t gatewayCount;
    gatewayCount = otMqttsnGetActiveGateways(GetInstancePtr(), gateways, sizeof(gateways));
    for (uint16_t i = 0; i < gatewayCount; i++)
    {
        otMqttsnGatewayInfo &info = gateways[i];
        OutputFormat("gateway ");
        OutputIp6Address(*static_cast<Ip6::Address *>(&info.mGatewayAddress));
        OutputFormat(": gateway_id=%u\r\n", (uint16_t)info.mGatewayId);
    }
    return OT_ERROR_NONE;
}

otError Mqtt::Process(Arg aArgs[])
{
#define CmdEntry(aCommandString)                            \
    {                                                       \
        aCommandString, &Mqtt::Process<Cmd(aCommandString)> \
    }

    static constexpr Command kCommands[] = {
        CmdEntry("awake"),    CmdEntry("connect"),   CmdEntry("disconnect"), CmdEntry("gateways"),
        CmdEntry("publish"),  CmdEntry("publishm1"), CmdEntry("reconnect"),  CmdEntry("register"), 
        CmdEntry("searchgw"), CmdEntry("sleep"),     CmdEntry("start"),      CmdEntry("state"),
        CmdEntry("stop"),     CmdEntry("subscribe"), CmdEntry("unsubscribe"),
    };

    static_assert(BinarySearch::IsSorted(kCommands), "kCommands is not sorted");

    otError        error = OT_ERROR_INVALID_COMMAND;
    const Command *command;

    if (aArgs[0].IsEmpty() || (aArgs[0] == "help"))
    {
        OutputCommandTable(kCommands);
        ExitNow(error = aArgs[0].IsEmpty() ? error : OT_ERROR_NONE);
    }

    command = BinarySearch::Find(aArgs[0].GetCString(), kCommands);
    VerifyOrExit(command != nullptr);

    error = (this->*command->mHandler)(aArgs + 1);

exit:
    return error;
}

void Mqtt::HandleConnected(otMqttsnReturnCode aCode, void *aContext)
{
	static_cast<Mqtt *>(aContext)->HandleConnected(aCode);
}

void Mqtt::HandleConnected(otMqttsnReturnCode aCode)
{
	if (aCode == kCodeAccepted)
	{
		OutputFormat("connected\r\n");
	}
	else
	{
	    PrintFailedWithCode("connect", aCode);
	}
}

void Mqtt::HandleSubscribed(otMqttsnReturnCode aCode, const otMqttsnTopic *aTopic, otMqttsnQos aQos, void* aContext)
{
    static_cast<Mqtt *>(aContext)->HandleSubscribed(aCode, aTopic, aQos);
}

void Mqtt::HandleSubscribed(otMqttsnReturnCode aCode, const otMqttsnTopic *aTopic, otMqttsnQos aQos)
{
    OT_UNUSED_VARIABLE(aQos);
    if (aCode == kCodeAccepted)
    {
        OutputFormat("subscribed topic id:");
        if (aTopic != NULL)
        {
            OutputFormat("%u\r\n", otMqttsnGetTopicId(aTopic));
        }
    }
    else
    {
        PrintFailedWithCode("subscribe", aCode);
    }
}

void Mqtt::HandleRegistered(otMqttsnReturnCode aCode, const otMqttsnTopic *aTopic, void* aContext)
{
    static_cast<Mqtt *>(aContext)->HandleRegistered(aCode, aTopic);
}

void Mqtt::HandleRegistered(otMqttsnReturnCode aCode, const otMqttsnTopic *aTopic)
{
    if (aCode == kCodeAccepted)
    {
        OutputFormat("registered topic id:%u\r\n", otMqttsnGetTopicId(aTopic));
    }
    else
    {
        PrintFailedWithCode("register", aCode);
    }
}

void Mqtt::HandlePublished(otMqttsnReturnCode aCode, void* aContext)
{
    static_cast<Mqtt *>(aContext)->HandlePublished(aCode);
}

void Mqtt::HandlePublished(otMqttsnReturnCode aCode)
{
    if (aCode == kCodeAccepted)
    {
        OutputFormat("published\r\n");
    }
    else
    {
        PrintFailedWithCode("publish", aCode);
    }
}

void Mqtt::HandleUnsubscribed(otMqttsnReturnCode aCode, void* aContext)
{
    static_cast<Mqtt *>(aContext)->HandleUnsubscribed(aCode);
}

void Mqtt::HandleUnsubscribed(otMqttsnReturnCode aCode)
{
    if (aCode == kCodeAccepted)
    {
        OutputFormat("unsubscribed\r\n");
    }
    else
    {
        PrintFailedWithCode("unsubscribe", aCode);
    }
}

otMqttsnReturnCode Mqtt::HandlePublishReceived(const uint8_t* aPayload, int32_t aPayloadLength, const otMqttsnTopic *aTopic, void* aContext)
{
    return static_cast<Mqtt *>(aContext)->HandlePublishReceived(aPayload, aPayloadLength, aTopic);
}

otMqttsnReturnCode Mqtt::HandlePublishReceived(const uint8_t* aPayload, int32_t aPayloadLength, const otMqttsnTopic *aTopic)
{
    if (aTopic->mType == kTopicId)
    {
        OutputFormat("received publish from topic id %u:\r\n", otMqttsnGetTopicId(aTopic));
    }
    else if (aTopic->mType == kShortTopicName)
    {
        OutputFormat("received publish from topic %s:\r\n", otMqttsnGetTopicName(aTopic));
    }
    OutputFormat("%.*s\r\n", static_cast<int>(aPayloadLength), aPayload);
    return kCodeAccepted;
}

void Mqtt::HandleDisconnected(otMqttsnDisconnectType aType, void* aContext)
{
    static_cast<Mqtt *>(aContext)->HandleDisconnected(aType);
}

void Mqtt::HandleDisconnected(otMqttsnDisconnectType aType)
{
    const char* disconnectTypeText;
    if (otMqttsnDisconnectTypeToString(aType, &disconnectTypeText) == OT_ERROR_NONE)
    {
        OutputFormat("disconnected reason: %s\r\n", disconnectTypeText);
    }
    else
    {
        OutputFormat("disconnected with unknown reason: %d\r\n", aType);
    }
}

void Mqtt::HandleSearchgwResponse(const otIp6Address* aAddress, uint8_t aGatewayId, void* aContext)
{
    static_cast<Mqtt *>(aContext)->HandleSearchgwResponse(aAddress, aGatewayId);
}

void Mqtt::HandleSearchgwResponse(const otIp6Address* aAddress, uint8_t aGatewayId)
{
    OutputFormat("searchgw response from ");
    OutputIp6Address(*static_cast<const Ip6::Address *>(aAddress));
    OutputFormat(": gateway_id=%u\r\n", (unsigned int)aGatewayId);
}

void Mqtt::PrintFailedWithCode(const char *aCommandName, otMqttsnReturnCode aCode)
{
    const char* codeText;
    if (otMqttsnReturnCodeToString(aCode, &codeText) == OT_ERROR_NONE)
    {
        OutputFormat("%s failed: %s\r\n", aCommandName, codeText);
    }
    else
    {
        OutputFormat("%s failed with unknown code: %d\r\n", aCommandName, aCode);
    }
}

} // namespace Cli
} // namespace ot

#endif
