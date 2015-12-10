// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_SSD1306/Adafruit_GFX.h"
#include "Adafruit_SSD1306/Adafruit_SSD1306.h"

// This #include statement was automatically added by the Particle IDE.
#include "SparkFunMicroOLED/SparkFunMicroOLED.h"

// This #include statement was automatically added by the Particle IDE.
#include "HttpClient/HttpClient.h"

// References:  
// - https://www.hackster.io/steph/the-jack-o-lantern-early-warning-system-7f09a2
// - http://www.mycontraption.com/sound-effects-with-and-arduino/
// - https://github.com/pkourany/Adafruit_SSD1306/blob/master/ssd1306_128x64_spi.ino

#define MotionSensorPin A0
#define PiezoPin D0
#define ledRedPin D1
#define ledGreenPin D2

// use hardware SPI
// OLED_D0 -> A3 (SPI CLK)
// OLED_D1 -> A5 (SPI MOSI)
#define OLED_DC     D3
#define OLED_CS     D4
#define OLED_RESET  D5
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

const float note_A7 = 3520.00;
const float note_B7 = 3951.07;
const float note_C7 = 2093.00;
const float note_D7 = 2349.32;
const float note_E7 = 2637.02;
const float note_F7 = 2793.83;
const float note_G7 = 3135.96;
const float note_C8 = 4186.01;

float tempF;
char tempFStr[50];
bool tempSet = FALSE;

static const int LOOP_DELAY = 1000;
bool lightOn = false;
int lightDuration = 0;
static const int LIGHT_TIMEOUT = 5 *500; // Keep the light on for 5 seconds.

static const char* BLUE     = "blue";
static const char* YELLOW   = "yellow";
static const char* RED      = "red";

static const char* LIFX_HOST = "lifx-temp-indicator.appspot.com";
static const char* LIFX_KEY = "cc0b02e0d6b069ce12def4d38f6e4c1f8968fd250b02635ae09124d7cc593e0f";
static const char* LIFX_BRIGHTNESS = "0.25";
static const char* LIFX_DURATION = "0.0";
static const char* LIFX_SELECTOR = "id:d073d511a66a";

static const char* LIFX_ON_PATH = "/lifx?action=on&key=%s&selector=%s&color=%s&brightness=%s&duration=%s";
static const char* LIFX_OFF_PATH = "/lifx?action=off&key=%s&selector=%s";

static const float HOT = 75.0;
static const float COLD = 55.0;

HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    { "Accept", "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

void setup() {
    // Subscribe to the temperature produced by the particle chip outside my house.
    Particle.subscribe("outside-temp-f", tempHandler, MY_DEVICES);
    
    // Open the serial port for writing.
    Serial.begin(9600);
    
    // Register the pins. 
    pinMode(MotionSensorPin, INPUT); 
    pinMode(PiezoPin, OUTPUT);    
    pinMode(ledRedPin, OUTPUT);          
    pinMode(ledGreenPin, OUTPUT);
  
    // Initialize the LCD display.
    // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
    display.begin(SSD1306_SWITCHCAPVCC);
    display.clearDisplay();
    // init done
    
    // Init the http request instance
    request.hostname = LIFX_HOST;
    request.port = 80;
    
    // Turn the LIFX light off.  It may or may not be turned on but we don't know.
    turnLightOff();
    
    //lets wait a few seconds for the display to calibrate...
    delay(5000);
    Serial.println("Ok!");
}

void loop() {
    
    if (lightOn && lightDuration <= LIGHT_TIMEOUT) {
        // The light is on.
        // Update lightDuration.
        Serial.println("Update ligth duration");
        lightDuration += LOOP_DELAY;
        delay(LOOP_DELAY);
        
    } else if (lightOn && lightDuration > LIGHT_TIMEOUT) {
        // The light is on.
        // Turn light off
        Serial.println("Turn light off");
        
        if (r2d2ActionOff() == 200) {
            lightOn = false;
            lightDuration = 0;
        }
        delay(LOOP_DELAY);
        
    } else {
        // Check to see if we need to turn the light on.
        // read motion sensor
        bool motionDetected = (digitalRead(MotionSensorPin) == HIGH);
        if (motionDetected && tempSet) {
            
            // Trigger the R2D2 action which plays some R2D2 sounds, flashes some LED's, displays the temperature, and turns on the LIFX bulb.
            Serial.println("Turn light on");
            r2d2ActionOn();
            if (turnLightOn() == 200) {
                lightOn = true;
            }
        }
    }
}  

//
// Trigger the R2D2 Action
// Beep like R2D2.
// Flash the red and green LED's.
// Display the temperature on the LCD display.
// Turn the LIFX bulb on.
//
void r2d2ActionOn() {
    display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0,25);
    display.println(tempFStr);
    display.display();
    
    digitalWrite(ledRedPin, HIGH);
    beep(PiezoPin, note_A7,100); //A
    beep(PiezoPin, note_G7,100); //G
    beep(PiezoPin, note_E7,100); //E
    digitalWrite(ledRedPin, LOW);
    beep(PiezoPin, note_C7,100); //C
    beep(PiezoPin, note_D7,100); //D
    beep(PiezoPin, note_B7,100); //B
    digitalWrite(ledGreenPin, HIGH);
    beep(PiezoPin, note_F7,100); //F
    beep(PiezoPin, note_C8,100); //C
    beep(PiezoPin, note_A7,100); //A
    digitalWrite(ledGreenPin, LOW);
    beep(PiezoPin, note_G7,100); //G
    beep(PiezoPin, note_E7,100); //E
    beep(PiezoPin, note_C7,100); //C
    digitalWrite(ledRedPin, HIGH);
    beep(PiezoPin, note_D7,100); //D
    beep(PiezoPin, note_B7,100); //B
    beep(PiezoPin, note_F7,100); //F
    digitalWrite(ledRedPin, LOW);
    beep(PiezoPin, note_C8,100); //C
}

//
// Turn off the LCD display and the LIFX bulb.
//
int r2d2ActionOff() {
    display.clearDisplay();
    display.display();
    return turnLightOff();
}

//
// Turn the LIFX light off.
// 
int turnLightOff() {
    
    char path[150];
    sprintf(path, LIFX_OFF_PATH, LIFX_KEY, LIFX_SELECTOR);
    request.path = path;
    // Get request
    http.get(request, response, headers);
    Serial.print("Application>\tResponse status: ");
    Serial.println(response.status);

    Serial.print("Application>\tHTTP Response Body: ");
    Serial.println(response.body);
    return response.status;
}

//
// Turn the LIFX light on.
//
int turnLightOn() {
    
    char path[150];
    if (tempF >= HOT)
        sprintf(path, LIFX_ON_PATH, LIFX_KEY, LIFX_SELECTOR, RED, LIFX_BRIGHTNESS, LIFX_DURATION);
    else if (tempF <= COLD)
        sprintf(path, LIFX_ON_PATH, LIFX_KEY, LIFX_SELECTOR, BLUE, LIFX_BRIGHTNESS, LIFX_DURATION);
    else
        sprintf(path, LIFX_ON_PATH, LIFX_KEY, LIFX_SELECTOR, YELLOW, LIFX_BRIGHTNESS, LIFX_DURATION);
        
    request.path = path;
    // Get request
    http.get(request, response, headers);
    Serial.print("Application>\tResponse status: ");
    Serial.println(response.status);

    Serial.print("Application>\tHTTP Response Body: ");
    Serial.println(response.body);
    return response.status;
}

//
// Send a beep to the buzzer
//
void beep(int speakerPin, float noteFrequency, long noteDuration) {
    int x;
    // Convert the frequency to microseconds
    float microsecondsPerWave = 1000000/noteFrequency;
    // Calculate how many milliseconds there are per HIGH/LOW cycles.
    float millisecondsPerCycle = 1000/(microsecondsPerWave * 2);
    // Multiply noteDuration * number or cycles per millisecond
    float loopTime = noteDuration * millisecondsPerCycle;
    // Play the note for the calculated loopTime.
    for (x=0;x<loopTime;x++) {
        digitalWrite(speakerPin,HIGH);
        delayMicroseconds(microsecondsPerWave);
        digitalWrite(speakerPin,LOW);
        delayMicroseconds(microsecondsPerWave);
    }
}

//
// Callback handler to set the temperature sent from the particle chip outside my house
// 
void tempHandler(const char *event, const char *data) {
    Serial.println(event);
    Serial.println(data);
    tempF = atof(data);
    sprintf(tempFStr, "%s F", data);
    tempSet = TRUE;
}