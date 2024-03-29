/*
  LED

  This example creates a BLE peripheral with service that contains a
  characteristic to control an LED.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <FastLED.h>
#define LED_PIN     7
#define NUM_LEDS    105
CRGB leds[NUM_LEDS];
int BUFFER_SIZE = (NUM_LEDS + 2) * 4;

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLECharacteristic switchCharacteristic("00001803-0000-1000-8000-00805f9b34fb", BLERead | BLEWrite, BUFFER_SIZE);
//BLEWordCharacteristic colorCharacteristic("00001803-0000-1000-8000-00805f9b34fb", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN; // pin to use for the LED

void setup() {
//  Serial.begin(9600);
//  while (!Serial);
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB(255, 255, 255);
  FastLED.show();

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);
  //ledService.addCharacteristic(colorCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  //switchCharacteristic.writeValue(0);
  //colorCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  delay(20);
  leds[0] = CRGB(255, 0, 0);
  leds[1] = CRGB(0, 0, 0);
  leds[2] = CRGB(0, 0, 0);
  FastLED.show();
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    static CHSV color = CHSV(0, 255, 255);
    Serial.print("Connected to central: ");
    // // print the central's MAC address:
    Serial.println(central.address());
    digitalWrite(ledPin, HIGH);         // will turn the LED on
    leds[1] = CRGB(0, 255, 0);
    FastLED.show();
    delay(20);

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
          //for (int i = 0 ; i < NUM_LEDS ; i++) {
          //  leds[i] = CRGB(0, 0, 0);
          //}
          FastLED.clear();
          byte buffer[BUFFER_SIZE];
          int typeEnds = 0;
          int length = switchCharacteristic.readValue(buffer, BUFFER_SIZE);
          int i = 0;
          while (i < length && typeEnds < 4) {
            int typeIndex = 0;
            Serial.println("New Type");
            while (buffer[i + typeIndex] != 0xFF && i < length) {
              if (typeIndex == 0) {
                Serial.println("New Color");
                Serial.println(buffer[i + typeIndex]);
                color = CHSV(buffer[i + typeIndex], 255, 255);
              } else {
                int position = buffer[i + typeIndex];
                leds[position] = color;
                Serial.println("Position");
                Serial.println(position);
              }
              typeIndex++;
            }
            i += (typeIndex + 1);
            typeEnds++;
          }
          Serial.println("Show led");
          FastLED.show();
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
  digitalWrite(ledPin, LOW);          // will turn the LED off
}
