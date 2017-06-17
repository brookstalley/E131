SYSTEM_THREAD(ENABLED); // This makes the system cloud connection run on a background thread so as to not delay our timing

/*
 * E131.ino
 *
 * Project: E131 - E.131 (sACN) library for Particle
 
 * Copyright (c) 2017 AdmiralTriggerHappy
 * Based in part on work by Shelby Merrick (http://www.forkineye.com)
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

// Don't change this unless you really know what you're doing!
#define DEFAULT_UNIVERSE_SIZE 512
#define MAXIMUM_UNIVERSE_ACCEPTED 128

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

/* E1.31 Listener Types */
typedef enum
{
    E131_UNICAST,
    E131_MULTICAST
} e131_listen_t;

/* Constants for packet validation */
static const char ACN_ID[12] = { 0x41, 0x53, 0x43, 0x2d, 0x45, 0x31, 0x2e, 0x31, 0x37, 0x00, 0x00, 0x00 };
static const uint32_t VECTOR_ROOT = 4;
static const uint32_t VECTOR_FRAME = 2;
static const uint8_t VECTOR_DMP = 2;

uint8_t sequence; /* Sequence tracker */

uint8_t *data; // Pointer to DMX channel data
uint16_t universe; // DMX Universe of last valid packet
e131_packet_t packetBuffer1; // Packet buffer
e131_packet_t packetBuffer2; // Double buffer
e131_packet_t *packetWorkingBuffer; // Pointer to working packet buffer
e131_packet_t *packet; // Pointer to last valid packet
e131_stats_t  stats; // Statistics tracker

// An UDP instance to let us send and receive packets over UDP
UDP udp;
int lastDrawTime = 0;
const int millisecondsBetweenDrawCalls = 20;

bool previousWiFiReadiness = false;
bool wiFiReadiness = false;
int wiFiDisconnectTime = 0;

    IPAddress myAddress(192,168,1,199);
    IPAddress netmask(255,255,255,0);
    IPAddress gateway(192,168,1,1);
    IPAddress dns(192,168,1,1);

IPAddress myIp;
String myIpString = "";
String firmwareVersion = "0000000012";
String systemVersion = "";
String dmxData = "Blank";


/* Diag functions */
void dumpError(e131_error_t error);
uint16_t parsePacket();
e131_error_t validateE131Packet();

// My functions
void checkForUDPData();


 //Test Code
    int blueLED = D0;
    int greenLED = D1;
    int redLED = D2;

void setup()
{
    //Test Code
    // Here's the pin configuration, same as last time
   pinMode(blueLED, OUTPUT);
   pinMode(redLED, OUTPUT);
   pinMode(greenLED, OUTPUT);

   // This is saying that when we ask the cloud for the function "led", it will employ the function ledToggle() from this app.

   // For good measure, let's also make sure both LEDs are off when we start:
   digitalWrite(blueLED, HIGH);
   digitalWrite(redLED, HIGH);
   digitalWrite(greenLED, HIGH);
   
   //End Test Code

    memset(packetBuffer1.raw, 0, sizeof(packetBuffer1.raw));
    memset(packetBuffer2.raw, 0, sizeof(packetBuffer2.raw));
    packet = &packetBuffer1;
    packetWorkingBuffer = &packetBuffer2;

    sequence = 0;
    stats.num_packets = 0;
    stats.sequence_errors = 0;
    stats.packet_errors = 0;

    // Auto Wi-Fi Antenna Selection
    WiFi.selectAntenna(ANT_AUTO); // ANT_INTERNAL ANT_EXTERNAL ANT_AUTO

    // Setup the Serial connection for debugging
    Serial.begin(115200);

    // Setup cloud variables and functions
    systemVersion = System.version();
    
    
    WiFi.setStaticIP(myAddress, netmask, gateway, dns);

    // now let's use the configured IP
    WiFi.useStaticIP();
    
    
    myIpString = String(String(myIp[0], DEC) + "." + String(myIp[2], DEC) + "." + String(myIp[2], DEC) + "." + String(myIp[3], DEC));
    Particle.variable("localIP", myIpString);
    Particle.variable("e131FVersion", firmwareVersion);
    Particle.variable("sysVersion", systemVersion);
    Particle.variable("DMX",dmxData);

    // Setup the UDP connection
    waitUntil(WiFi.ready);
    udp.begin(E131_DEFAULT_PORT);
    udp.joinMulticast(myAddress);

}

void loop()
{
    checkWiFiStatus();
    //dmxData = "State0";

   // checkForTestingMode();

    checkForUDPData();



}

void checkWiFiStatus()
{
    // Check to see if the WiFi connection was lost.
    previousWiFiReadiness = wiFiReadiness;
    wiFiReadiness = WiFi.ready();
    // WiFi stopped. Release udp so we can reinit once it's ready again
    if(wiFiReadiness == false && previousWiFiReadiness == true)
    {
        Serial.println("WiFi not ready");
        udp.stop();
        wiFiDisconnectTime = millis();
    }
    // WiFi is back. Open up udp port again
    else if(wiFiReadiness == true && previousWiFiReadiness == false)
    {
        Serial.println("WiFi Back online");
        myIp = WiFi.localIP();
        myIpString = String(String(myIp[0], DEC) + "." + String(myIp[2], DEC) + "." + String(myIp[2], DEC) + "." + String(myIp[3], DEC));
        Serial.print("ip:");
        Serial.println(myIp);

        udp.begin(E131_DEFAULT_PORT);
        udp.joinMulticast(myAddress);
        
    }
    // WiFi connection was lost and has been for a while
    else if(wiFiReadiness == false && previousWiFiReadiness == false && wiFiDisconnectTime > 0 && millis() - wiFiDisconnectTime > 5000)
    {
        wiFiDisconnectTime = 0;
        // Reboot since we can't seem to find a WiFi connection
        System.reset();
    }
}

void checkForTestingMode()
{
   
}

void checkForUDPData()
{
   // dmxData = "State1";
    // Parse a packet and update pixels
    int dataSize = parsePacket();
    if(dataSize > 0)
    {
        dmxData = "State2";
       redBright(data[1]);
       redBright(data[2]);
       redBright(data[3]);
                       for(int i=0; i < sizeof(data)/sizeof(data[0]);i++)
    {
        dmxData = dmxData + data[i];
        dmxData = dmxData + ", ";
    }

    }
}

/* Main packet parser */
uint16_t parsePacket()
{
    e131_error_t error;
    uint16_t retval = 0;
    
    //dmxData = "State0";
    
    //udp.receivePacket(packetWorkingBuffer->raw, E131_PACKET_SIZE);
     int size = udp.receivePacket(packetWorkingBuffer->raw, E131_PACKET_SIZE);
    if (size > 0)
    {
        dmxData = "State1";
        udp.read(packetWorkingBuffer->raw, size);
        error = validateE131Packet();
        if (!error)
        {
            e131_packet_t *swap = packet;
            packet = packetWorkingBuffer;
            packetWorkingBuffer = swap;
            //printUDPData(packetWorkingBuffer->raw, size);
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
    else
    {
        dmxData = "Error0";
    }
    
    return retval;
}

/* Packet validater */
e131_error_t validateE131Packet()
{
    if (memcmp(packetWorkingBuffer->acn_id, ACN_ID, sizeof(packetWorkingBuffer->acn_id)) !=0)
     dmxData = "Error1";
        dmxData = packetWorkingBuffer->acn_id;
        return ERROR_ACN_ID;
    if (htonl(packetWorkingBuffer->root_vector) != VECTOR_ROOT)
    dmxData = "Error2";
        return ERROR_VECTOR_ROOT;
    if (htonl(packetWorkingBuffer->frame_vector) != VECTOR_FRAME)
    dmxData = "Error3";
        return ERROR_VECTOR_FRAME;
    if (packetWorkingBuffer->dmp_vector != VECTOR_DMP)
        dmxData = "Error4";
        return ERROR_VECTOR_DMP;
    return ERROR_NONE;
}

void dumpError(e131_error_t error)
{
    switch (error) {
        case ERROR_ACN_ID:
            Serial.print(F("INVALID PACKET ID: "));
            for (uint8_t i = 0; i < sizeof(ACN_ID); i++)
                Serial.print(packetWorkingBuffer->acn_id[i], HEX);
            Serial.println("");
            break;
        case ERROR_PACKET_SIZE:
            Serial.println(F("INVALID PACKET SIZE: "));
            break;
        case ERROR_VECTOR_ROOT:
            Serial.print(F("INVALID ROOT VECTOR: 0x"));
            Serial.println(htonl(packetWorkingBuffer->root_vector), HEX);
            break;
        case ERROR_VECTOR_FRAME:
            Serial.print(F("INVALID FRAME VECTOR: 0x"));
            Serial.println(htonl(packetWorkingBuffer->frame_vector), HEX);
            break;
        case ERROR_VECTOR_DMP:
            Serial.print(F("INVALID DMP VECTOR: 0x"));
            Serial.println(packetWorkingBuffer->dmp_vector, HEX);
    }
}

//Test Code

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
