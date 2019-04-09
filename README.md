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

# Gettings started

1. Clone this repository
2. Copy `settings-example.h` to `settings.h`
3. Check that `NWKSKEY`, `APPSKEY`, `DEVEUI` and `DEVADDR` have correct values in `settings.h`.
4. Open AQLoRaBurk.ino, compile and upload the code to the board

## LoRaWAN

Check very carefully all LoRaWAN network related values in `settings.h`.

## Wiring

Check pin numbers from `AQLoRaBurk.ino`:

```
// I2C settings
#define SDA 21           // Connect to BMEx80 SDA
#define SCL 22           // Connect to BMEx80 SCL

#define SDS011_RXPIN 39  // Connect to SDS011 TX
#define SDS011_TXPIN 36  // Connect to SDS011 RX
```
(TODO: move to settings.h)


# Known issues
* LoRaWAN spreading factor is hard coded (SP7)
* Only LoRaWAN ABP is supported. If you need OTAA, just implement it and make a pull request
* I2C (for BME) and serial (for SDS011) pins are hard coded

# TODO
* Move all `#define` lines from `_Sensors.ino` to `settings.h`
* Move all `#define` lines from `AQLoRaBurk.ino` to `settings.h`

