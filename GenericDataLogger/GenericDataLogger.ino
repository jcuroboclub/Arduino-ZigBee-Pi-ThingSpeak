/*
 * Generic Data Logger example
 * A starting framework to make easy-to-manage dataloggers.
 *
 * This exmaple uses sparkfun weather shield, but is simple to extend to other sensors.
 * It will send data over Serial, using various formats (see the dump functions).
 *
 * Troubleshooting:
 * - Check your paramters and constants. e.g., did you add or remove sensors or dumps?
 *   Is N_SENSORS and N_DUMPS still the correct size?
 *
 * Created by Ashley Gillman, 09/05/15
*/
#include<stdlib.h>
#include<string.h>
#include <Wire.h> // I2C
#include "DataLogger.h"
#include "MemoryFree.h" // https://github.com/maniacbug/MemoryFree

// Include any sensor libraries here
#include "MPL3115A2.h" // Pressure sensor
#include "HTU21D.h" // Humidity sensor

#define DEBUG 1

// pins
const byte STAT1 = 7;
const byte STAT2 = 8;
const byte LIGHT = A1;
const byte REFERENCE_3V3 = A3;
const byte SOUND = A0;

// parameters
const unsigned long UPDATE = 15500; // update interval, 15.5s (ThingSpeak preferred)
const unsigned int SOUND_UPDATE = 1000;
const short N_SENSORS = 5;
const short N_DUMPS = 2;

// Sensors
const ReadingFnPointer sensors[] =
  {*getHumidityStr, *getTemperatureStr, *getPressureStr, *getLightStr, *getSoundStr};
const unsigned int channels[] =
  {37372,           37372,              37372,           37372,        37372       };
const int fields[] =
  {1,               2,                  3,               4,            5           };

// other Constants
const char READING_WIDTH = 12; // xxxxxx.xxx
const char READING_PRECISION = 3;

// global vars
uint8_t * heapptr, * stackptr;
unsigned long lastUpdate; //The millis counter to see when a second rolls by
unsigned long lastSoundUpdate;
unsigned long soundSum;
unsigned int soundCount;
MPL3115A2 pressure;
HTU21D humidity;
DataLogger dataLogger;
unsigned long sound_pres = 0;

void setup() {
  // General setup
  Serial.begin(9600);
  Serial.print("Beginning...");
  pinMode(STAT1, OUTPUT); //Status LED Blue
  pinMode(STAT2, OUTPUT); //Status LED Green
  
  // Add custom sensor setup code to this function.
  setupSensors();
  
  // Configure DataLogger
  dataLogger = DataLogger(N_SENSORS, READING_WIDTH, N_DUMPS);
  // Configure data logger inputs
  for (int i=0; i<N_SENSORS; ++i) {
    dataLogger.addReading(sensors[i]);
  }
  // configure data logger outputs
  dataLogger.addDump(*dumpToSerial);
  dataLogger.addDump(*dumpToSerialJSON);
  
  Serial.println(" done.");
}

void loop() {
  // Every UPDATE milliseconds, or on millis overflow, update again
  if ( (millis() - lastUpdate >= UPDATE) || (lastUpdate > millis()) ) {
    digitalWrite(STAT1, HIGH); // Blink stat LED
    lastUpdate += UPDATE; // update timer
    lastSoundUpdate = lastUpdate;
    
    dataLogger.takeReadings(); // update readings from sensors
    dataLogger.dumpReadings(); // write out to XBee
    
    //debugStats(); // Optional debugging
    digitalWrite(STAT1, LOW); // Turn off stat LED
  }
  // Every SOUND_UPDATE milliseconds, or on millis overflow, update again
  else if ((millis() - lastSoundUpdate >= SOUND_UPDATE) || lastSoundUpdate > millis() ) {
    lastSoundUpdate += SOUND_UPDATE;
    soundSum += analogRead(SOUND);
    soundCount += 1;
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// SENSORS
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setupSensors() {
  // Configure the pressure and temperature sensor
  pressure.begin(); // Get sensor online
  pressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  pressure.setOversampleRate(7); // Set Oversample to the recommended 128
  pressure.enableEventFlags(); // Enable all three pressure and temp event flags 

  // Configure the humidity sensor
  humidity.begin();
}

// Sensor input functions should take a buffer, and save their value to
// that buffer. They must not have a return value.

void getHumidityStr(char *buf) {
  dtostrf(humidity.readHumidity(), READING_WIDTH, READING_PRECISION, buf);
}

void getTemperatureStr(char *buf) {
  dtostrf(pressure.readTemp(), READING_WIDTH, READING_PRECISION, buf);
}

void getPressureStr(char *buf) {
  dtostre(pressure.readPressure(), buf, READING_WIDTH-6, 0);
}

void getLightStr(char *buf) {
  float operatingVoltage = analogRead(REFERENCE_3V3);
  float lightSensor = analogRead(LIGHT);
  operatingVoltage = 3.3 / operatingVoltage; //The reference voltage is 3.3V
  lightSensor = operatingVoltage * lightSensor;
  
  dtostrf(lightSensor, READING_WIDTH, READING_PRECISION, buf);
}

void getSoundStr(char *buf) {
  dtostrf((float)soundSum / (float)soundCount, READING_WIDTH, READING_PRECISION, buf);
  soundSum = 0;
  soundCount = 0;
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// DUMP
// Theses are the data output routines. They should take as input a string
// of data (char*), followed by two shorts which represent the
// two-dimensional shape of the data (no. sensors and size of each output
// value).
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Writes results to Serial.
// Start char: '!', end char: '\n', delimiter: ','.
void dumpToSerial(char* data, short height, short width) {
  Serial.print('!');
  for (int i=0; i<height; ++i) {
    for (int j=0; j<width; ++j) {
      //Serial.print(" ");
      //Serial.print(i*width+j);
      //Serial.print(":");
      Serial.print(data[i*width+j]);
    }
    if (i<height-1)
      Serial.print(',');
  }
  Serial.println();
}

// Writes to Serial for ThingSpeak uploading
// Start char: '^', end char: '\n', body is in JSON format.
void dumpToSerialJSON(char* data, short height, short width) {
  Serial.print("^[");
  for (int i=0; i<height; ++i) {
    //debugStats();
    delay(5);
    Serial.print(F("{channel: "));
    Serial.print(channels[i]);
    Serial.print(F(",field: "));
    Serial.print(fields[i]);
    Serial.print(F(",data: "));
    for (int j=0; j<width; ++j) {
      if (data[i*width+j] != ' ') // optional: omit whitespace
        Serial.print(data[i*width+j]);
    }
    if (i<height-1)
      Serial.print("},");
    else
      Serial.print('}');
  }
  Serial.println(']');
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Debugging Code
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void debugStats() {
  if (DEBUG) {
    Serial.print("FM: "); Serial.println(freeMemory());
    check_mem();
    Serial.print("HP: "); Serial.println((unsigned int) heapptr);
    Serial.print("SP: "); Serial.println((unsigned int) stackptr);
  }
}
  

/* This function places the current value of the heap and stack pointers in the
* variables. You can call it from any place in your code and save the data for
* outputting or displaying later. This allows you to check at different parts of
* your program flow.
* The stack pointer starts at the top of RAM and grows downwards. The heap pointer
* starts just above the static variables etc. and grows upwards. SP should always
* be larger than HP or you'll be in big trouble! The smaller the gap, the more
* careful you need to be. Julian Gall 6-Feb-2009.
*/
void check_mem() {
  //uint8_t * heapptr, * stackptr;  // I declared these globally
  stackptr = (uint8_t *)malloc(4);  // use stackptr temporarily
  heapptr = stackptr;                  // save value of heap pointer
  free(stackptr);                        // free up the memory again (sets stackptr to 0)
  stackptr =  (uint8_t *)(SP);       // save value of stack pointer
}
