/**
 * UDAWA - Universal Digital Agriculture Watering Assistant
 * Firmware for Vanilla UDAWA Board (starter firmware)
 * Licensed under aGPLv3
 * Researched and developed by PRITA Research Group & Narin Laboratory
 * prita.undiknas.ac.id | narin.co.id
**/
#ifndef main_h
#define main_h

#define DOCSIZE 1500
#define ASYNC_TCP_SSL_ENABLED true
#ifdef ASYNC_TCP_SSL_ENABLED
    #define MQTT_SERVER_FINGERPRINT {0xa9, 0xf5, 0x35, 0x77, 0x7f, 0x7d, 0xcd, 0x3e, 0x04, 0x94, 0x29, 0x4e, 0xc7, 0x5e, 0x44, 0xb1, 0x63, 0xb6, 0x84, 0xa1};
#endif

#include <libudawa.h>
#include <TaskManagerIO.h>

#define CURRENT_FIRMWARE_TITLE "UDAWA-Vanilla"
#define CURRENT_FIRMWARE_VERSION "0.0.1"

const char* settingsPath = "/settings.json";

struct Settings
{
    unsigned long lastUpdated;
    bool fTeleDev;
    unsigned long publishInterval;
    unsigned long myTaskInterval;
};

callbackResponse processSaveConfig(const callbackData &data);
callbackResponse processSaveSettings(const callbackData &data);
callbackResponse processSharedAttributesUpdate(const callbackData &data);
callbackResponse processSyncClientAttributes(const callbackData &data);
callbackResponse processReboot(const callbackData &data);

void loadSettings();
void saveSettings();
void syncClientAttributes();
void publishDeviceTelemetry();
void myTask();

#endif