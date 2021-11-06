/**
 * UDAWA - Universal Digital Agriculture Watering Assistant
 * Function helper library for ATMega328 based Co-MCU firmware development
 * Licensed under aGPLv3
 * Researched and developed by PRITA Research Group & Narin Laboratory
 * prita.undiknas.ac.id | narin.co.id
**/
#ifndef libudawaatmega328_h
#define libudawaatmega328_h

#include "Arduino.h"
#include <ArduinoJson.h>
#include <ArduinoLog.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DOCSIZE 512
#define countof(a) (sizeof(a) / sizeof(a[0]))
#define COMPILED __DATE__ " " __TIME__
#define BOARD "nanoatmega328new"
#define ONE_WIRE_BUS 2

struct Led
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  bool isBlink;
  uint16_t blinkCount;
  uint16_t blinkDelay;
  uint32_t lastRun;
  uint8_t lastState = 1;
  uint8_t off = 255;
  uint8_t on = 0;
};

struct Buzzer
{
  uint16_t beepCount;
  uint16_t beepDelay;
  uint32_t lastRun;
  bool lastState;
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

class libudawaatmega328
{
  public:
    libudawaatmega328();
    libudawaatmega328(const char *configMainFile, const char *configRoutineFile);
    void begin();
    void execute();
    int getFreeHeap(uint16_t min,uint16_t max);
    int getFreeHeap();
    void getWaterEC(StaticJsonDocument<DOCSIZE> &doc);
    void getWaterTemp(StaticJsonDocument<DOCSIZE> &doc);
    float readWaterTemp();
    void setConfigCoMCU(StaticJsonDocument<DOCSIZE> &doc);
    void setPin(StaticJsonDocument<DOCSIZE> &doc);
    int getPin(StaticJsonDocument<DOCSIZE> &doc);
    void setRgbLed(StaticJsonDocument<DOCSIZE> &doc);
    void setBuzzer(StaticJsonDocument<DOCSIZE> &doc);
    void serialWriteToESP32(StaticJsonDocument<DOCSIZE> &doc);
    void serialReadFromESP32();
    void setPanic(StaticJsonDocument<DOCSIZE> &doc);
    void runRgbLed();
    void runBuzzer();
    void runPanic();
    void coMCUGetInfo(StaticJsonDocument<DOCSIZE> &doc);
    ConfigCoMCU configCoMCU;
    OneWire oneWire;
    DallasTemperature ds18b20;
  private:
    bool _toBoolean(String &value);
    void _serialCommandHandler(HardwareSerial &serial);
    Led _rgb;
    Buzzer _buzzer;
};



libudawaatmega328::libudawaatmega328()
{
  _buzzer.lastState = 1;
  oneWire = OneWire(ONE_WIRE_BUS);
  ds18b20 = DallasTemperature(&oneWire);
}

void libudawaatmega328::begin()
{
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  configCoMCU.fPanic = false;
  configCoMCU.pEcKcoe = 2.9;
  configCoMCU.pEcTcoe = 0.019;
  configCoMCU.pEcVin = 4.54;
  configCoMCU.pEcPpm = 0.5;
  configCoMCU.pEcR1 = 0.5;
  configCoMCU.pEcRa = 25;

  configCoMCU.bfreq = 1600;
  configCoMCU.fBuzz = true;

  configCoMCU.pinBuzzer = 3;
  configCoMCU.pinLedR =  9;
  configCoMCU.pinLedG =  10;
  configCoMCU.pinLedB =  11;
  configCoMCU.pinEcPower =  15;
  configCoMCU.pinEcGnd =  16;
  configCoMCU.pinEcData = 14;
}

void libudawaatmega328::execute()
{
  serialReadFromESP32();
  runRgbLed();
  runBuzzer();
  runPanic();
}

void libudawaatmega328::coMCUGetInfo(StaticJsonDocument<DOCSIZE> &doc)
{
  doc["method"] = "coMCUSetInfo";
  doc["board"] = BOARD;
  doc["fwv"] = COMPILED;
  doc["heap"] = getFreeHeap();
  serialWriteToESP32(doc);
}

int libudawaatmega328::getFreeHeap(uint16_t min,uint16_t max)
{
  if (min==max-1)
  {
    return min;
  }
  int size=max;
  int lastSize=size;
  byte *buf;
  while ((buf = (byte *) malloc(size)) == NULL)
  {
    lastSize=size;
    size-=(max-min)/2;
  };

  free(buf);
  return getFreeHeap(size,lastSize);
}

int libudawaatmega328::getFreeHeap()
{
    return getFreeHeap(0,4096);
}

void libudawaatmega328::serialWriteToESP32(StaticJsonDocument<DOCSIZE> &doc)
{
  serializeJson(doc, Serial);
}

void libudawaatmega328::serialReadFromESP32()
{
  StaticJsonDocument<DOCSIZE> doc;

  if (Serial.available())
  {
    DeserializationError err = deserializeJson(doc, Serial);

    if (err == DeserializationError::Ok)
    {
      const char* method = doc["method"].as<const char*>();
      if(strcmp(method, (const char*) "coMCUGetInfo") == 0)
      {
        coMCUGetInfo(doc);
      }
      else if(strcmp(method, (const char*) "setConfigCoMCU") == 0)
      {
        setConfigCoMCU(doc);
      }
      else if(strcmp(method, (const char*) "getWaterTemp") == 0)
      {
        getWaterTemp(doc);
      }
      else if(strcmp(method, (const char*) "getWaterEC") == 0)
      {
        getWaterEC(doc);
      }
      else if(strcmp(method, (const char*) "setRgbLed") == 0)
      {
        setRgbLed(doc);
      }
      else if(strcmp(method, (const char*) "setBuzzer") == 0)
      {
        setBuzzer(doc);
      }
      else if(strcmp(method, (const char*) "setPin") == 0)
      {
        setPin(doc);
      }
      else if(strcmp(method, (const char*) "getPin") == 0)
      {
        getPin(doc);
      }
      else if(strcmp(method, (const char*) "setPanic") == 0)
      {
        setPanic(doc);
      }
      //serializeJson(doc, Serial);
    }
    else
    {
      //Log.error(F("SerialCoMCU DeserializeJson() returned: %s" CR), err.c_str());
    }
  }
}

void libudawaatmega328::setConfigCoMCU(StaticJsonDocument<DOCSIZE> &doc)
{

  configCoMCU.fPanic = doc["fPanic"].as<bool>();
  configCoMCU.pEcKcoe = doc["pEcKcoe"].as<float>();
  configCoMCU.pEcTcoe = doc["pEcTcoe"].as<float>();
  configCoMCU.pEcVin = doc["pEcVin"].as<float>();
  configCoMCU.pEcPpm = doc["pEcPpm"].as<float>();
  configCoMCU.pEcR1 = doc["pEcR1"].as<unsigned int>();
  configCoMCU.pEcRa = doc["pEcRa"].as<unsigned int>();

  configCoMCU.bfreq = doc["bfreq"].as<uint16_t>();
  configCoMCU.fBuzz = doc["fBuzz"].as<bool>();

  configCoMCU.pinBuzzer = doc["pinBuzzer"].as<uint8_t>();
  configCoMCU.pinLedR = doc["pinLedR"].as<uint8_t>();
  configCoMCU.pinLedG = doc["pinLedG"].as<uint8_t>();
  configCoMCU.pinLedB = doc["pinLedB"].as<uint8_t>();
  configCoMCU.pinEcPower = doc["pinEcPower"].as<uint8_t>();
  configCoMCU.pinEcGnd = doc["pinEcGnd"].as<uint8_t>();
  configCoMCU.pinEcData = doc["pinEcData"].as<uint8_t>();
}

void libudawaatmega328::setPin(StaticJsonDocument<DOCSIZE> &doc)
{
  pinMode(doc["params"]["pin"].as<uint8_t>(), doc["params"]["mode"].as<uint8_t>());
  if(doc["params"]["type"].as<char>() == 'A'){
    analogWrite(doc["params"]["pin"].as<uint8_t>(), doc["params"]["aval"].as<uint16_t>());
  }
  else if(doc["params"]["type"].as<char>() == 'D'){
    digitalWrite(doc["params"]["pin"].as<uint8_t>(), doc["params"]["state"].as<uint8_t>());
  }
}

int libudawaatmega328::getPin(StaticJsonDocument<DOCSIZE> &doc)
{
  int state;
  if(doc["params"]["type"].as<char>() == 'A')
  {
    state = analogRead(doc["params"]["pin"].as<uint8_t>());
    doc["params"]["state"] = state;
  }
  else if(doc["params"]["type"].as<char>() == 'D')
  {
    state = digitalRead(doc["params"]["pin"].as<uint8_t>());
    doc["params"]["state"] = state;
  }
  serialWriteToESP32(doc);
  return state;
}

void libudawaatmega328::setRgbLed(StaticJsonDocument<DOCSIZE> &doc)
{
  _rgb.r = doc["params"]["r"].as<uint8_t>();
  _rgb.g = doc["params"]["g"].as<uint8_t>();
  _rgb.b = doc["params"]["b"].as<uint8_t>();
  _rgb.isBlink = doc["params"]["isBlink"].as<uint8_t>();
  _rgb.blinkCount = doc["params"]["blinkCount"].as<uint16_t>();
  _rgb.blinkDelay = doc["params"]["blinkDelay"].as<uint16_t>();
  _rgb.off = doc["params"]["off"].as<uint8_t>();
  _rgb.on = doc["params"]["on"].as<uint8_t>();
}

void libudawaatmega328::setBuzzer(StaticJsonDocument<DOCSIZE>& doc)
{
  _buzzer.beepCount = doc["params"]["beepCount"].as<uint16_t>();
  _buzzer.beepDelay = doc["params"]["beepDelay"].as<uint16_t>();
}

void libudawaatmega328::runRgbLed()
{
  uint32_t now = millis();
  if(!_rgb.isBlink)
  {
    analogWrite(configCoMCU.pinLedR, _rgb.r);
    analogWrite(configCoMCU.pinLedG, _rgb.g);
    analogWrite(configCoMCU.pinLedB, _rgb.b);
  }
  else if(_rgb.blinkCount > 0 && now - _rgb.lastRun > _rgb.blinkDelay){
    if(_rgb.lastState == 1){
      analogWrite(configCoMCU.pinLedR, _rgb.r);
      analogWrite(configCoMCU.pinLedG, _rgb.g);
      analogWrite(configCoMCU.pinLedB, _rgb.b);

      _rgb.blinkCount--;
    }
    else{
      analogWrite(configCoMCU.pinLedR, _rgb.off);
      analogWrite(configCoMCU.pinLedG, _rgb.off);
      analogWrite(configCoMCU.pinLedB, _rgb.off);
    }

    _rgb.lastState = !_rgb.lastState;
    _rgb.lastRun = now;
  }
}

void libudawaatmega328::runBuzzer()
{
  uint32_t now = millis();
  if(_buzzer.beepCount > 0 && now - _buzzer.lastRun > _buzzer.beepDelay)
  {
    if(_buzzer.lastState == 1){
      tone(configCoMCU.pinBuzzer, configCoMCU.bfreq, _buzzer.beepDelay);
      _buzzer.beepCount--;
    }
    else{
      noTone(configCoMCU.pinBuzzer);
    }

    _buzzer.lastState = !_buzzer.lastState;
    _buzzer.lastRun = now;
  }
}

float libudawaatmega328::readWaterTemp()
{
  float celcius = ds18b20.getTempCByIndex(0);
  return celcius;
}

void libudawaatmega328::getWaterTemp(StaticJsonDocument<DOCSIZE> &doc)
{
  JsonObject params = doc.createNestedObject("params");
  doc["method"] = "setWaterTemp";
  params["celcius"] = readWaterTemp();

  serialWriteToESP32(doc);
}

void libudawaatmega328::getWaterEC(StaticJsonDocument<DOCSIZE> &doc)
{
  pinMode(configCoMCU.pinEcPower, OUTPUT);
  pinMode(configCoMCU.pinEcGnd, OUTPUT);
  pinMode(configCoMCU.pinEcData, INPUT);

  float ec = 0;
  float ec25 = 0;
  int ppm = 0;
  float raw = 0;
  float voltageDrop = 0;
  float rc = 0;
  float waterTemp = readWaterTemp();

  digitalWrite(configCoMCU.pinEcPower, HIGH);
  raw = analogRead(configCoMCU.pinEcData);
  raw = analogRead(configCoMCU.pinEcData);
  digitalWrite(configCoMCU.pinEcPower, LOW);

  voltageDrop = (configCoMCU.pEcVin * raw) / 1024.0;
  rc =(voltageDrop * configCoMCU.pEcR1) / (configCoMCU.pEcVin - voltageDrop);
  rc = rc - configCoMCU.pEcRa;
  ec = 1000 / (rc * configCoMCU.pEcKcoe);

  ec25  =  ec / (1 + configCoMCU.pEcTcoe * (waterTemp - 25.0));
  ppm =(ec25) * (configCoMCU.pEcPpm * 1000);

  JsonObject params = doc.createNestedObject("params");
  doc["method"] = "setWaterEC";
  params["waterEC"] = ec25;
  params["waterPPM"] = ppm;
  params["waterTemp"] = waterTemp;
  params["vDrop"] = voltageDrop;
  params["_ec"] = ec;
  params["raw"] = raw;
  serialWriteToESP32(doc);
}

void libudawaatmega328::runPanic()
{
  if(configCoMCU.fPanic)
  {
    _rgb.r = 0;
    _rgb.g = 255;
    _rgb.b = 255;
    _rgb.isBlink = 1;
    _rgb.blinkCount = 10;
    _rgb.blinkDelay = 100;

    _buzzer.beepCount = 10;
    _buzzer.beepDelay = 100;
  }
}

void libudawaatmega328::setPanic(StaticJsonDocument<DOCSIZE> &doc)
{
  configCoMCU.fPanic = doc["params"]["state"].as<bool>();
}

#endif