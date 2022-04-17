/**
 * UDAWA - Universal Digital Agriculture Watering Assistant
 * Function helper library for ESP32 based UDAWA multi-device firmware development
 * Licensed under aGPLv3
 * Researched and developed by PRITA Research Group & Narin Laboratory
 * prita.undiknas.ac.id | narin.co.id
**/

#ifndef libudawa_h
#define libudawa_h

#include <secret.h>
#include <esp32-hal-log.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <ArduinoOTA.h>
#include <thingsboard.h>
#include <TaskManagerIO.h>

#define countof(a) (sizeof(a) / sizeof(a[0]))
#define COMPILED __DATE__ " " __TIME__
#define LOG_REC_SIZE 10
#define LOG_REC_LENGTH 192
#define PIN_RXD2 16
#define PIN_TXD2 17
#define WIFI_FALLBACK_COUNTER 5
#ifndef DOCSIZE
  #define DOCSIZE 1024
#endif

const char* configFile = "/cfg.json";
const char* configFileCoMCU = "/comcu.json";
char logBuff[LOG_REC_LENGTH];
char _logRec[LOG_REC_SIZE][LOG_REC_LENGTH];
uint8_t _logRecIndex;
bool FLAG_IOT_SUBSCRIBE = false;
bool FLAG_IOT_INIT = false;
bool FLAG_OTA_UPDATE_INIT = false;
uint8_t WIFI_RECONNECT_ATTEMPT = 0;
bool WIFI_IS_DEFAULT = false;

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
  char dssid[64];
  char dpass[64];
  char upass[64];
  char accessToken[64];
  bool provSent;

  char provisionDeviceKey[24];
  char provisionDeviceSecret[24];

  uint8_t relayChannels[4];
};

struct ConfigCoMCU
{
  bool fPanic;
  float pEcKcoe;
  float pEcTcoe;
  float pEcVin;
  float pEcPpm;
  uint16_t pEcR1;
  uint16_t pEcRa;

  uint16_t bfreq;
  bool fBuzz;

  uint8_t pinBuzzer;
  uint8_t pinLedR;
  uint8_t pinLedG;
  uint8_t pinLedB;
  uint8_t pinEcPower;
  uint8_t pinEcGnd;
  uint8_t pinEcData;
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
void configCoMCULoadFailSafe();
void configCoMCULoad();
void configCoMCUSave();
void configCoMCUReset();
bool loadFile(const char* filePath, char* buffer);
callbackResponse processProvisionResponse(const callbackData &data);
void recordLog(uint8_t level, const char* fileName, int, const char* functionName);
void iotSendLog();
void iotInit();
void startup();
void networkInit();
void udawa();
void otaUpdateInit();
void serialWriteToCoMcu(StaticJsonDocument<DOCSIZE> &doc, bool isRpc);
void serialReadFromCoMcu(StaticJsonDocument<DOCSIZE> &doc);
void syncConfigCoMCU();
void readSettings(StaticJsonDocument<DOCSIZE> &doc,const char* path);
void writeSettings(StaticJsonDocument<DOCSIZE> &doc, const char* path);
void setCoMCUPin(uint8_t pin, char type, bool mode, uint16_t aval, bool state);


WiFiClientSecure ssl = WiFiClientSecure();
Config config;
ConfigCoMCU configcomcu;
ThingsBoardSized<DOCSIZE, 64> tb(ssl);
volatile bool provisionResponseProcessed = false;

void startup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  #ifdef USE_SERIAL2
    Serial2.begin(115200, SERIAL_8N1, PIN_RXD2, PIN_TXD2);
  #endif

  config.logLev = 5;
  if(!SPIFFS.begin(true))
  {
    //configReset();
    configLoadFailSafe();
    sprintf_P(logBuff, PSTR("Problem with file system. Failsafe config was loaded."));
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  else
  {
    sprintf_P(logBuff, PSTR("Loading config..."));
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
    configLoad();
  }
}

void networkInit()
{
  WiFi.onEvent(cbWifiOnConnected, SYSTEM_EVENT_STA_CONNECTED);
  WiFi.onEvent(cbWiFiOnDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  WiFi.onEvent(cbWiFiOnLostIp, SYSTEM_EVENT_STA_LOST_IP);
  WiFi.onEvent(cbWiFiOnGotIp, SYSTEM_EVENT_STA_GOT_IP);
  WiFi.mode(WIFI_STA);
  if(!config.wssid || *config.wssid == 0x00 || strlen(config.wssid) > 32)
  {
    configLoadFailSafe();
    sprintf_P(logBuff, PSTR("SSID too long or missing! Failsafe config was loaded."));
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  WiFi.begin(config.wssid, config.wpass);
  WiFi.setHostname(config.name);
  WiFi.setAutoReconnect(true);

  ssl.setCACert(CA_CERT);

  taskManager.scheduleFixedRate(10000, [] {
    if(WiFi.status() == WL_CONNECTED && !tb.connected())
    {
      iotInit();
    }
  });
}

void udawa() {
  taskManager.runLoop();
  ArduinoOTA.handle();

  tb.loop();

  if(FLAG_OTA_UPDATE_INIT)
  {
    FLAG_OTA_UPDATE_INIT = 0;
    otaUpdateInit();
  }

  if(FLAG_IOT_INIT)
  {
    FLAG_IOT_INIT = 0;
    iotInit();
  }
}

void reboot()
{
  sprintf_P(logBuff, PSTR("Device rebooting..."));
  recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
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

void otaUpdateInit()
{
  ArduinoOTA.setHostname(config.name);
  ArduinoOTA.setPasswordHash(config.upass);
  ArduinoOTA.begin();

  ArduinoOTA
    .onStart([]()
    {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
          type = "sketch";
      else // U_SPIFFS
          type = "filesystem";
    })
    .onEnd([]()
    {
      reboot();
    })
    .onProgress([](unsigned int progress, unsigned int total)
    {

    })
    .onError([](ota_error_t error)
    {
      reboot();
    }
  );
}

void iotInit()
{
  int freeHeap = ESP.getFreeHeap();
  sprintf_P(logBuff, PSTR("Initializing IoT, available memory: %d"), freeHeap);
  recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
  if(freeHeap < 92000)
  {
    sprintf_P(logBuff, PSTR("Unable to init IoT, insufficient memory: %d"), freeHeap);
    recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
    return;
  }
  if(!config.provSent)
  {
    if(!tb.connected())
    {
      sprintf_P(logBuff, PSTR("Starting provision initiation to %s:%d"),  config.broker, config.port);
      recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
      if(tb.connect(config.broker, "provision", config.port, config.name))
      {
        sprintf_P(logBuff, PSTR("Connected to provisioning server: %s:%d"),  config.broker, config.port);
        recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));

        GenericCallback cb[2] = {
          { "provisionResponse", processProvisionResponse },
          { "provisionResponse", processProvisionResponse }
        };
        if(tb.callbackSubscribe(cb, 2))
        {
          if(tb.sendProvisionRequest(config.name, config.provisionDeviceKey, config.provisionDeviceSecret))
          {
            config.provSent = true;
            sprintf_P(logBuff, PSTR("Provision request was sent!"));
            recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
          }
        }
      }
      else
      {
        sprintf_P(logBuff, PSTR("Failed to connect to provisioning server: %s:%d"),  config.broker, config.port);
        recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
        return;
      }
    }
  }
  else if(provisionResponseProcessed || config.accessToken)
  {
    if(!tb.connected())
    {
      sprintf_P(logBuff, PSTR("Connecting to broker %s:%d"), config.broker, config.port);
      recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
      if(!tb.connect(config.broker, config.accessToken, config.port, config.name))
      {
        sprintf_P(logBuff, PSTR("Failed to connect to IoT Broker %s"), config.broker);
        recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
        return;
      }
      else
      {
        iotSendLog();
        sprintf_P(logBuff, PSTR("IoT Connected!"));
        recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
        FLAG_IOT_SUBSCRIBE = true;
      }
    }
  }
}

void cbWifiOnConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  sprintf_P(logBuff, PSTR("WiFi Connected to %s"), WiFi.SSID().c_str());
  recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
  WIFI_RECONNECT_ATTEMPT = 0;
}

void cbWiFiOnDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  WIFI_RECONNECT_ATTEMPT += 1;
  if(WIFI_RECONNECT_ATTEMPT >= WIFI_FALLBACK_COUNTER)
  {
    if(!WIFI_IS_DEFAULT)
    {
      sprintf_P(logBuff, PSTR("WiFi (%s) Disconnected! Attempt: %d/%d"), config.dssid, WIFI_RECONNECT_ATTEMPT, WIFI_FALLBACK_COUNTER);
      recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
      WiFi.begin(config.dssid, config.dpass);
      WIFI_IS_DEFAULT = true;
    }
    else
    {
      sprintf_P(logBuff, PSTR("WiFi (%s) Disconnected! Attempt: %d/%d"), config.wssid, WIFI_RECONNECT_ATTEMPT, WIFI_FALLBACK_COUNTER);
      recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
      WiFi.begin(config.wssid, config.wpass);
      WIFI_IS_DEFAULT = false;
    }
    WIFI_RECONNECT_ATTEMPT = 0;
  }
  else
  {
    WiFi.reconnect();
  }
}

void cbWiFiOnLostIp(WiFiEvent_t event, WiFiEventInfo_t info)
{
  sprintf_P(logBuff, PSTR("WiFi (%s) IP Lost!"), WiFi.SSID().c_str());
  recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
  WiFi.reconnect();
}

void cbWiFiOnGotIp(WiFiEvent_t event, WiFiEventInfo_t info)
{
  FLAG_IOT_INIT = 1;
  FLAG_OTA_UPDATE_INIT = 1;
}

void configReset()
{
  bool formatted = SPIFFS.format();
  if(formatted)
  {
    sprintf_P(logBuff, PSTR("SPIFFS formatting success."));
    recordLog(3, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  else
  {
    sprintf_P(logBuff, PSTR("SPIFFS formatting failed."));
    recordLog(3, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  File file;
  file = SPIFFS.open(configFile, FILE_WRITE);
  if (!file) {
    sprintf_P(logBuff, PSTR("Failed to create config file. Config reset is cancelled."));
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
    file.close();
    return;
  }

  StaticJsonDocument<DOCSIZE> doc;

  char dv[16];
  sprintf(dv, "%s", getDeviceId());
  doc["name"] = "UDAWA" + String(dv);
  doc["model"] = "Generic";
  doc["group"] = "UDAWA";
  doc["broker"] = broker;
  doc["port"] = port;
  doc["wssid"] = wssid;
  doc["wpass"] = wpass;
  doc["dssid"] = dssid;
  doc["dpass"] = dpass;
  doc["upass"] = upass;
  doc["accessToken"] = accessToken;
  doc["provSent"] = false;
  doc["provisionDeviceKey"] = provisionDeviceKey;
  doc["provisionDeviceSecret"] = provisionDeviceSecret;
  doc["logLev"] = 5;

  size_t size = serializeJson(doc, file);
  file.close();

  sprintf_P(logBuff, PSTR("Verifiying resetted config file (size: %d) is written successfully..."), size);
  recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
  file = SPIFFS.open(configFile, FILE_READ);
  if (!file)
  {
    sprintf_P(logBuff, PSTR("Failed to open the config file!"));
    recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  else
  {
    sprintf_P(logBuff, PSTR("New config file opened, size: %d"), file.size());
    recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));

    if(file.size() < 1)
    {
      sprintf_P(logBuff, PSTR("Config file size is abnormal: %d, trying to rewrite..."), file.size());
      recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));

      size_t size = serializeJson(doc, file);
      sprintf_P(logBuff, PSTR("Writing: %d of data, file size: %d"), size, file.size());
      recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
    }
    else
    {
      sprintf_P(logBuff, PSTR("Config file size is normal: %d, trying to reboot..."), file.size());
      recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
      file.close();
      reboot();
    }
  }
  file.close();
}

void configLoadFailSafe()
{
  char dv[16];
  sprintf(dv, "%s", getDeviceId());
  strlcpy(config.hwid, dv, sizeof(config.hwid));

  String name = "UDAWA" + String(dv);
  strlcpy(config.name, name.c_str(), sizeof(config.name));
  strlcpy(config.model, "Generic", sizeof(config.model));
  strlcpy(config.group, "UDAWA", sizeof(config.group));
  strlcpy(config.broker, broker, sizeof(config.broker));
  strlcpy(config.wssid, wssid, sizeof(config.wssid));
  strlcpy(config.wpass, wpass, sizeof(config.wpass));
  strlcpy(config.dssid, dssid, sizeof(config.dssid));
  strlcpy(config.dpass, dpass, sizeof(config.dpass));
  strlcpy(config.upass, upass, sizeof(config.upass));
  strlcpy(config.accessToken, accessToken, sizeof(config.accessToken));
  config.provSent = false;
  config.port = port;
  strlcpy(config.provisionDeviceKey, provisionDeviceKey, sizeof(config.provisionDeviceKey));
  strlcpy(config.provisionDeviceSecret, provisionDeviceSecret, sizeof(config.provisionDeviceSecret));
  config.logLev = 5;
}

void configLoad()
{
  File file = SPIFFS.open(configFile, FILE_READ);
  sprintf_P(logBuff, PSTR("Loading config file."));
  recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
  if(file.size() > 1)
  {
    sprintf_P(logBuff, PSTR("Config file size is normal: %d, trying to fit it in %d docsize."), file.size(), DOCSIZE);
    recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  else
  {
    file.close();
    sprintf_P(logBuff, PSTR("Config file size is abnormal: %d. Closing file and trying to reset..."), file.size());
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
    configReset();
    return;
  }

  StaticJsonDocument<DOCSIZE> doc;
  DeserializationError error = deserializeJson(doc, file);

  if(error)
  {
    sprintf_P(logBuff, PSTR("Failed to load config file! (%s - %s - %d). Falling back to failsafe."), configFile, error.c_str(), file.size());
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
    file.close();
    configLoadFailSafe();
    return;
  }
  else
  {
    char dv[16];
    sprintf(dv, "%s", getDeviceId());
    strlcpy(config.hwid, dv, sizeof(config.hwid));

    sprintf_P(logBuff, PSTR("Device ID: %s"), dv);
    recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));


    String name = "UDAWA" + String(dv);
    strlcpy(config.name, name.c_str(), sizeof(config.name));
    //strlcpy(config.name, doc["name"].as<const char*>(), sizeof(config.name));
    strlcpy(config.model, doc["model"].as<const char*>(), sizeof(config.model));
    strlcpy(config.group, doc["group"].as<const char*>(), sizeof(config.group));
    strlcpy(config.broker, doc["broker"].as<const char*>(), sizeof(config.broker));
    strlcpy(config.wssid, doc["wssid"].as<const char*>(), sizeof(config.wssid));
    strlcpy(config.wpass, doc["wpass"].as<const char*>(), sizeof(config.wpass));
    strlcpy(config.dssid, doc["dssid"].as<const char*>(), sizeof(config.dssid));
    strlcpy(config.dpass, doc["dpass"].as<const char*>(), sizeof(config.dpass));
    strlcpy(config.upass, doc["upass"].as<const char*>(), sizeof(config.upass));
    strlcpy(config.accessToken, doc["accessToken"].as<const char*>(), sizeof(config.accessToken));
    config.provSent = doc["provSent"].as<bool>();
    config.port = doc["port"].as<uint16_t>() ? doc["port"].as<uint16_t>() : port;
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
  if(!SPIFFS.remove(configFile))
  {
    sprintf_P(logBuff, PSTR("Failed to delete the old configFile: %s"), configFile);
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
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
  doc["dssid"] = config.dssid;
  doc["dpass"] = config.dpass;
  doc["upass"] = config.upass;
  doc["accessToken"] = config.accessToken;
  doc["provSent"] = config.provSent;
  doc["provisionDeviceKey"] = config.provisionDeviceKey;
  doc["provisionDeviceSecret"] = config.provisionDeviceSecret;
  doc["logLev"] = config.logLev;

  serializeJson(doc, file);
  file.close();
}


void configCoMCUReset()
{
  SPIFFS.remove(configFileCoMCU);
  File file = SPIFFS.open(configFileCoMCU, FILE_WRITE);
  if (!file) {
    file.close();
    return;
  }

  StaticJsonDocument<DOCSIZE> doc;

  doc["fPanic"] = false;
  doc["pEcKcoe"] = 2.9;
  doc["pEcTcoe"] = 0.019;
  doc["pEcVin"] = 4.54;
  doc["pEcPpm"] = 0.5;
  doc["pEcR1"] = 1000;
  doc["pEcRa"] =  25;

  doc["bfreq"] = 1600;
  doc["fBuzz"] = 1;

  doc["pinBuzzer"] = 3;
  doc["pinLedR"] = 9;
  doc["pinLedG"] = 10;
  doc["pinLedB"] = 11;
  doc["pinEcPower"] = 15;
  doc["pinEcGnd"] = 16;
  doc["pinEcData"] = 14;

  serializeJson(doc, file);
  file.close();

  sprintf_P(logBuff, PSTR("ConfigCoMCU hard reset:"));
  recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
  serializeJsonPretty(doc, Serial);
}

void configCoMCULoad()
{
  File file = SPIFFS.open(configFileCoMCU);
  StaticJsonDocument<DOCSIZE> doc;
  DeserializationError error = deserializeJson(doc, file);

  if(error)
  {
    file.close();
    configCoMCUReset();
    return;
  }
  else
  {

    configcomcu.fPanic = doc["fPanic"].as<bool>();
    configcomcu.pEcKcoe = doc["pEcKcoe"].as<float>();
    configcomcu.pEcTcoe = doc["pEcTcoe"].as<float>();
    configcomcu.pEcVin = doc["pEcVin"].as<float>();
    configcomcu.pEcPpm = doc["pEcPpm"].as<float>();
    configcomcu.pEcR1 = doc["pEcR1"].as<uint16_t>();
    configcomcu.pEcRa =  doc["pEcRa"].as<uint16_t>();

    configcomcu.bfreq = doc["bfreq"].as<uint16_t>();
    configcomcu.fBuzz = doc["fBuzz"].as<bool>();

    configcomcu.pinBuzzer = doc["pinBuzzer"].as<uint8_t>();
    configcomcu.pinLedR = doc["pinLedR"].as<uint8_t>();
    configcomcu.pinLedG = doc["pinLedG"].as<uint8_t>();
    configcomcu.pinLedB = doc["pinLedB"].as<uint8_t>();
    configcomcu.pinEcPower = doc["pinEcPower"].as<uint8_t>();
    configcomcu.pinEcGnd = doc["pinEcGnd"].as<uint8_t>();
    configcomcu.pinEcData = doc["pinEcData"].as<uint8_t>();

    sprintf_P(logBuff, PSTR("ConfigCoMCU loaded successfuly."));
    recordLog(4, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  file.close();
}

void configCoMCUSave()
{
  if(!SPIFFS.remove(configFileCoMCU))
  {
    sprintf_P(logBuff, PSTR("Failed to delete the old configFileCoMCU: %s"), configFileCoMCU);
    recordLog(1, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  File file = SPIFFS.open(configFileCoMCU, FILE_WRITE);
  if (!file)
  {
    file.close();
    return;
  }

  StaticJsonDocument<DOCSIZE> doc;

  doc["fPanic"] = configcomcu.fPanic;
  doc["pEcKcoe"] = configcomcu.pEcKcoe;
  doc["pEcTcoe"] = configcomcu.pEcTcoe;
  doc["pEcVin"] = configcomcu.pEcVin;
  doc["pEcPpm"] = configcomcu.pEcPpm;
  doc["pEcR1"] = configcomcu.pEcR1;
  doc["pEcRa"] = configcomcu.pEcRa;

  doc["bfreq"] = configcomcu.bfreq;
  doc["fBuzz"] = configcomcu.fBuzz;

  doc["pinBuzzer"] = configcomcu.pinBuzzer;
  doc["pinLedR"] = configcomcu.pinLedR;
  doc["pinLedG"] = configcomcu.pinLedG;
  doc["pinLedB"] = configcomcu.pinLedB;
  doc["pinEcPower"] = configcomcu.pinEcPower;
  doc["pinEcGnd"] = configcomcu.pinEcGnd;
  doc["pinEcData"] = configcomcu.pinEcData;

  serializeJson(doc, file);
  file.close();
}

void syncConfigCoMCU()
{
  configCoMCULoad();

  StaticJsonDocument<DOCSIZE> doc;
  doc["fPanic"] = configcomcu.fPanic;
  doc["pEcKcoe"] = configcomcu.pEcKcoe;
  doc["pEcTcoe"] = configcomcu.pEcTcoe;
  doc["pEcVin"] = configcomcu.pEcVin;
  doc["pEcPpm"] = configcomcu.pEcPpm;
  doc["pEcR1"] = configcomcu.pEcR1;
  doc["pEcRa"] = configcomcu.pEcRa;

  doc["bfreq"] = configcomcu.bfreq;
  doc["fBuzz"] = configcomcu.fBuzz;

  doc["pinBuzzer"] = configcomcu.pinBuzzer;
  doc["pinLedR"] = configcomcu.pinLedR;
  doc["pinLedG"] = configcomcu.pinLedG;
  doc["pinLedB"] = configcomcu.pinLedB;
  doc["pinEcPower"] = configcomcu.pinEcPower;
  doc["pinEcGnd"] = configcomcu.pinEcGnd;
  doc["pinEcData"] = configcomcu.pinEcData;

  doc["method"] = "setConfigCoMCU";
  serialWriteToCoMcu(doc, 0);
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

callbackResponse processProvisionResponse(const callbackData &data)
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
    return callbackResponse("provisionResponse", 1);
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
  provisionResponseProcessed = true;
  return callbackResponse("provisionResponse", 1);
  reboot();
}

void recordLog(uint8_t level, const char* fileName, int lineNumber, const char* functionName)
{
  if(level > config.logLev)
  {
    return;
  }

  const char *levels;
  if(level == 5){levels = "D";}
  else if(level == 4){levels = "I";}
  else if(level == 3){levels = "W";}
  else if(level == 2){levels = "C";}
  else if(level == 1){levels = "E";}
  else{levels = "X";}

  char formattedLog[LOG_REC_LENGTH];
  sprintf_P(formattedLog, PSTR("[%s][%s:%d] %s: %s"), levels, fileName, lineNumber, functionName, logBuff);
  if(tb.connected())
  {
    StaticJsonDocument<DOCSIZE> doc;
    doc["log"] = formattedLog;
    tb.sendTelemetryDoc(doc);
    doc.clear();
  }
  else
  {
    if(_logRecIndex == LOG_REC_SIZE)
    {
      _logRecIndex = 0;
    }
    sprintf_P(_logRec[_logRecIndex], PSTR("[%s][%s:%d] %s: %s"), levels, fileName, lineNumber, functionName, logBuff);
    _logRecIndex++;
  }
  Serial.println(formattedLog);
}

void iotSendLog()
{
  StaticJsonDocument<DOCSIZE> doc;
  for(uint8_t i = 0; i < LOG_REC_SIZE; i++)
  {
    if(_logRec[i][0] != 0)
    {
      doc["log"] = _logRec[i][0];
      tb.sendTelemetryDoc(doc);
      _logRec[i][0] = 0;
    }
  }
  doc.clear();
}

void serialWriteToCoMcu(StaticJsonDocument<DOCSIZE> &doc, bool isRpc)
{
  if(false)
  {
    StringPrint stream;
    serializeJson(doc, stream);
    String result = stream.str();
    sprintf_P(logBuff, PSTR("%s"), result.c_str());
    recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
  }
  serializeJson(doc, Serial2);
  serializeJsonPretty(doc, Serial);
  if(isRpc)
  {
    delay(50);
    serialReadFromCoMcu(doc);
  }
}

void serialReadFromCoMcu(StaticJsonDocument<DOCSIZE> &doc)
{
  StringPrint stream;
  String result;
  ReadLoggingStream loggingStream(Serial2, stream);
  DeserializationError err = deserializeJson(doc, loggingStream);
  result = stream.str();
  if (err == DeserializationError::Ok)
  {
    if(true){
      sprintf_P(logBuff, PSTR("%s"), result.c_str());
      recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
    }
  }
  else
  {
    sprintf_P(logBuff, PSTR("Serial2CoMCU DeserializeJson() returned: %s, content: %s"), err.c_str(), result.c_str());
    recordLog(5, PSTR(__FILE__), __LINE__, PSTR(__func__));
    return;
  }
}


void readSettings(StaticJsonDocument<DOCSIZE> &doc, const char* path)
{
  File file = SPIFFS.open(path);
  DeserializationError error = deserializeJson(doc, file);

  if(error)
  {
    file.close();
    return;
  }
  file.close();
}

void writeSettings(StaticJsonDocument<DOCSIZE> &doc, const char* path)
{
  SPIFFS.remove(path);
  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file)
  {
    file.close();
    return;
  }
  serializeJson(doc, file);
  file.close();
}

void setCoMCUPin(uint8_t pin, char type, bool mode, uint16_t aval, bool state)
{
  StaticJsonDocument<DOCSIZE> doc;
  JsonObject params = doc.createNestedObject("params");
  doc["method"] = "setPin";
  params["pin"] = pin;
  params["mode"] = mode;
  params["type"] = type;
  params["state"] = state;
  params["aval"] = aval;
  serialWriteToCoMcu(doc, false);
}

#endif