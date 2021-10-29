/**
 * UDAWA - Universal Digital Agriculture Watering Assistant
 * Function helper library for ESP32 based UDAWA multi-device firmware development
 * Licensed under aGPLv3
 * Researched and developed by PRITA Research Group & Narin Laboratory
 * prita.undiknas.ac.id | narin.co.id
**/

#ifndef libudawa_h
#define libudawa_h

#include "secret.h"
#include <esp32-hal-log.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Arduino.h>
#include <thingsboard.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define countof(a) (sizeof(a) / sizeof(a[0]))
#define DOCSIZE 1500
#define COMPILED __DATE__ " " __TIME__
#define LOG_REC_SIZE 30
#define LOG_REC_LENGTH 192

const char* configFile = "/cfg.json";
char logBuff[LOG_REC_LENGTH];
char _logRec[LOG_REC_SIZE][LOG_REC_LENGTH];
uint8_t _logRecIndex;

struct Config
{
  char hwid[16];
  char name[32];
  char model[16];
  char group[16];
  uint8_t logLev;

  char broker[128];
  uint16_t port;
  char wssid[64];
  char wpass[64];
  char accessToken[64];
  bool provSent;

  char provisionDeviceKey[24];
  char provisionDeviceSecret[24];

  uint8_t relayChannels[4];
};


void reboot();
char* getDeviceId();
void cbWifiOnConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void cbWiFiOnDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void cbWiFiOnLostIp(WiFiEvent_t event, WiFiEventInfo_t info);
void cbWiFiOnGotIp(WiFiEvent_t event, WiFiEventInfo_t info);
void configLoadFailSafe();
void configLoad();
void configSave();
void configReset();
bool loadFile(const char* filePath, char* buffer);
void processProvisionResponse(const Provision_Data &data);
void recordLog(uint8_t level, const char* fileName, int, const char* functionName);
void iotInit();
void startup();
void udawa();

static const char NARIN_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIGSjCCBDKgAwIBAgIJAMxU3KljbiooMA0GCSqGSIb3DQEBCwUAMIGwMQswCQYD
VQQGEwJJRDENMAsGA1UECAwEQmFsaTEQMA4GA1UEBwwHR2lhbnlhcjEhMB8GA1UE
CgwYQ1YuIE5hcmF5YW5hIEluc3RydW1lbnRzMSQwIgYDVQQLDBtOYXJpbiBDZXJ0
aWZpY2F0ZSBBdXRob3JpdHkxFjAUBgNVBAMMDU5hcmluIFJvb3QgQ0ExHzAdBgkq
hkiG9w0BCQEWEGNlcnRAbmFyaW4uY28uaWQwIBcNMjAwMTE2MDUyMjM1WhgPMjA1
MDAxMDgwNTIyMzVaMIGwMQswCQYDVQQGEwJJRDENMAsGA1UECAwEQmFsaTEQMA4G
A1UEBwwHR2lhbnlhcjEhMB8GA1UECgwYQ1YuIE5hcmF5YW5hIEluc3RydW1lbnRz
MSQwIgYDVQQLDBtOYXJpbiBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkxFjAUBgNVBAMM
DU5hcmluIFJvb3QgQ0ExHzAdBgkqhkiG9w0BCQEWEGNlcnRAbmFyaW4uY28uaWQw
ggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQC7SU2ahwCe1KktoaUEQLmr
E91S2UwaqGvcGksy9j08GnI1NU1MpqsVrPSxuLQRr7ww2IG9hzKN0rKIhkXUfBCJ
X8/K7bxEkLl2yfcJjql90/EdAjWClo3CURM1zIqzxggkZKmdEGrPV/WGkYchmxuT
QvYDoPVLScXhN7NtTfzd3x/zWwe4WHg4THpfqeyE6vCLoNeDKvF2GsP0xsYtows8
pTjKH9gh0kFi+aYoVxjbH8KB78ktWAOo1T2db3KUF4/gYweYk4b/vS1egdDm821/
6qC7XrsnaApyRm73RKtmhAzldx9D1YqdVbFIx5oRlEg3+uI7hv/YD6Icfhazw1ql
Su8U7g8Ax8OPVdjdJ41lgkFs+OpY4GnfzjIzhvQ+kRIPyDQQaLwCxFZJZIa2jAnB
R5GzSM+Py0d+oStELotd0O3kLC7z4eFdxfRuaaQzofn/aUT1K7NsbG8V7rC3lG9P
8Jc+SU7zP/XSqVjKFTzRnIQ6C4WwkdWwS7uN2FSQBmlIw4EHaYcoMbQCVN2DcGFE
iMH+8/kp99UBKCu2MKq1zM+W0n+dNJ65EdeygOw580EIdNY5DPbpQTeNMt1idXF2
8C9jMyInGt6ZgZ5IfjExfDDIb6WYl3KRmZxqXjWCMg1e5TwMrbeoeg3R6kFY3lG4
TReG7Zjp1PQ3vxMC4ZWcpQIDAQABo2MwYTAdBgNVHQ4EFgQU4rkKARnad9D4bdWs
t3Jo8KKTxHAwHwYDVR0jBBgwFoAU4rkKARnad9D4bdWst3Jo8KKTxHAwDwYDVR0T
AQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAYYwDQYJKoZIhvcNAQELBQADggIBAJAY
pl1fdOG8GpvTIDts/H1CZbT6bF85+FnAV6abL/X4dIx/sQpg9TASAicyMzqUtDSb
2WaD3tkDvFhu7/vzG62x0tcpGw99Bxy0pkSkO9J3KrrxxiK21810aZnxOLZFuvgJ
2O/jugBK8MdCemBpmX93imgXSLJSXJf5/yVcETXFGUmf5p7Ze3Wdi0AxFvjPe2yN
D2MFvp2dtJ9mFaGaCG9v5wjyVVZM+oTGI9JzfNURq//qYWX+Tz9HJVNVeuYvEUnb
LXe9FVZej1+RVsBut9eCyo4GWOqMgRWp/dyMKz3shHFec0pc0fluo7yQH82OonoM
ZzkqgKVmkP5LVW0WqrDKbPTmpsq3ISYJwe7Msnu5D47iUnuc22axPzOH7ZRXE+2n
1Vkig4iYxz2IFZCwO3Ei9LxDlaJh+juHNnS0ziosDrTw0c/VWjkV+XwhRhfNq+Cx
crjMwThsIrz+JXrTihppMvSQhJHjIB/KoiUsa63qVsv6JA+yeBwvthwJ4Kl2ioDg
rH2VNKMU9e/dsWRqfRdUxH29pyJHQFjlv8MXWlFrKgoyrOLN2wkO2RJdKm0hblZW
vh+RH1AiwshFKw9rxdUXJBGGVgn5F0Ie4alDI8ehelOpmrZgFYMOCpcFSpJ5vbXM
ny6l9/duT2POAsUN5IwHGDu8b2NT+vCUQRFVHY31
-----END CERTIFICATE-----
)EOF";

#endif