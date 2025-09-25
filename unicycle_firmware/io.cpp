#include "stdinc.h"
#include <WiFiUdp.h>

struct
{
  uint8_t channel[6];
  float data[6];
} dataout;

union udat
{
  int32_t intval;
  float floatval;
};

struct
{
  uint8_t type;
  udat data;
} datain;

IPAddress target(192, 168, 4, 2);
WiFiUDP udp;
static uint8_t subLoop;
static bool measurementOut;

void setupInputOutput()
{
  for(int i = 0; i < 5; ++i) dataout.channel[i] = '1' + i;
  dataout.channel[5] = '1';
  udp.begin(23456);
}

void loopInputOutput()
{
  if (!wifiOn) return;

  subLoop++;
  subLoop %= 5;

  #define ENTRY(ID, var, unit, name, minvalue, maxvalue)\
  case ID: dataout.data[c] = (var); break;

  if(subLoop == 0)
  {
    if(measurementOut && wifiOn)
    {
      /*
      sensorData.taskTime[0][ttavg] = sensorData.taskTime[0][ttsum] / sensorData.taskTime[0][ttnum];
      sensorData.taskTime[1][ttavg] = sensorData.taskTime[1][ttsum] / sensorData.taskTime[1][ttnum];
      */
      uint8_t c;
      for(c = 0; c < 5; c++)
      {
        switch(dataout.channel[c])
        {
        #include "mtable.h"
        default: dataout.data[c] = -1; break;
        }
      }
      #define ENTRY(ID, var, increment, unit, name)\
      case ID: dataout.data[c] = (var); break;
      switch(dataout.channel[c])
      {
        #include "ptable.h"
        default: dataout.data[c] = -1; break;
      }
      udp.beginPacket(target, 1234);
      udp.write((uint8_t *)&dataout, sizeof(dataout));
      udp.endPacket();
    }
    //Serial.println(sensorData.taskTime[1][ttmax]);
    /*
    sensorData.taskTime[0][ttmin] = 65000;
    sensorData.taskTime[0][ttmax] = 0;
    sensorData.taskTime[0][ttsum] = 0;
    sensorData.taskTime[0][ttnum] = 0;
    sensorData.taskTime[1][ttmin] = 65000;
    sensorData.taskTime[1][ttmax] = 0;
    sensorData.taskTime[1][ttsum] = 0;
    sensorData.taskTime[1][ttnum] = 0;
    */
  }


  #define ENTRY(ID, var, increment, unit, name)\
  case ID: var += datain.data.floatval; break;

  int cb = udp.parsePacket();
  if (cb == sizeof(datain))
  {
    udp.read((uint8_t*)&datain, cb);
    switch(datain.type)
    {
      case 'a':
      {
        input.a = (datain.data.intval >> 0) & 0xff;
        input.b = (datain.data.intval >> 8) & 0xff;
        input.c = (datain.data.intval >> 16) & 0xff;
        input.d = (datain.data.intval >> 24) & 0xff;
        break;
      }
      case 'r':
      {
        ESP.restart();
        break;
      }
      case 'c':
      {
        dataout.channel[datain.data.intval >> 8] = datain.data.intval & 0xff;
        break;
      }
      case 'p':
      {
        dataout.channel[5] = datain.data.intval;
        break;
      }
      case 'i':
      {
        switch(dataout.channel[5])
        {
          #include "ptable.h"
        }
        break;
      }
      case 'm':
      {
        measurementOut = true;
        break;
      }
      case 's':
      {
        measurementOut = false;
        break;
      }
      case 'x':
      {
        measurementOut = !measurementOut;
        break;
      }
    }
  }
}
