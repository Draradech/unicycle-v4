#include "stdinc.h"

void setup()
{
  setupWifi();
  setupOta();
  setupInputOutput();
  setupSensors();
  setupControl();
  setupUI();
  //setupBLE();
  setupCan();
}

unsigned long last;
void loop()
{
  unsigned long now = micros();
  if (now - last >= 2000)
  {
    loopSensors();
    loopControl();
    loopUI();
    loopInputOutput();
    if (now - last >= 4000) last = now;
    else last += 2000;
  }
  //loopBLE();
  loopOta();
  loopCan();
}
