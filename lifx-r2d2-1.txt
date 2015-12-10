// This #include statement was automatically added by the Particle IDE.
#include "spark-dallas-temperature/spark-dallas-temperature.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire/OneWire.h"// This #include statement was automatically added by the Particle IDE.

DallasTemperature dallas(new OneWire(D2));
int pwr = A5;

float fault = -100.0;

void setup(){
    // Start the serial out.
    Serial.begin(9600);
    dallas.begin();
    
    // Registry the pins
    pinMode(pwr, OUTPUT); // The pin powering the temperature sensor is output (sending out consistent power)
}

void loop(){
    // Turn the power to the temperature sensor on.
    digitalWrite(pwr, HIGH);
    
    // Read the temperature.
    dallas.requestTemperatures();
    float tempF = dallas.getTempFByIndex( 0 );
    
    if (tempF > fault) {
        // If the temperature was read successfully, 
        // Publish the temperature to the particle cloud and print it to the serial out.
        char tempFStr[64];
        sprintf(tempFStr, "%2.2f", tempF);
        Particle.publish("outside-temp-f", tempFStr, 60, PRIVATE);
        Serial.println(tempFStr);
    }
    
    delay(5000);
}