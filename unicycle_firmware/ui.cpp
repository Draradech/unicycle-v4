#include "stdinc.h"
#include <FastLED.h>

CRGB leds[1];

void setupUI()
{
  FastLED.addLeds<WS2812B, 47, RGB>(leds, 1);
  sensorData.chargeState = chg_normal;
}

void ledColor(uint8_t r, uint8_t g, uint8_t b)
{
  leds[0] = CRGB(r, g, b);
  FastLED.show();
}

static int8_t lastCharge = -1;
void loopUI()
{
  if(joystickTimeout) joystickTimeout--;
  
  static int8_t dbb = 0;
  int8_t b = digitalRead(0) * 2 - 1;
  dbb += b;
  dbb = LIMIT(dbb, -10, 10);

  static int8_t dbj = 0;
  dbj += ((BUTTON(10) && BUTTON(11)) ? 1 : -1);
  dbj = LIMIT(dbj, -10, 10);
  if ((dbb == 0 && b == -1) || (dbj == 0 && BUTTON(10) && BUTTON(11)))
  {
    if (!wifiOn)
    {
      setupWifi();
      setupOta();
      setupInputOutput();
      lastCharge = -1;
    }
    else
    {
      disconnectWifi();
      lastCharge = -1;
    }
  }

  if (sensorData.chargeState != chg_empty)
  {
         if (sensorData.voltage >= 14.0f) sensorData.chargeState = chg_normal;
    else if (sensorData.voltage >= 13.6f) sensorData.chargeState = chg_low;
    else if (sensorData.voltage >= 13.2f) sensorData.chargeState = chg_critical;
    else                                  sensorData.chargeState = chg_empty;
  }

  if (sensorData.chargeState != lastCharge)
  {
    lastCharge = sensorData.chargeState;
    switch (sensorData.chargeState)
    {
      case chg_normal:
      {
        if (wifiOn) ledColor(0, 32, 64);
        else ledColor(0, 32, 0);
        break;
      }
      case chg_low:
      {
        if (wifiOn) ledColor(64, 32, 64);
        else ledColor(64, 32, 0);
        break;
      }
      case chg_critical:
      {
        if (wifiOn) ledColor(64, 0, 64);
        else ledColor(64, 0, 0);
        break;
      }
      case chg_empty:
      {
        actuator.disabled = true;
        disconnectWifi();
        disconnectBLE();
        break;
      }
    }
  }

  if (sensorData.chargeState == chg_empty)
  {
    static uint8_t loop = 0;
    if (loop == 0) ledColor(64, 0, 0);
    if (loop == 25) ledColor(0, 0, 0);
    loop++;
  }
}
