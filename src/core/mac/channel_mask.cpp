/*
 *  Copyright (c) 2016-2018, The OpenThread Authors.
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

/**
 * @file
 *   This file implements MAC Channel Mask.
 */

#include "channel_mask.hpp"

#include "common/code_utils.hpp"
#include "common/random.hpp"

namespace ot {
namespace Mac {

uint8_t ChannelMask::GetNumberOfChannels(void) const
{
    uint8_t num     = 0;
    uint8_t channel = kChannelIteratorFirst;

    while (GetNextChannel(channel) == OT_ERROR_NONE)
    {
        num++;
    }

    return num;
}

otError ChannelMask::GetNextChannel(uint8_t &aChannel) const
{
    otError error = OT_ERROR_NOT_FOUND;

    if (aChannel == kChannelIteratorFirst)
    {
        aChannel = (Radio::kChannelMin - 1);
    }

    for (aChannel++; aChannel <= Radio::kChannelMax; aChannel++)
    {
        if (ContainsChannel(aChannel))
        {
            ExitNow(error = OT_ERROR_NONE);
        }
    }

exit:
    return error;
}

uint8_t ChannelMask::ChooseRandomChannel(void) const
{
    uint8_t channel = kChannelIteratorFirst;
    uint8_t randomIndex;

    VerifyOrExit(!IsEmpty());

    randomIndex = Random::NonCrypto::GetUint8InRange(0, GetNumberOfChannels());

    SuccessOrExit(GetNextChannel(channel));

    while (randomIndex-- != 0)
    {
        SuccessOrExit(GetNextChannel(channel));
    }

exit:
    return channel;
}

ChannelMask::InfoString ChannelMask::ToString(void) const
{
    InfoString string;
    uint8_t    channel  = kChannelIteratorFirst;
    bool       addComma = false;
    otError    error;

    IgnoreError(string.Append("{"));

    error = GetNextChannel(channel);

    while (error == OT_ERROR_NONE)
    {
        uint8_t rangeStart = channel;
        uint8_t rangeEnd   = channel;

        while ((error = GetNextChannel(channel)) == OT_ERROR_NONE)
        {
            if (channel != rangeEnd + 1)
            {
                break;
            }

            rangeEnd = channel;
        }

        IgnoreError(string.Append("%s%d", addComma ? ", " : " ", rangeStart));
        addComma = true;

        if (rangeStart < rangeEnd)
        {
            IgnoreError(string.Append("%s%d", rangeEnd == rangeStart + 1 ? ", " : "-", rangeEnd));
        }
    }

    IgnoreError(string.Append("}"));

    return string;
}

} // namespace Mac
} // namespace ot
