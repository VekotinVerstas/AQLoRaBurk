/*
In python shell create keys with this snippet (replace content in lines with your keys):

lines = '''
1234567890abcdef1234567890abcdef
11223344556677889900aabbccddeeff
9900aabbccddeeff
'''.split()
keys = ['NWKSKEY[16]', 'APPSKEY[16]', 'DEVEUI[8]']
conf = ['static const u4_t DEVADDR = 0x{}; // NOTE: last 8 characters from DEVEUI'.format(lines[2][-8:])]
for k in range(0, len(keys)) :
  code = ', '.join(['0x'+lines[k][i:i+2] for i in range(0, len(lines[k]), 2)])
  conf.append("static const PROGMEM u1_t {} = {{ {} }};".format(keys[k], code))

print('\n'.join(conf))

*/


#define BOXNUM 0x00

// uint8_t version = 1;
// char s_id[16];
const unsigned TX_INTERVAL = 1 * 60;  // LoRa send interval in seconds.

// #define PRINT_KEYS  // Uncomment if you want to print the keys below to the Serial at boot time
static PROGMEM u1_t NWKSKEY[16] = { 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef };
static PROGMEM u1_t APPSKEY[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
static PROGMEM u1_t DEVEUI[8] = { 0x99, 0x00, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, BOXNUM };
static u4_t DEVADDR = 0xccddee00 | BOXNUM; // NOTE: last 8 characters from DEVEUI

// I2C settings
#define SDA 21
#define SCL 22

// Software serial settings for SDS011
#define SDS011_RXPIN 39  // Connect to SDS011 TX
#define SDS011_TXPIN 36  // Connect to SDS011 RX

// BME680 heating time. I wouldn't touch this setting without good reasoning
#define BME680_HEATING_TIME 150 // milliseconds

// How often to read sensors
#define BME280_READ_DELAY 1000   // 1000 ms is too short delay, since we are sending the data once in a minute anyway
#define BME680_READ_DELAY 30000  // Do not use to low value, since heating gas sensor may affect temp and RH values!
#define SDS011_READ_DELAY 1000
