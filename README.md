# OpenThread with MQTT-SN client support

* [OpenThread with MQTT-SN client support](#OpenThread-with-MQTT-SN-client-support)<br>
  * [Overview](#Overview)<br>
  * [About OpenThread](#About-OpenThread)<br>
  * [About MQTT-SN](#About-MQTT-SN)<br>
* [Trying MQTT-SN client with CLI application example](#Trying-MQTT-SN-client-with-CLI-application-example)<br>
  * [Requirements](#Requirements)<br>
  * [Build and flash RCP](#Build-and-flash-RCP)<br>
  * [Install border router with MQTT-SN gateway](#Install-border-router-with-MQTT-SN-gateway)
  * [Build CLI example](#Build-CLI-example)
  * [Connect to the broker](#Connect-to-the-broker)
  * [Subscribe the topic](#Subscribe-the-topic)
  * [Publish a message](#Publish-a-message)
* [MQTT-SN source files](#MQTT-SN-source-files)
* [Code samples](#Code-samples)

### Overview

This project contains fork of OpenThread SDK which implements MQTT-SN protocol on Thread network. MQTT-SN implementation is part of OpenThread library build. MQTT-SN implementation allows user to send MQTT messages from Thread network to regular MQTT broker in external IP network. This is not official OpenThread project. OpenThread code may be outdated and there can be some bugs or missing features. If you want to use latest OpenThread project please go to [official repository](https://github.com/openthread/openthread).

* MQTT-SN over Thread network uses UDP as transport layer. UDP MQTT-SN packets are tranformed and forwarded to MQTT broker by [Eclipse Paho MQTT-SN Gateway](https://github.com/eclipse/paho.mqtt-sn.embedded-c/tree/master/MQTTSNGateway).

* There is introduced [C API](include/openthread/mqttsn.h) for custom applications and [CLI commands](src/cli/README_MQTT.md) which can be used for basic client functions evaluation.

**Provided MQTT-SN client implements most important features specified by protocol MQTT-SN v1.2:**

* Publish and subscribe with QoS level 0, 1, 2
* Easy publish with QoS level -1 without maintaining connection
* Periodic keepalive requests
* Multicast gateway search and advertising
* Sleep mode for sleeping devices

### About OpenThread
[ot-gh-action-simulation]: https://github.com/openthread/openthread/actions?query=workflow%3ASimulation+branch%3Amaster+event%3Apush
[ot-gh-action-simulation-svg]: https://github.com/openthread/openthread/workflows/Simulation/badge.svg?branch=master&event=push
[ot-lgtm]: https://lgtm.com/projects/g/openthread/openthread/context:cpp
[ot-lgtm-svg]: https://img.shields.io/lgtm/grade/cpp/g/openthread/openthread.svg?logo=lgtm&logoWidth=18

OpenThread released by Google is an open-source implementation of the Thread networking protocol. Google Nest has released OpenThread to make the technology used in Nest products more broadly available to developers to accelerate the development of products for the connected home.

<a href="https://www.amazon.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-amazon.png" alt="Amazon" width="200px"></a><a href="https://www.arm.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-arm.png" alt="ARM" width="200px"></a><a href="https://www.cascoda.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-cascoda.png" alt="Cascoda" width="200px"></a><a href="https://www.eero.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-eero.png" alt="Eero" width="200px"></a><a href="https://www.espressif.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-espressif-github.png" alt="Espressif" width="200px"></a><a href="https://www.google.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-google.png" alt="Google" width="200px"></a><a href="https://www.infineon.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-infineon.png" alt="Infineon" width="200px"></a><a href="https://mmbnetworks.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-mmb-networks.png" alt="MMB Networks" width="200px"></a><a href="https://www.nabucasa.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-nabu-casa.png" alt="Nabu Casa" width="200px"></a><a href="https://www.nanoleaf.me/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-nanoleaf.png" alt="Nanoleaf" width="200px"></a><a href="http://www.nordicsemi.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-nordic.png" alt="Nordic" width="200px"></a><a href="http://www.nxp.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-nxp.png" alt="NXP" width="200px"></a><a href="http://www.qorvo.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-qorvo.png" alt="Qorvo" width="200px"></a><a href="https://www.qualcomm.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-qc.png" alt="Qualcomm" width="200px"></a><a href="https://www.samsung.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-samsung.png" alt="Samsung" width="200px"></a><a href="https://www.silabs.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-silabs.png" alt="Silicon Labs" width="200px"></a><a href="https://www.st.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-stm.png" alt="STMicroelectronics" width="200px"></a><a href="https://www.synopsys.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-synopsys.png" alt="Synopsys" width="200px"></a><a href="https://www.telink-semi.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-telink-github.png" alt="Telink Semiconductor" width="200px"></a><a href="https://www.ti.com/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-ti.png" alt="Texas Instruments" width="200px"></a><a href="https://www.zephyrproject.org/"><img src="https://github.com/openthread/openthread/raw/main/doc/images/ot-contrib-zephyr.png" alt="Zephyr Project" width="200px"></a>
### About MQTT-SN

[MQTT-SN v1.2](https://mqtt.org/tag/mqtt-sn) is formerly known as MQTT-S. MQTT for Sensor Networks is aimed at embedded devices on non-TCP/IP networks, such as Zigbee. MQTT-SN is a publish/subscribe messaging protocol for wireless sensor networks (WSN), with the aim of extending the MQTT protocol beyond the reach of TCP/IP infrastructure for Sensor and Actuator solutions. More informations can be found in [MQTT-SN specification](http://www.mqtt.org/new/wp-content/uploads/2009/06/MQTT-SN_spec_v1.2.pdf).

## Trying MQTT-SN client with CLI application example

### Requirements

* Linux device such as [Raspberry Pi](https://www.raspberrypi.org/) acting as border router with installed Docker - [Docker install guide](https://docs.docker.com/v17.09/engine/installation/), [guide for Raspberry](https://www.raspberrypi.org/blog/docker-comes-to-raspberry-pi/)
* 2 Thread devices compatible with OpenThread - check [here](https://openthread.io/platforms)
* Environment for building firmware with [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)

### Build and flash RCP

OpenThread Border Router runs on an RCP design. Select a supported OpenThread platform to use as an RCP and follow the 
[official building instructions](https://openthread.io/guides/border-router/build). For example for KW41Z platform run
following commands:

```
git clone https://github.com/openthread/ot-kw41z.git --recursive
cd ot-kw41z
./script/bootstrap
./script/build -DOT_BORDER_AGENT=ON -DOT_BORDER_ROUTER=ON -DOT_COMMISSIONER=ON -DOT_UDP_FORWARD=ON
```

After a successful build, the elf files are found in output/kw41z/bin. You can convert them to bin files using arm-none-eabi-objcopy:

```
cd /output/kw41z/bin
arm-none-eabi-objcopy -O binary ot-rcp ot-rcp.bin
```

Then flash the binary and connect RCP device to border router device.

### Install border router with MQTT-SN gateway

Border router device provides functionality for routing and  forwarding communication from Thread subnet to other IP networks. In this example border router consists of [OpenThread Border Router](https://openthread.io/guides/border-router) (OTBR) which is capable of routing communication with Thread network and [Eclipse Paho MQTT-SN Gateway](https://github.com/eclipse/paho.mqtt-sn.embedded-c/tree/master/MQTTSNGateway) which transforms MQTT-SN UDP packets to MQTT.

All applications for border router are provided as Docker images. First step is to install docker on your platform ([Docker install guide](https://docs.docker.com/v17.09/engine/installation/)). On Raspberry Pi platform just run following script:

```
curl -sSL https://get.docker.com | sh
```

In real application it is best practice to use DNS server for resolving host IP address. In this example are used simple static addresses for locating services. Create custom docker network `test` for this purpose:

```
sudo docker network create --subnet=172.18.0.0/16 test
```

Run new OTBR container from official image:

```
sudo docker run -d --name otbr --sysctl "net.ipv6.conf.all.disable_ipv6=0 \
        net.ipv4.conf.all.forwarding=1 net.ipv6.conf.all.forwarding=1" -p 8080:80 \
        --dns=127.0.0.1 -v /dev/ttyACM0:/dev/ttyACM0 --net test --ip 172.18.0.6 \
        --privileged openthread/otbr --radio-url spinel+hdlc+uart:///dev/ttyACM0"
```

Container will use `test` network with static IP address 172.18.0.6. If needed replace `/dev/ttyACM0` in `-v` and `--radio-url` parameter with name under which appear RCP device in your system (`/dev/ttyS0`, `/dev/ttyUSB0` etc.).

In latest OTBR image it may be necessary to manually activate NAT64 feature by running following command in running container (after the network is setup and up).

```
# ot-ctl nat64 enable
Done
```

You can check if NAT64 is enabled:

```
# ot-ctl nat64 state
PrefixManager: Active
Translator: Active
Done
```

 NAT-64 prefix is set automatically by OpenThread. It allows address translation and routing to local addresses. Border Router web GUI is bound to port 8080.

Next step is to run Mosquitto container as MQTT broker for sample test. Broker IP address in `test` network will be 172.18.0.7:

```
sudo docker run -d --name mosquitto --net test --ip 172.18.0.7 kyberpunk/mosquitto
```

As last step run container with MQTT-SN Gateway:

```
sudo docker run -d --name paho --net test --ip 172.18.0.8 kyberpunk/paho \
        --broker-name 172.18.0.7 --broker-port 1883
```

MQTT-SN gateway service address is 172.18.0.8 which can be translated to IPv6 as 2018:ff9b::ac12:8. See more information [here](https://openthread.io/guides/thread-primer/ipv6-addressing).

**IMPORTANT NOTICE: In this network configuration MQTT-SN network does not support SEARCHGW and ADVERTISE messages in Thread network until you configure multicast forwarding.** Alternativelly you can use UDPv6 version of gateway ([kyberpunk/paho6](https://hub.docker.com/repository/docker/kyberpunk/paho6) image) and attach it to OTBR container interface wpan0 (`--net "container:otbr"`).

### Build CLI example

Build the CLI example firmware accordingly to your [platform](https://openthread.io/platforms). Original openthread submodule **must** be replaced by [kyberpunk/openthread](https://github.com/kyberpunk/openthread). For example for KW41Z platform run:

```
git clone https://github.com/openthread/ot-kw41z.git
cd ot-kw41z
git submodule remove openthread
rm -rf openthread
echo "" > .gitmodules
git submodule add https://github.com/kyberpunk/openthread.git openthread
git submodule update --init --recursive
./script/bootstrap
./script/build -DOT_MQTT=ON -DOT_JOINER=ON
```

Convert fimware to binary and flash your device with `ot-cli-ftd.bin`.

```
cd /output/kw41z/bin
arm-none-eabi-objcopy -O binary ot-cli-ftd ot-cli-ftd.bin
```

### Connect to the broker

Firs of all the CLI device must be commisioned into the Thread network. Follow the the [OTBR commissioning guide](https://openthread.io/guides/border-router/external-commissioning). When device joined the Thread network you can start MQTT-SN service and connect to gateway which is reachable on NAT-64 translated IPv6 address e.g. 2018:ff9b::ac12:8.

```
> mqtt start
Done
> mqtt connect 2018:ff9b::ac12:8 10000
Done
connected
```

You will see `connected` message when client successfully connected to the gateway. Client stays connected and periodically sends keepalive messages. See more information about `connect` command in [CLI reference](src/cli/README_MQTT.md#connect).

You can also see log of messages forwarded by MQTT-SN gateway:

```
docker logs paho
```

CLI also supports automatic IPv4 address translation using preferred NAT64 prefix. You can then use MQTT-SN gateway IPv4 address directly in command:

```
> mqtt connect 172.18.0.8 10000
Done
connected
```

### Subscribe the topic

After successful connection can CLI device subscribe to MQTT topic with `subscribe` command.

```bash
> mqtt subscribe sensors
Done
subscribed topic id: 1
```

You can test subscription by sending test message with `mosquitto_pub` from mosquitto Docker container.

```bash
$ sudo docker exec -it mosquitto mosquitto_pub -h 127.0.0.1 -t sensors -m "{\"temperature\":24.0}"
```

Following output should appean on CLI device:

```bash
received publish from topic id 1:
{"temperature":24.0}
```

See more information about `subscribe` command in [CLI reference](src/cli/README_MQTT.md#subscribe).

### Publish a message

Publish a message with publish `command`. To determine the topic id `register` command should be used:

```bash
> mqtt register sensors
registered topic id:1
> mqtt publish 1 {"temperature":24.0}
Done
published
```

`published` message should be written to CLI output when publish succeeded. You can test reception of PUBLISH message with `mosquitto_sub` command from mosquitto Docker container.

```bash
$ sudo docker exec -it mosquitto mosquitto_sub -h 127.0.0.1 -t sensors
{"temperature":24.0}
```

See more information about `publish` command in [CLI reference](src/cli/README_MQTT.md#publish).

## MQTT-SN source files
[third_party/paho](third_party/paho)<br>
[src/core/mqttsn/mqttsn_client.cpp](src/core/mqttsn/mqttsn_client.cpp)<br>
[src/core/mqttsn/mqttsn_client.hpp](src/core/mqttsn/mqttsn_client.hpp)<br>
[src/core/mqttsn/mqttsn_gateway_list.cpp](src/core/mqttsn/mqttsn_gateway_list.cpp)<br>
[src/core/mqttsn/mqttsn_gateway_list.hpp](src/core/mqttsn/mqttsn_gateway_list.hpp)<br>
[src/core/mqttsn/mqttsn_serializer.cpp](src/core/mqttsn/mqttsn_serializer.cpp)<br>
[src/core/mqttsn/mqttsn_serializer.hpp](src/core/mqttsn/mqttsn_serializer.hpp)<br>
[include/openthread/mqttsn.h](include/openthread/mqttsn.h)<br>
[src/core/api/mqttsn_api.cpp](src/core/api/mqttsn_api.cpp)<br>
[src/cli/cli_mqtt.cpp](src/cli/cli_mqtt.cpp)<br>
[src/cli/cli_mqtt.hpp](src/cli/cli_mqtt.hpp)<br>
[tests/unit/test_mqttsn.cpp](tests/unit/test_mqttsn.cpp)

## Code samples

You can find code samples how to use MQTT-SN client with OpenThread in separate repository [here](https://github.com/kyberpunk/openthread-mqttsn/).
