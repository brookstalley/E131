// Example usage for E131 library by AdmiralTriggerHappy.

#include "E131.h"
SYSTEM_THREAD(ENABLED);

// Initialize objects from the lib
E131 e131;

 //Test Code
    int blueLED = D0;
    int greenLED = D1;
    int redLED = D2;

void setup() {
    // Call functions on initialized library objects that require hardware
    
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
	Serial.begin(115200);
    waitUntil(WiFi.ready);
	e131.begin();
}
void loop() {

	e131.parsePacket();
	//Serial.printlnf("## data= %d , %d, %d", e131.data[0],e131.data[1],e131.data[2]);
	redBright(e131.data[0]);
       greenBright(e131.data[1]);
       blueBright(e131.data[2]);

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


