# Arduino-ZigBee-Pi-ThingSpeak
An interface to attempt to simplify IoT ThingSpeak networks.

## Network
The intended network topology for this code is:

> Arduino - XBee (AT Router/End Point)  \
>
> Arduino - XBee (AT Router/End Point) --  XBee (API Coordinator) - Raspberry Pi - ThingSpeak
>
> Arduino - XBee (AT Router/End Point)  /

### Arduino
The Arduino endpoints are set up as data loggers.
See [GenericDataLogger](./GenericDataLogger) for info.
The code is tested using Arduino Uno, but should be applicable to other boards.

For testing, the [Sparkfun Weather Shield](https://www.sparkfun.com/products/12081) was used.

### XBee Configuration
The LAN is a ZigBee mesh, based on the recommendations of [Gadgeteer@HCI-Lab](http://blog.hcilab.org/gadgeteer/2012/12/xbee-zigbee-howto/).
Data points are set up in AT mode for simple transmission, whereas the coordinator is set up in API mode to enable message tracking.
The abridged setup process is as follows:

1. Set up coordinator with API firmware.
1. Reset coordinator settings to default.
1. Set the following settings:
> Set PAN        (ID) to desired network ID (common to all)
> Set API Enable (AP) to 2
> Set name       (NI) to desired name (unique)
1. Set up other modules as routers or end points.
1. Reset these module's setting to default.
1. Set the following settings:
> Set PAN        (ID) to desired network ID (same as coordinator)
> Set Dest. high (DH) to 0
> Set Dest. low  (DL) to 0
> Set Chan. ver. (JV) to 1
> Set name       (NI) to desired name (unique)

XBee Pro S2B modules were used for testing, but the code and process should be applicable to other ZigBee enabled XBee modules.

### Raspberry Pi
*Under Development*

### ThingSpeak
*Under Development*

# Software Dependencies
There are a number of dependencies for each part of the software.

## Arduino
There are no direct dependencies for the library, however the example file uses the following:

- [MemoryFree](https://github.com/maniacbug/MemoryFree): For debugging purposes.
- [HTU21D](https://github.com/sparkfun/HTU21D_Breakout): Sparkfun weather shield humidity sensor.
- [MPL3115A2](https://github.com/sparkfun/MPL3115A2_Breakout): Sparufun weather shield pressure sensor.
