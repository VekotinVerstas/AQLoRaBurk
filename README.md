# AQLoRaBurk

This Arduino sketch reads BME280 or BME680 environmental sensor
and SDS011 particulate matter (PM2.5/PM10) sensor values.

If they are correctly wired they should be automatically detected.

Data is transmitted over LoRaWAN network using ABP activation. 
Default sending interval is about one minute.

SDS011 values are read once in second, put into an array and when it is time to send
the data, QuickStats is used to calculate min, max, avg and mean of PM10 and PM2.5
values.

BME680 is read once in 30 seconds (default).

TTGO ESP32 LoRa V2 and V2.1 have been tested to be functional with this code.

Surprisingly this code has been very stable: dozens of deployments have been
running months without issues.  

# Gettings started

## Arduino libraries

Install the libraries below using Arduino IDE's

`Sketch->Include Library->Manage Libraries...`:

* Adafruit_Sensor
* Adafruit_BME280
* Adafruit_BME680
* Adafruit_SSD1306
* MCCI LoRaWAN LMIC library by IBM, Matthis Kooijman, Terry Moore etc.

### Define radio type in LMIC lib

Do not forget to define the radio type correctly 

`#define CFG_eu868 1`
 
in `libraries/MCCI_LoRaWAN_LMIC_library/project_config/lmic_project_config.h` or from your BOARDS.txt.

## The code

1. Clone this repository
2. Copy `settings-example.h` to `settings.h`
3. Check that `NWKSKEY`, `APPSKEY`, `DEVEUI` and `DEVADDR` have correct values in `settings.h`.
4. Open AQLoRaBurk.ino, compile and upload the code to the board

## LoRaWAN

Check very carefully all LoRaWAN network related values in `settings.h`.

## Wiring

Check pin numbers from `settings.h`:

```
// I2C settings
#define SDA 21           // Connect to BMEx80 SDA
#define SCL 22           // Connect to BMEx80 SCL

#define SDS011_RXPIN 39  // Connect to SDS011 TX
#define SDS011_TXPIN 36  // Connect to SDS011 RX
```

In addition:
* Connect 5V and GND to correct pins in SDS011.
* Connect 3V3 and GND to correct pins in BMEx80.

# Known issues
* LoRaWAN spreading factor is hard coded (SP7)
* Only LoRaWAN ABP is supported. If you need OTAA, just implement it and make a pull request
