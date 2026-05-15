#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

// ThingsBoard config
#define THINGSBOARD_ENABLE_DEBUG
#define USING_HTTPS // true // using HTTP protocol
#define ENCRYPTED true // TLS
#define THINGSBOARD_ENABLE_PSRAM 1
#define THINGSBOARD_ENABLE_DYNAMIC 1
const char* TOKEN = "";
const char* THINGSBOARD_SERVER = "";

// STATES
// states
#define STATE_STORAGE_READY 
#define STATE_FILE_READY 0x40
#define STATE_MEMS_READY 0x20

// WIFI CONFIG
#define WIFI_SSID "wifi_name"
#define WIFI_PASSWORD "123123123"

#endif // CONFIG_H_INCLUDED