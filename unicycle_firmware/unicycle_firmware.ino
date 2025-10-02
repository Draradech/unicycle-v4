#include "stdinc.h"

void setup()
{
  setupSensors();
  setupControl();
  setupUI();
  setupBLE();
  setupCan();
  setupSystem();
}

void loop2ms()
{
  loopCan();
  loopSensors();
  loopControl();
  loopUI();
  loopInputOutput();
}

void loop()
{
  loopBLE();
  loopOta();
}
