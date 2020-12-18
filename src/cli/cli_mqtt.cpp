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

using ot::Utils::CmdLineParser::ParseAsUint8;
using ot::Utils::CmdLineParser::ParseAsUint16;
using ot::Utils::CmdLineParser::ParseAsUint32;

namespace ot {
namespace Cli {

const struct Mqtt::Command Mqtt::sCommands[] = {
    {"help", &Mqtt::ProcessHelp},               {"start", &Mqtt::ProcessStart},
    {"stop", &Mqtt::ProcessStop},               {"connect", &Mqtt::ProcessConnect},
    {"reconnect", &Mqtt::ProcessReconnect},     {"subscribe", &Mqtt::ProcessSubscribe},
    {"state", &Mqtt::ProcessState},             {"register", &Mqtt::ProcessRegister},
    {"publish", &Mqtt::ProcessPublish},         {"publishm1", &Mqtt::ProcessPublishm1},
    {"unsubscribe", &Mqtt::ProcessUnsubscribe}, {"disconnect", &Mqtt::ProcessDisconnect},
    {"sleep", &Mqtt::ProcessSleep},             {"awake", &Mqtt::ProcessAwake},
    {"searchgw", &Mqtt::ProcessSearchgw},       {"gateways", &Mqtt::ProcessGateways}
};

Mqtt::Mqtt(Interpreter &aInterpreter)
    : mInterpreter(aInterpreter)
{
    ;
}

otError Mqtt::Process(uint8_t aArgsLength, char *aArgs[])
{
    otError error = OT_ERROR_PARSE;

    if (aArgsLength < 1)
    {
        ProcessHelp(0, NULL);
        error = OT_ERROR_NONE;
    }
    else
    {
        for (size_t i = 0; i < OT_ARRAY_LENGTH(sCommands); i++)
        {
            if (strcmp(aArgs[0], sCommands[i].mName) == 0)
            {
                error = (this->*sCommands[i].mCommand)(aArgsLength, aArgs);
                break;
            }
        }
    }

    return error;
}

otError Mqtt::ProcessHelp(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    for (size_t i = 0; i < OT_ARRAY_LENGTH(sCommands); i++)
    {
        mInterpreter.OutputFormat("%s\r\n", sCommands[i].mName);
    }

    return OT_ERROR_NONE;
}

otError Mqtt::ProcessStart(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);
    otError error;
    uint16_t port = OPENTHREAD_CONFIG_MQTTSN_DEFAULT_PORT;
    if (aArgsLength > 2)
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    if (aArgsLength == 2)
    {
        SuccessOrExit(error = ParseAsUint16(aArgs[1], port));
    }

    SuccessOrExit(error = otMqttsnSetPublishReceivedHandler(mInterpreter.mInstance, &Mqtt::HandlePublishReceived, this));
    SuccessOrExit(error = otMqttsnStart(mInterpreter.mInstance, port));
exit:
    return error;
}

otError Mqtt::ProcessStop(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return otMqttsnStop(mInterpreter.mInstance);
}

otError Mqtt::ProcessConnect(uint8_t aArgsLength, char *aArgs[])
{
	otError error;
	otIp6Address destinationIp;
	uint16_t destinationPort;

    if (aArgsLength != 3)
    {
    	ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = otIp6AddressFromString(aArgs[1], &destinationIp));
    SuccessOrExit(error = ParseAsUint16(aArgs[2], destinationPort));
    SuccessOrExit(error = otMqttsnSetConnectedHandler(mInterpreter.mInstance, &Mqtt::HandleConnected, this));
    SuccessOrExit(error = otMqttsnSetDisconnectedHandler(mInterpreter.mInstance, &Mqtt::HandleDisconnected, this));
    SuccessOrExit(error = otMqttsnConnectDefault(mInterpreter.mInstance, &destinationIp, destinationPort));

exit:
	return error;
}

otError Mqtt::ProcessReconnect(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return otMqttsnReconnect(mInterpreter.mInstance);
}

otError Mqtt::ProcessSubscribe(uint8_t aArgsLength, char *aArgs[])
{
    otError error;
    otMqttsnQos qos = kQos1;
    otMqttsnTopic topic;
    if (aArgsLength < 2 || aArgsLength > 3)
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = ParseTopic(aArgs[1], &topic));
    if (aArgsLength > 2)
    {
        SuccessOrExit(error = otMqttsnStringToQos(aArgs[2], &qos));
    }
    SuccessOrExit(error = otMqttsnSubscribe(mInterpreter.mInstance, &topic, qos, &Mqtt::HandleSubscribed, this));
exit:
    return error;
}

otError Mqtt::ProcessState(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    otError error;
    otMqttsnClientState clientState;
    const char *clientStateString;
    clientState = otMqttsnGetState(mInterpreter.mInstance);
    SuccessOrExit(error = otMqttsnClientStateToString(clientState, &clientStateString));
    mInterpreter.OutputFormat("%s\r\n", clientStateString);
exit:
    return error;
}

otError Mqtt::ProcessRegister(uint8_t aArgsLength, char *aArgs[])
{
    otError error;
    char *topicName;
    if (aArgsLength != 2)
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    topicName = aArgs[1];
    SuccessOrExit(error = otMqttsnRegister(mInterpreter.mInstance, topicName, &HandleRegistered, this));
exit:
    return error;
}

otError Mqtt::ParseTopic(char *aValue, otMqttsnTopic *aTopic)
{
    otError error = OT_ERROR_NONE;
    uint16_t topicId = 0;

    // Parse topic
    // If string starts with '@' it will be considered as normal topic ID
    // If string starts with '$' it will be considered as predefined topic ID
    // Otherwise it is short topic name
    if (aValue[0] == '@')
    {
        SuccessOrExit(error = ParseAsUint16(&aValue[1], topicId));
        *aTopic = otMqttsnCreateTopicId((otMqttsnTopicId)topicId);
    }
    else if (aValue[0] == '$')
    {
        SuccessOrExit(error = ParseAsUint16(&aValue[1], topicId));
        *aTopic = otMqttsnCreatePredefinedTopicId((otMqttsnTopicId)topicId);
    }
    else
    {
        *aTopic = otMqttsnCreateTopicName(aValue);
    }

exit:
    return error;
}

otError Mqtt::ProcessPublish(uint8_t aArgsLength, char *aArgs[])
{
    otError error;
    otMqttsnQos qos = kQos1;
    const char* data = "";
    int32_t length = 0;
    otMqttsnTopic topic;

    if (aArgsLength < 3 || aArgsLength > 4)
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = ParseTopic(aArgs[1], &topic));
    SuccessOrExit(error = otMqttsnStringToQos(aArgs[2], &qos));
    if (aArgsLength > 3)
    {
        data = aArgs[3];
        length = strlen(aArgs[3]);
    }
    SuccessOrExit(error = otMqttsnPublish(mInterpreter.mInstance, (uint8_t *)data,
        length, qos, false, &topic, &Mqtt::HandlePublished, this));
exit:
    return error;
}

otError Mqtt::ProcessPublishm1(uint8_t aArgsLength, char *aArgs[])
{
    otError error;
    otIp6Address destinationIp;
    uint16_t destinationPort;
    const char* data = "";
    int32_t length = 0;
    otMqttsnTopic topic;

    if (aArgsLength < 5)
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }

    SuccessOrExit(error = otIp6AddressFromString(aArgs[1], &destinationIp));
    SuccessOrExit(error = ParseAsUint16(aArgs[2], destinationPort));
    SuccessOrExit(error = ParseTopic(aArgs[3], &topic));
    if (aArgsLength > 4)
    {
        data = aArgs[4];
        length = strlen(aArgs[4]);
    }

    SuccessOrExit(error = otMqttsnPublishQosm1(mInterpreter.mInstance, (uint8_t *)data,
        length, false, &topic, &destinationIp, destinationPort));
exit:
    return error;
}

otError Mqtt::ProcessUnsubscribe(uint8_t aArgsLength, char *aArgs[])
{
    otError error;
    otMqttsnTopic topic;

    if (aArgsLength != 2)
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }

    SuccessOrExit(error = ParseTopic(aArgs[1], &topic));
    SuccessOrExit(error = otMqttsnUnsubscribe(mInterpreter.mInstance, &topic, &Mqtt::HandleUnsubscribed, this));
exit:
    return error;
}

otError Mqtt::ProcessDisconnect(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    return otMqttsnDisconnect(mInterpreter.mInstance);
}

otError Mqtt::ProcessSleep(uint8_t aArgsLength, char *aArgs[])
{
    otError error;
    uint16_t duration;

    if (aArgsLength != 2)
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = ParseAsUint16(aArgs[1], duration));
    SuccessOrExit(error = otMqttsnSleep(mInterpreter.mInstance, duration));
exit:
    return error;
}

otError Mqtt::ProcessAwake(uint8_t aArgsLength, char *aArgs[])
{
    otError error;
    uint32_t timeout;

    if (aArgsLength != 2)
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = ParseAsUint32(aArgs[1], timeout));
    SuccessOrExit(error = otMqttsnAwake(mInterpreter.mInstance, timeout));
exit:
    return error;
}

otError Mqtt::ProcessSearchgw(uint8_t aArgsLength, char *aArgs[])
{
    otError error;
    otIp6Address multicastAddress;
    uint16_t port;
    uint8_t radius;

    if (aArgsLength != 4)
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }
    SuccessOrExit(error = otIp6AddressFromString(aArgs[1], &multicastAddress));
    SuccessOrExit(error = ParseAsUint16(aArgs[2], port));
    SuccessOrExit(error = ParseAsUint8(aArgs[3], radius));
    SuccessOrExit(error = otMqttsnSetSearchgwHandler(mInterpreter.mInstance, &Mqtt::HandleSearchgwResponse, this));
    SuccessOrExit(error = otMqttsnSearchGateway(mInterpreter.mInstance, &multicastAddress, port, radius));
exit:
    return error;
}

otError Mqtt::ProcessGateways(uint8_t aArgsLength, char *aArgs[])
{
    OT_UNUSED_VARIABLE(aArgsLength);
    OT_UNUSED_VARIABLE(aArgs);

    otMqttsnGatewayInfo gateways[kMaxGatewayInfoCount];
    uint16_t gatewayCount;
    gatewayCount = otMqttsnGetActiveGateways(mInterpreter.mInstance, gateways, sizeof(gateways));
    for (uint16_t i = 0; i < gatewayCount; i++)
    {
        otMqttsnGatewayInfo &info = gateways[i];
        mInterpreter.OutputFormat("gateway ");
        mInterpreter.OutputIp6Address(*static_cast<Ip6::Address *>(&info.mGatewayAddress));
        mInterpreter.OutputFormat(": gateway_id=%d\r\n", (uint32_t)info.mGatewayId);
    }
    return OT_ERROR_NONE;
}

void Mqtt::HandleConnected(otMqttsnReturnCode aCode, void *aContext)
{
	static_cast<Mqtt *>(aContext)->HandleConnected(aCode);
}

void Mqtt::HandleConnected(otMqttsnReturnCode aCode)
{
	if (aCode == kCodeAccepted)
	{
		mInterpreter.OutputFormat("connected\r\n");
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
        mInterpreter.OutputFormat("subscribed topic id:");
        if (aTopic != NULL)
        {
            mInterpreter.OutputFormat("%u\r\n", otMqttsnGetTopicId(aTopic));
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
        mInterpreter.OutputFormat("registered topic id:%u\r\n", otMqttsnGetTopicId(aTopic));
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
        mInterpreter.OutputFormat("published\r\n");
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
        mInterpreter.OutputFormat("unsubscribed\r\n");
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
        mInterpreter.OutputFormat("received publish from topic id %u:\r\n", otMqttsnGetTopicId(aTopic));
    }
    else if (aTopic->mType == kShortTopicName)
    {
        mInterpreter.OutputFormat("received publish from topic %s:\r\n", otMqttsnGetTopicName(aTopic));
    }
    mInterpreter.OutputFormat("%.*s\r\n", aPayloadLength, aPayload);
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
        mInterpreter.OutputFormat("disconnected reason: %s\r\n", disconnectTypeText);
    }
    else
    {
        mInterpreter.OutputFormat("disconnected with unknown reason: %d\r\n", aType);
    }
}

void Mqtt::HandleSearchgwResponse(const otIp6Address* aAddress, uint8_t aGatewayId, void* aContext)
{
    static_cast<Mqtt *>(aContext)->HandleSearchgwResponse(aAddress, aGatewayId);
}

void Mqtt::HandleSearchgwResponse(const otIp6Address* aAddress, uint8_t aGatewayId)
{
    mInterpreter.OutputFormat("searchgw response from ");
    mInterpreter.OutputIp6Address(*static_cast<const Ip6::Address *>(aAddress));
    mInterpreter.OutputFormat(": gateway_id=%u\r\n", (unsigned int)aGatewayId);
}

void Mqtt::PrintFailedWithCode(const char *aCommandName, otMqttsnReturnCode aCode)
{
    const char* codeText;
    if (otMqttsnReturnCodeToString(aCode, &codeText) == OT_ERROR_NONE)
    {
        mInterpreter.OutputFormat("%s failed: %s\r\n", aCommandName, codeText);
    }
    else
    {
        mInterpreter.OutputFormat("%s failed with unknown code: %d\r\n", aCommandName, aCode);
    }
}

} // namespace Cli
} // namespace ot

#endif
