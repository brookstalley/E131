E131 - E1.31 (sACN) library for Particle
=======================================
This library is to simplify the validation and handling of E1.31 sACN (DMX over Ethernet) traffic.  It supports both Unicast and Multicast configurations and exposes the full E1.31 packet to the user.  Currently, development is targeted for the The electron from particle.io.  

### Supported Hardware
- Electron

### API / Usage
#### Notes

#### Initializers
These are to initialize the network stack and should be in the ```setup()``` function of your sketch.

##### Generic UDP only Initializer
This initializer only sets up the UDP listener.  It is up to the user to setup their network connection.  The initializers below this one will setup the network connection for you.
- ```void begin(e131_listen_t type, uint16_t universe = 1)```: valid types are ```E131_UNICAST``` and ```E131_MULTICAST```.  ```universe``` is optional and only used for Multicast configuration.

##### Unicast WiFi Initializers
- ```int begin(const char *ssid, const char *passphrase)```: returns ```WiFi.status()```
- ```int begin(const char *ssid, const char *passphrase, IPAddress ip, IPAddress netmask, IPAddress gateway, IPAddress dns)```: returns ```WiFi.status()```

##### Multicast WiFi Initializers (ESP8266 Only)
- ```int beginMulticast(const char *ssid, const char *passphrase, uint16_t universe)```: returns ```WiFi.status()```
- ```int beginMulticast(const char *ssid, const char *passphrase, uint16_t universe, IPAddress ip, IPAddress netmask, IPAddress gateway, IPAddress dns)```: returns ```WiFi.status()```

##### Unicast Ethernet Initializers
- ```int begin(uint8_t *mac)```: DHCP initializer. Returns 0 if DHCP failed, 1 for success
- ```void begin(uint8_t *mac, IPAddress ip, IPAddress netmask, IPAddress gateway, IPAddress dns)```: Static initializer

#### Loop Handlers
These are non-blocking handlers to be used in the ```loop()``` function of your sketch.
- ```int parsePacket()```: Checks and parses new packets, returns number of DMX Channels in packet as ```uint16_t```

#### Exposed Data Structures
- ```byte *data```: Pointer to DMX channel data from packet.  Always valid if double-buffering is enabled (see notes above)
- ```uint16_t universe```: DMX Universe of last valid packet
- ```e131_packet_t *packet```: Pointer to last packet. Always valid if double-buffering is enabled (see notes above)
- ```e131_stats_t stats```: E1.31 Statistics

### Resources:
- Latest code: http://github.com/forkineye/E131
- Other Stuff: http://forkineye.com
