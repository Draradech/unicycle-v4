#include <NimBLEDevice.h>
#include "stdinc.h"

#define SERIAL_PRINT 0

static const char HID_SERVICE[] = "1812";
static const char HID_REPORT_DATA[] = "2A4D";
static NimBLEClient* gClient = nullptr;
static bool gNeedDiscovery = false;

static void notifyCB(NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify)
{
  memcpy(&joystickReport, pData, sizeof(joystickReport));
  joystickTimeout = 100;
}

class ClientCallbacks : public NimBLEClientCallbacks
{
  void onDisconnect(NimBLEClient* pClient, int reason) override
  {
    #if SERIAL_PRINT
    Serial.printf("%s Disconnected, reason = 0x%02X - Starting scan\n", pClient->getPeerAddress().toString().c_str(), reason);
    #endif
    NimBLEDevice::deleteClient(pClient);
    NimBLEDevice::getScan()->start(0);
  }

  void onConnectFail(NimBLEClient* pClient, int reason) override
  {
    #if SERIAL_PRINT
    Serial.printf("%s Connect Failed, reason = 0x%02X - Starting scan\n", pClient->getPeerAddress().toString().c_str(), reason);
    #endif
    NimBLEDevice::deleteClient(pClient);
    NimBLEDevice::getScan()->start(0);
  }

  void onConnect(NimBLEClient* pClient) override
  {
    gClient = pClient;
    gNeedDiscovery = true;
  }
};

ClientCallbacks clientCB;

class ScanCallbacks: public NimBLEScanCallbacks
{
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override
  {
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(NimBLEUUID(HID_SERVICE)))
    {
      #if SERIAL_PRINT
      Serial.printf("Advertised HID Device found: %s\n", advertisedDevice->toString().c_str());
      #endif
      NimBLEDevice::getScan()->stop();

      NimBLEClient* pClient = NimBLEDevice::createClient();
      #if SERIAL_PRINT
      Serial.println("New client created");
      #endif

      pClient->setClientCallbacks(&clientCB, false);
      pClient->connect(advertisedDevice, true, true, true);
    }
  }

  void onScanEnd(const NimBLEScanResults&, int reason) override
  {
    #if SERIAL_PRINT
    Serial.printf("Scan Ended, reason = 0x%02X - Starting scan\n", reason);
    #endif
    NimBLEDevice::getScan()->start(0);
  }
};

void setupBLE()
{
  joystickTimeout = 0;
  #if SERIAL_PRINT
  Serial.begin();
  #endif
  NimBLEDevice::init("");
  NimBLEDevice::setPower(9);
  NimBLEScan* pScan = NimBLEDevice::getScan();
  pScan->setScanCallbacks(new ScanCallbacks());
  #if SERIAL_PRINT
  Serial.println("Starting scan");
  #endif
  pScan->start(0);
}

bool discoverAndSubscribe()
{
  #if SERIAL_PRINT
  Serial.printf("Connected to: %s\n", gClient->getPeerAddress().toString().c_str());
  Serial.printf("RSSI: %d\n", gClient->getRssi());
  #endif

  NimBLERemoteService* pSvc = nullptr;
  NimBLERemoteCharacteristic* pChr = nullptr;

  pSvc = gClient->getService(HID_SERVICE);
  if (!pSvc)
  {
    #if SERIAL_PRINT
    Serial.println("HID service not found");
    #endif
    return false;
  }

  bool subscribed = false;
  for (auto chr : pSvc->getCharacteristics(true))
  {
    if (chr->getUUID() == NimBLEUUID(HID_REPORT_DATA) && chr->getHandle() == 29)
    {
      if (!chr->subscribe(true, notifyCB))
      {
        #if SERIAL_PRINT
        Serial.println("Subscribe failed");
        #endif
        return false;
      }
      else
      {
        #if SERIAL_PRINT
        Serial.println("Subscribed to updates");
        #endif
        subscribed = true;
      }
    }
  }
  return subscribed;
}

void loopBLE()
{
  if (gNeedDiscovery && gClient && gClient->isConnected())
  {
    if (discoverAndSubscribe())
    {
      gNeedDiscovery = false;
    }
    else
    {
      gNeedDiscovery = false;
      gClient->disconnect();
    }
  }
}

void disconnectBLE()
{
  NimBLEDevice::deinit();
}
