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





WiFiClientSecure ssl = WiFiClientSecure();
Config config;
ThingsBoard tbProvision(ssl);
ThingsBoard tb(ssl);
volatile bool provisionResponseProcessed = false;
const Provision_Callback provisionCallback = processProvisionResponse;
bool FLAG_IOT_INIT = 0;

void startup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  if(!SPIFFS.begin())
  {
    configReset();
    configLoadFailSafe();
    sprintf_P(logBuff, PSTR("Problem with file system. Failsafe config was loaded."));
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  else
  {
    configLoad();
  }

  WiFi.onEvent(cbWifiOnConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(cbWiFiOnDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  WiFi.onEvent(cbWiFiOnLostIp, SYSTEM_EVENT_STA_LOST_IP);
  WiFi.onEvent(cbWiFiOnGotIp, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.mode(WIFI_STA);
  WiFi.begin(config.wssid, config.wpass);
  WiFi.setHostname(config.name);
  WiFi.setAutoReconnect(true);

  ssl.setCACert(NARIN_CERT_CA);
}

void udawa() {
  // put your main code here, to run repeatedly:
  if (!provisionResponseProcessed) {
    tbProvision.loop();
  }
  else {
    tb.loop();
  }

  if(FLAG_IOT_INIT)
  {
    FLAG_IOT_INIT = 0;
    iotInit();
  }
}

void reboot()
{
  esp_task_wdt_init(1,true);
  esp_task_wdt_add(NULL);
  while(true);
}

char* getDeviceId()
{
  char* decodedString = new char[16];
  uint64_t chipid = ESP.getEfuseMac();
  sprintf(decodedString, "%04X%08X",(uint16_t)(chipid>>32), (uint32_t)chipid);
  return decodedString;
}

void iotInit()
{
  if(!config.provSent)
  {
    if(!tbProvision.connected())
    {
      sprintf_P(logBuff, PSTR("Starting provision initiation to %s:%d"),  config.broker, config.port);
      recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
      if(!tbProvision.connect(config.broker, "provision", config.port))
      {
        sprintf_P(logBuff, PSTR("Failed to connect to provisioning server: %s:%d"),  config.broker, config.port);
        recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
        return;
      }
      if(tbProvision.Provision_Subscribe(provisionCallback))
      {
        if(tbProvision.sendProvisionRequest(config.name, config.provisionDeviceKey, config.provisionDeviceSecret))
        {
          config.provSent = true;
          sprintf_P(logBuff, PSTR("Provision request was sent!"));
          recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
        }
      }
    }
  }
  else if(provisionResponseProcessed || config.accessToken)
  {
    if(!tb.connected())
    {
      sprintf_P(logBuff, PSTR("Connecting to broker %s:%d"), config.broker, config.port);
      recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
      if(!tb.connect(config.broker, config.accessToken, config.port))
      {
        sprintf_P(logBuff, PSTR("Failed to connect to IoT Broker %s"), config.broker);
        recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
        return;
      }
      else
      {
        sprintf_P(logBuff, PSTR("IoT Connected!"));
        recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
      }
    }
  }
}

void cbWifiOnConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  IPAddress ip = WiFi.localIP();
  char ipa[25];
  sprintf(ipa, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  sprintf_P(logBuff, PSTR("WiFi Connected to %s"), WiFi.SSID().c_str());
  recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
}

void cbWiFiOnDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  sprintf_P(logBuff, PSTR("WiFi (%s) Disconnected!"), WiFi.SSID().c_str());
  recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
}

void cbWiFiOnLostIp(WiFiEvent_t event, WiFiEventInfo_t info)
{
  sprintf_P(logBuff, PSTR("WiFi (%s) IP Lost!"), WiFi.SSID().c_str());
  recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
}

void cbWiFiOnGotIp(WiFiEvent_t event, WiFiEventInfo_t info)
{
  FLAG_IOT_INIT = 1;
}

void configReset()
{
  SPIFFS.remove(configFile);
  File file = SPIFFS.open(configFile, FILE_WRITE);
  if (!file) {
    file.close();
    return;
  }

  StaticJsonDocument<DOCSIZE> doc;

  char dv[16];
  sprintf(dv, "%s", getDeviceId());
  doc["name"] = "UDAWA" + String(dv);
  doc["model"] = "Actuator4Ch";
  doc["group"] = "PRITA";
  doc["broker"] = "prita.undiknas.ac.id";
  doc["port"] = 8883;
  doc["wssid"] = wssid;
  doc["wpass"] = wpass;
  doc["accessToken"] = accessToken;
  doc["provSent"] = false;
  doc["provisionDeviceKey"] = provisionDeviceKey;
  doc["provisionDeviceSecret"] = provisionDeviceSecret;
  doc["logLev"] = 5;

  serializeJson(doc, file);
  file.close();

  sprintf_P(logBuff, PSTR("Config hard reset:"));
  recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
  serializeJsonPretty(doc, Serial);
}

void configLoadFailSafe()
{
  char dv[16];
  sprintf(dv, "%s", getDeviceId());
  strlcpy(config.hwid, dv, sizeof(config.hwid));

  String name = "UDAWA" + String(dv);
  strlcpy(config.name, name.c_str(), sizeof(config.name));
  strlcpy(config.model, "Actuator4Ch", sizeof(config.model));
  strlcpy(config.group, "PRITA", sizeof(config.group));
  strlcpy(config.broker, "prita.undiknas.ac.id", sizeof(config.broker));
  strlcpy(config.wssid, wssid, sizeof(config.wssid));
  strlcpy(config.wpass, wpass, sizeof(config.wpass));
  strlcpy(config.accessToken, accessToken, sizeof(config.accessToken));
  config.provSent = false;
  config.port = 8883;
  strlcpy(config.provisionDeviceKey, provisionDeviceKey, sizeof(config.provisionDeviceKey));
  strlcpy(config.provisionDeviceSecret, provisionDeviceSecret, sizeof(config.provisionDeviceSecret));
  config.logLev = 5;
}

void configLoad()
{
  File file = SPIFFS.open(configFile);
  StaticJsonDocument<DOCSIZE> doc;
  DeserializationError error = deserializeJson(doc, file);

  if(error)
  {
    file.close();
    configReset();
    return;
  }
  else
  {
    char dv[16];
    sprintf(dv, "%s", getDeviceId());
    strlcpy(config.hwid, dv, sizeof(config.hwid));

    sprintf_P(logBuff, PSTR("Device ID: %s"), dv);
    recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));

    strlcpy(config.name, doc["name"].as<const char*>(), sizeof(config.name));
    strlcpy(config.model, doc["model"].as<const char*>(), sizeof(config.model));
    strlcpy(config.group, doc["group"].as<const char*>(), sizeof(config.group));
    strlcpy(config.broker, doc["broker"].as<const char*>(), sizeof(config.broker));
    strlcpy(config.wssid, doc["wssid"].as<const char*>(), sizeof(config.wssid));
    strlcpy(config.wpass, doc["wpass"].as<const char*>(), sizeof(config.wpass));
    strlcpy(config.accessToken, doc["accessToken"].as<const char*>(), sizeof(config.accessToken));
    config.provSent = doc["provSent"].as<bool>();
    config.port = doc["port"].as<uint16_t>() ? doc["port"].as<uint16_t>() : 8883;
    config.logLev = doc["logLev"].as<uint8_t>();
    strlcpy(config.provisionDeviceKey, doc["provisionDeviceKey"].as<const char*>(), sizeof(config.provisionDeviceKey));
    strlcpy(config.provisionDeviceSecret, doc["provisionDeviceSecret"].as<const char*>(), sizeof(config.provisionDeviceSecret));

    sprintf_P(logBuff, PSTR("Config loaded successfuly."));
    recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  file.close();
}

void configSave()
{
  SPIFFS.remove(configFile);
  File file = SPIFFS.open(configFile, FILE_WRITE);
  if (!file)
  {
    file.close();
    return;
  }

  StaticJsonDocument<DOCSIZE> doc;
  doc["name"] = config.name;
  doc["model"] = config.model;
  doc["group"] = config.group;
  doc["broker"] = config.broker;
  doc["port"]  = config.port;
  doc["wssid"] = config.wssid;
  doc["wpass"] = config.wpass;
  doc["accessToken"] = config.accessToken;
  doc["provSent"] = config.provSent;
  doc["provisionDeviceKey"] = config.provisionDeviceKey;
  doc["provisionDeviceSecret"] = config.provisionDeviceSecret;
  doc["logLev"] = config.logLev;

  serializeJson(doc, file);
  file.close();

}

bool loadFile(const char* filePath, char *buffer)
{
  File file = SPIFFS.open(filePath);
  if(!file || file.isDirectory()){
      file.close();
      return false;
  }
  uint16_t i = 0;
  while(file.available()){
    buffer[i] = file.read();
    i++;
  }
  buffer[i] = '\0';
  file.close();
  return true;
}

void processProvisionResponse(const Provision_Data &data)
{
  sprintf_P(logBuff, PSTR("Received device provision response"));
  recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
  int jsonSize = measureJson(data) + 1;
  char buffer[jsonSize];
  serializeJson(data, buffer, jsonSize);

  if (strncmp(data["status"], "SUCCESS", strlen("SUCCESS")) != 0)
  {
    sprintf_P(logBuff, PSTR("Provision response contains the error: %s"), data["errorMsg"].as<const char*>());
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
    provisionResponseProcessed = true;
    return;
  }
  else
  {
    sprintf_P(logBuff, PSTR("Provision response credential type: %s"), data["credentialsType"].as<const char*>());
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  if (strncmp(data["credentialsType"], "ACCESS_TOKEN", strlen("ACCESS_TOKEN")) == 0)
  {
    strlcpy(config.accessToken, data["credentialsValue"].as<String>().c_str(), sizeof(config.accessToken));
    configSave();
  }
  if (strncmp(data["credentialsType"], "MQTT_BASIC", strlen("MQTT_BASIC")) == 0)
  {
    /*JsonObject credentials_value = data["credentialsValue"].as<JsonObject>();
    credentials.client_id = credentials_value["clientId"].as<String>();
    credentials.username = credentials_value["userName"].as<String>();
    credentials.password = credentials_value["password"].as<String>();
    */
  }
  if (tbProvision.connected()) {
    tbProvision.disconnect();
  }
  provisionResponseProcessed = true;
}

void recordLog(uint8_t level, const char* fileName, int lineNumber, const char* functionName)
{
  if(_logRecIndex == LOG_REC_SIZE)
  {
    _logRecIndex = 0;
  }
  const char *levels;
  if(level == 5){levels = "D";}
  else if(level == 4){levels = "I";}
  else if(level == 3){levels = "W";}
  else if(level == 2){levels = "C";}
  else if(level == 1){levels = "E";}
  else{levels = "X";}
  sprintf_P(_logRec[_logRecIndex], PSTR("[%s][%s:%d] %s: %s"), levels, fileName, lineNumber, functionName, logBuff);
  if(level <= config.logLev)
  {
    Serial.println(_logRec[_logRecIndex]);
  }
  _logRecIndex++;
}

#endif