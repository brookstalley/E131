// Example usage for E131 library by AdmiralTriggerHappy.

#include "E131.h"
SYSTEM_THREAD(ENABLED);

// Initialize objects from the lib
E131 e131;

	//Setup LED control pins
    int blueLED = D0;
    int greenLED = D1;
    int redLED = D2;

void setup() {
    // Call functions on initialized library objects that require hardware
    
    // Set the LED control pin to output
   pinMode(blueLED, OUTPUT);
   pinMode(redLED, OUTPUT);
   pinMode(greenLED, OUTPUT);

   // For good measure, let's also make sure all LEDs are off when we start:
   //In this example Common Cathode RGB LED is used so High is off.
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
	
		//Use the first 3 channels to control the Red, Green and Blue LED respectively
	   redBright(e131.data[1]);
       greenBright(e131.data[2]);
       blueBright(e131.data[3]);

}

//Some functions to set the brightness for each of the LEDs, as LEDs are common Cathode brightness value is inverted.

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


