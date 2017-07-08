E131 - E1.31 (sACN) library for Particle
=======================================
This library is to simplify the validation and handling of E1.31 sACN (DMX over Ethernet) traffic.  It supports both Unicast and Multicast configurations and exposes the full E1.31 packet to the user.  Currently, development is targeted for the The electron from particle.io.  

### Supported Hardware
- Particle Photon

### API / Usage
#### Notes

#### Initializers
These are to initialize the network stack and should be in the ```setup()``` function of your sketch.

  ```void begin()``` Starts with with only 1 universe at number 1
  
  ```void begin(uint16_t universes)``` specifies how many universes to monitor (Not currently finished)

#### Loop Handlers
These are non-blocking handlers to be used in the ```loop()``` function of your sketch.
- ```int parsePacket()```: Checks and parses new packets, returns number of DMX Channels in packet as ```uint16_t```

#### Exposed Data Structures
- ```byte *data```: Pointer to DMX channel data from packet.  Always valid if double-buffering is enabled (see notes above)
- ```uint16_t universe```: DMX Universe of last valid packet
- ```e131_stats_t stats```: E1.31 Statistics

### Resources:
- Based on code From: http://github.com/forkineye/E131
