//#define DEBUG_SERIAL    1
//#define DEBUG_MQTT      1 
#define MODBUS_RATE     9600      // Modbus speed of Growatt, do not change
#define SERIAL_RATE     115200    // Serial speed for status info
#define MAX485_DE       5         // D1, DE pin on the TTL to RS485 converter
#define MAX485_RE_NEG   4         // D2, RE pin on the TTL to RS485 converter
#define MAX485_RX       14        // D5, RO pin on the TTL to RS485 converter
#define MAX485_TX       12        // D6, DI pin on the TTL to RS485 converter
#define SLAVE_ID        1         // Default slave ID of Growatt
#define STATUS_LED      2         // Status LED on the Wemos D1 mini (D4)
#define UPDATE_MODBUS   2         // 1: modbus device is read every second
#define UPDATE_STATUS   300        // 10: status mqtt message is sent every 10 seconds
#define WIFICHECK       500       // how often check lost wifi connection

// Update the below parameters for your project
// Also check NTP.h for some parameters as well
const char* ssid = "WIFI_SSID";           // Wifi SSID
const char* password = "your_password";    // Wifi password
const char* mqtt_server = "x.x.x.x";     // MQTT server
const char* mqtt_user = "mqtt_user";             // MQTT userid
const char* mqtt_password = "mqtt_password";         // MQTT password
const char* clientID = "mqtt_client_id";                // MQTT client ID
const char* topicRoot = "your/mqtt*topic";             // MQTT root topic for the device, keep / at the end
const char* OTAPath = "/update_path";                   // OTA Path url
const char* OTAUser = "OTA_user";       // OTA user id
const char* OTAPass = "OTA_pass";  // OTA user password


// Comment the entire second below for dynamic IP (including the define)
 #define FIXEDIP   1
IPAddress local_IP(192, 168, x, x);         // Set your Static IP address
IPAddress gateway(192, 168, x, x);          // Set your Gateway IP address
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, x, x);
