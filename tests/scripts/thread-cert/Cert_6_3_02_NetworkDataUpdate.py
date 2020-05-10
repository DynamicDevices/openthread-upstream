#!/usr/bin/env python3
#
#  Copyright (c) 2016, The OpenThread Authors.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of the copyright holder nor the
#     names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#

import unittest

import thread_cert

LEADER = 1
ED = 2


class Cert_6_3_2_NetworkDataUpdate(thread_cert.TestCase):
    topology = {
        LEADER: {
            'mode': 'rsdn',
            'panid': 0xface,
            'whitelist': [ED]
        },
        ED: {
            'is_mtd': True,
            'mode': 'rsn',
            'panid': 0xface,
            'timeout': 10,
            'whitelist': [LEADER]
        },
    }

    def test(self):
        self.nodes[LEADER].start()
        self.simulator.go(5)
        self.assertEqual(self.nodes[LEADER].get_state(), 'leader')

        self.nodes[ED].start()
        self.simulator.go(5)
        self.assertEqual(self.nodes[ED].get_state(), 'child')

        self.nodes[LEADER].add_prefix('2001:2:0:1::/64', 'paros')
        self.nodes[LEADER].register_netdata()

        # Set lowpan context of sniffer
        self.simulator.set_lowpan_context(1, '2001:2:0:1::/64')

        self.simulator.go(5)

        addrs = self.nodes[ED].get_addrs()
        self.assertTrue(any('2001:2:0:1' in addr[0:10] for addr in addrs))
        for addr in addrs:
            if addr[0:10] == '2001:2:0:1':
                self.assertTrue(self.nodes[LEADER].ping(addr))

        self.nodes[LEADER].remove_whitelist(self.nodes[ED].get_addr64())
        self.nodes[ED].remove_whitelist(self.nodes[LEADER].get_addr64())

        self.nodes[LEADER].add_prefix('2001:2:0:2::/64', 'paros')
        self.nodes[LEADER].register_netdata()

        # Set lowpan context of sniffer
        self.simulator.set_lowpan_context(2, '2001:2:0:2::/64')

        self.simulator.go(5)

        self.nodes[LEADER].add_whitelist(self.nodes[ED].get_addr64())
        self.nodes[ED].add_whitelist(self.nodes[LEADER].get_addr64())
        self.simulator.go(10)

        addrs = self.nodes[ED].get_addrs()
        self.assertTrue(any('2001:2:0:1' in addr[0:10] for addr in addrs))
        self.assertTrue(any('2001:2:0:2' in addr[0:10] for addr in addrs))
        for addr in addrs:
            if addr[0:10] == '2001:2:0:1' or addr[0:10] == '2001:2:0:2':
                self.assertTrue(self.nodes[LEADER].ping(addr))


if __name__ == '__main__':
    unittest.main()
