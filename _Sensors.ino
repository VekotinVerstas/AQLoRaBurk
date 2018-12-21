#define SENSOR_SEND_MAX_DELAY 60000
#define BME280_READ_DELAY 1000
#define BME680_READ_DELAY 1000
#define SDS011_READ_DELAY 1000

void init_sensors() {
  init_bme280();
  init_bme680();
  init_sds011();
}

void read_sensors() {
  read_bme280();
  read_bme680();
  read_sds011();
}


void init_bme280() {
  Serial.print(F("INIT BME280: "));
  if (bme280.begin(0x76)) {
    Serial.println(F("found"));
    bme280_ok = 1;
  } else {
    Serial.println(F("not found"));
  }
}

void read_bme280() {
  // Read BME280 if it has been initialised successfully and it is time to read it
  if ((bme280_ok == 1) && (millis() > (bme280_lastRead + BME280_READ_DELAY))) {
    bme280_lastRead = millis();
    float humi = bme280.readHumidity();
    float temp = bme280.readTemperature();
    float pres = bme280.readPressure() / 100.0F;
    // Do something for the data here
    bme280_lastSend = millis();
    bme280_lastTemp = temp;
    bme280_lastHumi = humi;
    bme280_lastPres = pres;
  }
}

void init_bme680() {
  Serial.print(F("INIT BME680: "));
  if (bme680.begin()) {
    Serial.println(F("found"));
    // Set up oversampling and filter initialization
    bme680.setTemperatureOversampling(BME680_OS_8X);
    bme680.setHumidityOversampling(BME680_OS_2X);
    bme680.setPressureOversampling(BME680_OS_4X);
    bme680.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme680.setGasHeater(320, BME680_HEATING_TIME); // 320*C for 150 ms, from settings.h
    bme680_ok = 1;
  } else {
    Serial.println(F("not found"));
  }
}

void read_bme680() {
  // Read BME680 if it has been initialised successfully and it is time to read it
  if ((bme680_ok == 1) && (millis() > (bme680_lastRead + BME680_READ_DELAY))) {
    if (! bme680.performReading()) {
      Serial.println("Failed to perform reading :(");
      return;
    } else {
      bme680_lastRead = millis();
      float temp = bme680.temperature;
      float humi = bme680.humidity;
      float pres = bme680.pressure / 100.0F;
      float gas = bme680.gas_resistance / 1000.0F;
      // Do something for the data here
      bme680_lastSend = millis();
      bme680_lastTemp = temp;
      bme680_lastHumi = humi;
      bme680_lastPres = pres;
      bme680_lastGas = gas;
    }
  }
}

void init_sds011() {
  Serial.print(F("INIT sds011: "));
  sds011.setup(&Serial2);
  sds011.onData([](float pm25Value, float pm10Value) {
    if (pm_array_counter < pm_array_size) {
      sds011_pm25[pm_array_counter] = pm25Value;
      sds011_pm10[pm_array_counter] = pm10Value;
      pm_array_counter++;
    } else {
      Serial.println("Array full!");
      pm_array_counter = 0;
    }

    Serial.print("PM2.5: ");
    Serial.print(pm25Value);
    Serial.print(" PM10: ");
    Serial.println(pm10Value);
  });
  /*
    sds011.onResponse([](){
      // Serial.println("Response");
      // command has been executed
    });
  */
  sds011.onError([](int8_t error) {
    Serial.println("EROR");
    // error happened
    // -1: CRC error
  });
  sds011.setWorkingPeriod(5);
}

void read_sds011() {
  sds011.loop();
}
