/*******************************************************************************
   Copyright (c) 2018 Vekotinverstas / Forum Virium Helsinki

   Permission is hereby granted, free of charge, to anyone
   obtaining a copy of this document and accompanying files,
   to do whatever they want with them without any restriction,
   including, but not limited to, copying, modification and redistribution.
   NO WARRANTY OF ANY KIND IS PROVIDED.

   LoRaWAN part is heavily copied from lmic library's examples.

   Do not forget to define the radio type correctly
   #define CFG_eu868 1
   in
   libraries/MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h or from your BOARDS.txt.

 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <Wire.h>
#include <SPI.h>
#include "settings.h"
// Sensor support libraries
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BME680.h>
#include <Adafruit_SSD1306.h>
#include "SDS011.h"
#include "QuickStats.h"

QuickStats stats;

// LoRa payload
#define payloadSize 26
static uint8_t payload[payloadSize];

// I2C settings
#define SDA 21
#define SCL 22

#define BME680_HEATING_TIME 150 // milliseconds
#define SDS011_RXPIN 39  // Connect to SDS011 TX
#define SDS011_TXPIN 36  // Connect to SDS011 RX

// BME280 sensor
Adafruit_BME280 bme280;
uint8_t bme280_ok = 0;
uint32_t bme280_lastRead = 0;
uint32_t bme280_lastSend = 0;
float bme280_lastHumi = -999;
float bme280_lastTemp = -999;
float bme280_lastPres = -999;

// BME680 AQ sensor
Adafruit_BME680 bme680;
uint8_t bme680_ok = 0;
uint32_t bme680_lastRead = 0;
uint32_t bme680_lastSend = 0;
float bme680_lastTemp = -999;
float bme680_lastHumi = -999;
float bme680_lastPres = -999;
float bme680_lastGas = -999;

SDS011 sds011;
uint8_t sds011_ok = 0;
#define pm_array_size 120
uint32_t pm_array_counter = 0;
float sds011_pm25[pm_array_size];
float sds011_pm10[pm_array_size];

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned long lastDisplay;
unsigned long maxDisplayTime = 15 * 60 * 1000; // how many milli seconds display will stay powered on

static osjob_t sendjob;

void setup() {
  while (!Serial); // wait for Serial to be initialized
  Serial.begin(115200);
  delay(100);     // per sample code on RF_95 test
  Serial.println(F("Starting"));
  printLoRaWANkeys();
  // SDS011 serial
  Serial2.begin(9600, SERIAL_8N1, SDS011_RXPIN, SDS011_TXPIN);
  init_sensors();
  displayInit();
  uint16_t t = random(1000, 5000);
  Serial.println(t);
  delay(t);
  lastDisplay = millis();
  lmic_init();
  // Start job
  do_send(&sendjob);
  // Initialise payload
  for (uint8_t i = 0; i < payloadSize; i++) {
    payload[i] = 0;
  }
  
}

void loop() {
  unsigned long now;
  now = millis();
  if ((now & 512) != 0) {
    digitalWrite(13, HIGH);
  }
  else {
    digitalWrite(13, LOW);
  }
  read_sensors();
  os_runloop_once();
  if (now > lastDisplay + 5000) {
    displaySensorvalues();
    lastDisplay = now;
  }
}

void printArray(const char *keyName, u1_t x[], uint8_t s) {
  char buf [2];
  Serial.print(keyName);
  Serial.print(": ");
  for ( int i = 0 ; i < s; i++ ) {
    sprintf(buf, "%02x", x[i]);
    Serial.print(buf);
  }
  Serial.println();
}

void printLoRaWANkeys() {
#ifdef PRINT_KEYS
  printArray("NWKSKEY", NWKSKEY, 16);
  printArray("APPSKEY", APPSKEY, 16);
  printArray("DEVEUI", DEVEUI, 8);
  Serial.print(F("DevAddr: 0x")); 
  Serial.println(DEVADDR, HEX);
#endif
}

void displayInit() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  Serial.println(F("SSD1306 allocated"));
  display.clearDisplay();
  display.setTextSize(2);             // Draw 2X-scale text
  display.setCursor(0, 0);
  display.setTextColor(BLACK, WHITE); // Draw 'inverse' text
  display.println(F("AQ Burk"));

  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.println(F("Version 0.0.1"));
  display.println(F("Dev "));
  display.print(F("0x")); display.println(DEVADDR, HEX);
  if ( bme280_ok || bme680_ok || sds011_ok) {
    if (bme280_ok) {
      display.println(F("BME280 OK"));
    }
    if (bme680_ok) {
      display.println(F("BME680 OK"));
    }
    if (sds011_ok) {
      display.println(F("SDS011 OK"));
    }
  } else {
    display.println(F("NO SENSORS FOUND"));
  }
  display.display();
  delay(1000);
}

void displaySensorvalues() {
  // maxDisplayTime = 1000*20;  // 20 sec for debugging
  if (millis() > (maxDisplayTime)) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    return;
  }
  if (millis() > maxDisplayTime - 10 * 1000) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.setTextColor(BLACK, WHITE);
    display.println(F("Shutting"));
    display.println(F("display"));
    display.println(F("down..."));
    display.display();
    return;
  }
  Serial.println(F("Display values"));
  float temp = 0;
  float humi = 0;
  float pres = 0;
  float gas = 0;

  uint8_t protocol = 0x2A;
  // Add BME sensor values, if they are read at least once
  if (bme280_ok && (bme280_lastTemp > -999)) {
    protocol = 0x2A;
    temp = bme280_lastTemp;
    humi = bme280_lastHumi;
    pres = bme280_lastPres;
    gas = 0;
  } else if (bme680_ok && (bme680_lastTemp > -999)) {
    protocol = 0x2B;
    temp = bme680_lastTemp;
    humi = bme680_lastHumi;
    pres = bme680_lastPres;
    gas = bme680_lastGas;
  }
  uint8_t bufsize = 30;
  char buf1 [bufsize];
  int cx;
  display.setCursor(0, 0);
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text

  cx = snprintf ( buf1, bufsize, "T %.1f'C  H %.1f%%", temp, humi);
  display.println(buf1);

  cx = snprintf ( buf1, bufsize, "Pres %.1f hPa", pres );
  display.println(buf1);

  cx = snprintf ( buf1, bufsize, "Gas  %.1f kohm", gas );
  display.println(buf1);

  if (pm_array_counter > 0) {
    cx = snprintf ( buf1, bufsize, "PM2.5/10 %.1f %.1f", sds011_pm25[pm_array_counter-1], sds011_pm10[pm_array_counter-1] );
    display.println(buf1);
  }

  cx = snprintf ( buf1, bufsize, "Uptime %.1f s", (millis() / 1000.0) );
  display.println(buf1);

  display.display();

  Serial.println(buf1);
}


uint32_t addToPayload(uint8_t pl[], uint16_t val, uint32_t i) {
  pl[i++] = val >> 8;
  pl[i++] = val & 0x00FF;
  return i;
}

/**
   Generate payload which is an uint8_t array full of uint16_t values meaning
   bytes 0 and 1 contain the first uint16_t value and so on.
*/
void generatePayload() {
  float min25 = 0;
  float max25 = 0;
  float avg25 = 0;
  float med25 = 0;

  float min10 = 0;
  float max10 = 0;
  float avg10 = 0;
  float med10 = 0;

  float temp = 0;
  float humi = 0;
  float pres = 0;
  float gas = 0;

  uint8_t protocol = 0x2A;
  // More examples for statistics
  // https://github.com/dndubins/QuickStats/blob/master/examples/statistics/statistics.ino
  if (pm_array_counter > 0) {
    min25 = stats.minimum(sds011_pm25, pm_array_counter);
    max25 = stats.maximum(sds011_pm25, pm_array_counter);
    avg25 = stats.average(sds011_pm25, pm_array_counter);
    med25 = stats.median(sds011_pm25, pm_array_counter);
    min10 = stats.minimum(sds011_pm10, pm_array_counter);
    max10 = stats.maximum(sds011_pm10, pm_array_counter);
    avg10 = stats.average(sds011_pm10, pm_array_counter);
    med10 = stats.median(sds011_pm10, pm_array_counter);
  }
  // Add BME sensor values, if they are read at least once
  if (bme280_ok && (bme280_lastTemp > -999)) {
    protocol = 0x2A;
    temp = bme280_lastTemp + 100; // add 100 to make value always positive
    humi = bme280_lastHumi;
    pres = bme280_lastPres;
    gas = 0;
  } else if (bme680_ok && (bme680_lastTemp > -999)) {
    protocol = 0x2B;
    temp = bme680_lastTemp + 100; // add 100 to make value always positive
    humi = bme680_lastHumi;
    pres = bme680_lastPres;
    gas = bme680_lastGas;
  }
  char buffer [200];
  int cx;
  cx = snprintf ( buffer, 200, "Values to send: min2.5 %.1f max2.5 %.1f avg2.5 %.1f med2.5 %.1f min10 %.1f max10 %.1f avg10 %.1f med10 %.1f temp %.1f humi %.1f pres %.1f gas %.1f",
                  min25, max25, avg25, med25, min10, max10, avg10, med10, temp, humi, pres, gas );

  Serial.println(buffer);

  uint16_t tmp;
  uint8_t i = 0;

  // 2 first bytes defines protocol
  payload[i++] = 0x2A;
  payload[i++] = protocol;  // 2A=BME280, 2B=BME680
  i = addToPayload(payload, (uint16_t)(min25 * 10), i);
  i = addToPayload(payload, (uint16_t)(max25 * 10), i);
  i = addToPayload(payload, (uint16_t)(avg25 * 10), i);
  i = addToPayload(payload, (uint16_t)(med25 * 10), i);
  i = addToPayload(payload, (uint16_t)(min10 * 10), i);
  i = addToPayload(payload, (uint16_t)(max10 * 10), i);
  i = addToPayload(payload, (uint16_t)(avg10 * 10), i);
  i = addToPayload(payload, (uint16_t)(med10 * 10), i);
  i = addToPayload(payload, (uint16_t)(temp * 10), i);
  i = addToPayload(payload, (uint16_t)(humi * 10), i);
  i = addToPayload(payload, (uint16_t)(pres * 10), i);
  i = addToPayload(payload, (uint16_t)(gas * 10), i);

  pm_array_counter = 0;
}
