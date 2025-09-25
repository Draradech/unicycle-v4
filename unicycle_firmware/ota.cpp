#include "stdinc.h"
#include <ArduinoOTA.h>

static bool done = false;
static int16_t rebootDelay = 500;

void setupOta()
{
  ArduinoOTA.setHostname("unicycle");

  ArduinoOTA.onStart([]() {
    actuator.disabled = true;
    ledColor(0, 0, 64);
  });

  ArduinoOTA.onEnd([]() {
    done = true;
  });

  ArduinoOTA.begin();
}

void loopOta()
{
  if (!wifiOn) return;
  
  ArduinoOTA.handle();
  if(done)
  {
    if(--rebootDelay == 0)
    {
      ESP.restart();
    }
  }
}
