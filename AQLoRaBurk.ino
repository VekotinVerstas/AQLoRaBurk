/*******************************************************************************
 * Copyright (c) 2018 Vekotinverstas / Forum Virium Helsinki
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * LoRaWAN part is heavily copied from lmic library's examples.
 * 
 * Do not forget to define the radio type correctly 
 * #define CFG_eu868 1 
 * in
 * libraries/MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
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
