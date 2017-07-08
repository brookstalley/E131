#include "Particle.h"
SYSTEM_THREAD(ENABLED); // This makes the system cloud connection run on a background thread so as to not delay our timing

/*
* E131.cpp
*
* Project: E131 - E.131 (sACN) library for Electron
* Copyright (c) 2017 AdmiralTriggerHappy
* Dervived from work by:
* Copyright (c) 2015 Shelby Merrick
* http://www.forkineye.com
 *
 *  This program is provided free for you to use in any way that you wish,
 *  subject to the laws and regulations where you are using it.  Due diligence
 *  is strongly suggested before using this code.  Please give credit where due.
 *
 *  The Author makes no warranty of any kind, express or implied, with regard
 *  to this program or the documentation contained in this document.  The
 *  Author shall not be liable in any event for incidental or consequential
 *  damages in connection with, or arising out of, the furnishing, performance
 *  or use of these programs.
 *
 */

// Helpers
#define htons(x) ( ((x)<<8) | (((x)>>8)&0xFF) )
#define ntohs(x) htons(x)

#define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
((x)<< 8 & 0x00FF0000UL) | \
((x)>> 8 & 0x0000FF00UL) | \
((x)>>24 & 0x000000FFUL) )
#define ntohl(x) htonl(x)

/* Defaults */
#define E131_DEFAULT_PORT 5568
#define WIFI_CONNECT_TIMEOUT 10000  /* 10 seconds */

/* E1.31 Packet Offsets */
#define E131_ROOT_PREAMBLE_SIZE 0
#define E131_ROOT_POSTAMBLE_SIZE 2
#define E131_ROOT_ID 4
#define E131_ROOT_FLENGTH 16
#define E131_ROOT_VECTOR 18
#define E131_ROOT_CID 22

#define E131_FRAME_FLENGTH 38
#define E131_FRAME_VECTOR 40
#define E131_FRAME_SOURCE 44
#define E131_FRAME_PRIORITY 108
#define E131_FRAME_RESERVED 109
#define E131_FRAME_SEQ 111
#define E131_FRAME_OPT 112
#define E131_FRAME_UNIVERSE 113

#define E131_DMP_FLENGTH 115
#define E131_DMP_VECTOR 117
#define E131_DMP_TYPE 118
#define E131_DMP_ADDR_FIRST 119
#define E131_DMP_ADDR_INC 121
#define E131_DMP_COUNT 123
#define E131_DMP_DATA 125

#define E131_PACKET_SIZE 638

/* E1.31 Packet Structure */
typedef union
{
    struct
    {
        /* Root Layer */
        uint16_t preamble_size;
        uint16_t postamble_size;
        char	 acn_id[12];
        uint16_t root_flength;
        uint32_t root_vector;
        uint8_t  cid[16];

        /* Frame Layer */
        uint16_t frame_flength;
        uint32_t frame_vector;
        uint8_t  source_name[64];
        uint8_t  priority;
        uint16_t reserved;
        uint8_t  sequence_number;
        uint8_t  options;
        uint16_t universe;

        /* DMP Layer */
        uint16_t dmp_flength;
        uint8_t  dmp_vector;
        uint8_t  type;
        uint16_t first_address;
        uint16_t address_increment;
        uint16_t property_value_count;
        uint8_t unknownValue;
        uint8_t  property_values[513];
    } __attribute__((packed));

    uint8_t raw[E131_PACKET_SIZE];
} e131_packet_t;

/* Status structure */
typedef struct
{
    uint32_t    num_packets;
    uint32_t    sequence_errors;
    uint32_t    packet_errors;
} e131_stats_t;

/* Error Types */
typedef enum
{
    ERROR_NONE,
    ERROR_ACN_ID,
    ERROR_PACKET_SIZE,
    ERROR_VECTOR_ROOT,
    ERROR_VECTOR_FRAME,
    ERROR_VECTOR_DMP
} e131_error_t;

/* Constants for packet validation */
static const char ACN_ID[12] = { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 };
static const uint32_t VECTOR_ROOT = 4;
static const uint32_t VECTOR_FRAME = 2;
static const uint8_t VECTOR_DMP = 2;

uint8_t sequence; /* Sequence tracker */


const size_t UDP_BUFFER_SIZE = E131_PACKET_SIZE;
const int UDP_PORT = 5568;
int packetNumber = 1;
    IPAddress multicast(239,255,0,1);

UDP udp;
uint8_t udpBuffer[UDP_BUFFER_SIZE];
uint8_t *data; // Pointer to DMX channel data
e131_packet_t packetBuffer; // Packet buffer
e131_packet_t *packet; // Pointer to last valid packet
uint16_t universe; // DMX Universe of last valid packet
e131_stats_t  stats; // Statistics tracker

 //Test Code
    int blueLED = D0;
    int greenLED = D1;
    int redLED = D2;


void setup() {
	Serial.begin(115200);
	
	 //Test Code
    // Here's the pin configuration, same as last time
   pinMode(blueLED, OUTPUT);
   pinMode(redLED, OUTPUT);
   pinMode(greenLED, OUTPUT);

   // For good measure, let's also make sure both LEDs are off when we start:
   digitalWrite(blueLED, HIGH);
   digitalWrite(redLED, HIGH);
   digitalWrite(greenLED, HIGH);
   
   //End Test Code

    
    waitUntil(WiFi.ready);
	udp.begin(UDP_PORT);
	udp.joinMulticast(multicast);
	packet = &packetBuffer;

	Serial.printlnf("server=%s:%d", WiFi.localIP().toString().c_str(), UDP_PORT);
}

void loop() {

	parsePacket();
	redBright(data[0]);
       greenBright(data[1]);
       blueBright(data[2]);

}
 int redBright(int brightness) {
        brightness = 255 - brightness;
        analogWrite(redLED,brightness);
        return 1;
    }
    
    int greenBright(int bright) {
        bright = 255 - bright;
        analogWrite(greenLED,bright);
        return 1;
    }
    
    int blueBright(int bright) {
        bright = 255 - bright;
        analogWrite(blueLED,bright);
        return 1;
}

/* Packet validater */
e131_error_t validateE131Packet()
{
    if (memcmp(packet->acn_id, ACN_ID, sizeof(packet->acn_id)) !=0)
        return ERROR_ACN_ID;
    if (htonl(packet->root_vector) != VECTOR_ROOT)

        return ERROR_VECTOR_ROOT;
    if (htonl(packet->frame_vector) != VECTOR_FRAME)

        return ERROR_VECTOR_FRAME;
    if (packet->dmp_vector != VECTOR_DMP)

        return ERROR_VECTOR_DMP;
    return ERROR_NONE;
}

void dumpError(e131_error_t error)
{
    switch (error) {
        case ERROR_ACN_ID:
            Serial.print(F("INVALID PACKET ID: "));
            for (uint8_t i = 0; i < sizeof(ACN_ID); i++)
                Serial.print(packet->acn_id[i], HEX);
            Serial.println("");
            break;
        case ERROR_PACKET_SIZE:
            Serial.println(F("INVALID PACKET SIZE: "));
            break;
        case ERROR_VECTOR_ROOT:
            Serial.print(F("INVALID ROOT VECTOR: 0x"));
            Serial.println(htonl(packet->root_vector), HEX);
            break;
        case ERROR_VECTOR_FRAME:
            Serial.print(F("INVALID FRAME VECTOR: 0x"));
            Serial.println(htonl(packet->frame_vector), HEX);
            break;
        case ERROR_VECTOR_DMP:
            Serial.print(F("INVALID DMP VECTOR: 0x"));
            Serial.println(packet->dmp_vector, HEX);
    }
}

uint16_t parsePacket()
{
    e131_error_t error;
    uint16_t retval = 0;

     int size = udp.receivePacket(packet->raw, E131_PACKET_SIZE);
    if (size > 0)
    {
    	packetNumber++;
        error = validateE131Packet();

        if (!error)
        {
            
            universe = htons(packet->universe);
            data = packet->property_values;

            retval = htons(packet->property_value_count) - 1;
            if (packet->sequence_number != sequence++)
            {
                stats.sequence_errors++;
                sequence = packet->sequence_number + 1;
            }
            stats.num_packets++;
        }
        else
        {
            dumpError(error);
            stats.packet_errors++;
            
        }
    }
IPAddress remoteAddr = udp.remoteIP();
    Serial.printlnf("## packet=%d addr=%s data= %d",packetNumber, remoteAddr.toString().c_str(), data[0]);
    return retval;
}

