# OpenThread CLI - MQTT-SN Example

This OpenThread fork contains MQTT-SN protocol support. The MQTT client API may be invoked via the OpenThread CLI. For more details about the operations see [the specification](http://mqtt.org/documentation).

## Command Details

### help

Usage: `mqtt help`

Print dataset help menu.

```bash
> mqtt help
help
start
stop
connect
reconnect
subscribe
state
register
publish
publishm1
unsubscribe
disconnect
sleep
awake
searchgw
Done
```

### start

Usage: `mqtt start [port]`

Start the MQTT-SN client. Must be called before performing other operations.

* port: UDP port number used by MQTT-SN client. Default value is 10000 (optional).

```bash
> mqtt start
Done
```

### stop

Usage: `mqtt stop`

Stop the MQTT-SN client and free allocated resources.

```bash
> mqtt stop
Done
```

### connect

Usage: `mqtt connect <gateway-ip> <gateway-port>`

Connect to MQTT-SN gateway. Client is connected with default connection settings. Default keepalive time is 30 seconds, message timeout threshold 10 seconds and failed messages retransmission count is 3 times.

* gateway-ip: IPv6 address of MQTT-SN gateway.
* gateway-port: Port number of MQTT-SN service.

```bash
> mqtt connect 2018:ff9b::ac12:8 10000
Done
connected
```

### reconnect

Usage: `mqtt reconnect`

Reconnect MQTT-SN client with current connection settings. This is method is useful e.g. for returning from sleep mode to active mode.

```bash
> mqtt reconnect
Done
connected
```

### subscribe

Usage: `mqtt subscribe <topic> [qos]`

Subscribe to the topic.

* topic: String name of the topic to subscribe or predefined topic ID with `$` prefix. 
* qos: Requested quality of service level for the topic. Possible values are 0, 1, 2, -1. Default value is 1 (optional).

Specific topic ID is returned after response. This ID is used for publising to the topic.

Subscribe topic with topic name "sensors":

```bash
> mqtt subscribe sensors
Done
subscribed topic id: 1
```

Subscribe topic with predefined topic ID 1:

```bash
> mqtt subscribe $1
Done
subscribed topic id: 1
```

### state

Usage: `mqtt state`

Get current state of MQTT-SN client.

```bash
> mqtt state
Disconnected
Done
```

### register

Usage: `mqtt register <topic>`

Register the topic.

* topic: Name of the topic to be registered.

Specific topic ID is returned after response. This ID is used for publising to the topic.

```bash
> mqtt register sensors
Done
registered topic id: 1
```

### publish

Usage: `mqtt publish <topic> <qos> [payload]`

Publish data to the topic. Data are encoded as string and white spaces are currently not supported (separated words are recognized as additional parameters).

* topic: Topic to publish to. Use `@` prefix for topic ID returned by [register](#register). Use `$` prefix for predefined topic ID.
* qos: Quality of service of publish. Possible values are 0, 1, or 2.
* payload: Text to be send in publish message payload. If empty, then empty data are send (optional).

Publish to topic ID 1 with QoS 0:

```bash
> mqtt publish @1 0 {"temperature":24.0}
Done
published
```

Publish to predefined topic ID 1 with QoS 1:

```bash
> mqtt publish $1 1 {"temperature":24.0}
Done
published
```

### publishm1

Usage: `mqtt publishm1 <gateway-ip> <gateway-port> <topic> [payload]`

Publish data to the topic with QoS -1 without need of connection. Data are encoded as string and white spaces are currently not supported (separated words are recognized as additional parameters).

* gateway-ip: IPv6 address of MQTT-SN gateway.
* gateway-port: Port number of MQTT-SN service.
* topic: Topic to publish to. Use `$` prefix for predefined topic ID. Only predefined topic is allowed.
* payload: Text to be send in publish message payload. If empty, then empty data are send (optional).

Publish to predefined topic ID 1:

```bash
> mqtt publish 2018:ff9b::ac12:8 10000 $1 {"temperature":24.0}
Done
published
```

### unsubscribe

Usage: `mqtt unsubscribe <topic>`

Unsubscribe from the topic.

* topic: String name of the topic to subscribe or predefined topic ID with `$` prefix. 

Subscribe topic with topic name "sensors":

```bash
> mqtt unsubscribe sensors
Done
unsubscribed
```

Subscribe topic with predefined topic ID 1:

```bash
> mqtt unsubscribe $1
Done
unsubscribed
```

### disconnect

Usage: `mqtt disconnect`

Disconnect from the gateway.

```bash
> mqtt disconnect
Done
disconnected reason: Client
```

### sleep

Usage: `mqtt sleep <duration>`

Take client to the sleeping state.

* duration: Sleep duration time in s.

```bash
> mqtt sleep 60
Done
disconnected reason: Asleep
```

### awake

Usage: `mqtt awake <timeout>`

Take client to the awake state from the sleeping state. In this state device can receive buffered messages from the gateway.

* timeout: Maximal timeout in ms for which is the client awake before going asleep. After expiration the connection is lost. Default retransmission count is still applied.

```bash
> mqtt awake 1000
Done
connected
disconnected reason: Asleep
```

### searchgw

Usage: `mqtt searchgw <multicast-ip> <port> <radius>`

Send searchgw multicast message. If there is any MQTT-SN gateway listening in the same network it will answer with its connection details.

* multicast-ip: Multicast IPv6 address for spread the searchgw message.
* port: Destination port of MQTT-SN gateway.
* radius: Hop limit for multicast message.

```bash
> mqtt searchgw ff03::1 10000 5
Done
searchgw response from 2018:ff9b:0:0:0:0:ac12:8: gateway_id=1
```

If gateway is in the network behind border router forwarding for broadcast must be set.

### gateways

Get of active MQTT-SN gateways cached in the client. Gateways are periodically advertised or obtained with gwinfo message.

```bash
> mqtt gateways
gateway fd11:22:0:0:6648:fb52:b05:43f5: gateway_id=1
Done
```
