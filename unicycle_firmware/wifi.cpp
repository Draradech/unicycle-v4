#include "stdinc.h"

#include <WiFi.h>

const char* ssid = "unicycle";
const char* passwd = "unicyclepwd";

void setupWifi()
{
  WiFi.persistent(false);
  WiFi.useStaticBuffers(true);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, passwd);
  wifiOn = true;
}

void disconnectWifi()
{
  WiFi.softAPdisconnect(true);
  wifiOn = false;
}