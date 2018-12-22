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
#include <SPI.h>
#include "settings.h"
// Sensor support libraries
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BME680.h>
#include "SDS011.h"
#include "QuickStats.h"

QuickStats stats;

// LoRa payload
#define payloadSize 16
static uint8_t payload[payloadSize];

// I2C settings
// TODO: Check correct ESP32 pins
#define SDA 2
#define SCL 1

#define BME680_HEATING_TIME 150 // milliseconds
#define SDS011_RXPIN 39
#define SDS011_TXPIN 36

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

static osjob_t sendjob;

void setup() {
  while (!Serial); // wait for Serial to be initialized
  Serial.begin(115200);
  delay(100);     // per sample code on RF_95 test
  Serial.println(F("Starting"));
  // SDS011 serial
  Serial2.begin(9600, SERIAL_8N1, SDS011_RXPIN, SDS011_TXPIN);
  init_sensors();
  lmic_init();
  // Start job
  do_send(&sendjob);
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

  float min10 = 0;
  float max10 = 0;
  float avg10 = 0;

  // More examples for statistics
  // https://github.com/dndubins/QuickStats/blob/master/examples/statistics/statistics.ino
  if (pm_array_counter > 0) {
    min25 = stats.minimum(sds011_pm25, pm_array_counter);
    max25 = stats.maximum(sds011_pm25, pm_array_counter);
    avg25 = stats.average(sds011_pm25, pm_array_counter);
    min10 = stats.minimum(sds011_pm10, pm_array_counter);
    max10 = stats.maximum(sds011_pm10, pm_array_counter);
    avg10 = stats.average(sds011_pm10, pm_array_counter);
  }
  char buffer [100];
  int cx;
  cx = snprintf ( buffer, 100, "Values to send: min2.5 %.1f max2.5 %.1f avg2.5 %.1f min10 %.1f max10 %.1f avg10 %.1f",
                  min25, max25, avg25, min10, max10, avg10 );

  Serial.println(buffer);
  uint16_t tmp;
  uint8_t i = 0;

  // 2 first bytes defines protocol
  payload[i++] = 0x2A;
  payload[i++] = 0x2A;
  i = addToPayload(payload, (uint16_t)(min25 * 10), i);
  i = addToPayload(payload, (uint16_t)(max25 * 10), i);
  i = addToPayload(payload, (uint16_t)(avg25 * 10), i);
  i = addToPayload(payload, (uint16_t)(min10 * 10), i);
  i = addToPayload(payload, (uint16_t)(max10 * 10), i);
  i = addToPayload(payload, (uint16_t)(avg10 * 10), i);
  payload[i++] = 0;
  payload[i++] = 0;

  pm_array_counter = 0;
}
