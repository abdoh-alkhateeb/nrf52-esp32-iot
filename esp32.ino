#include <BLEDevice.h>

BLEScan* pScan;
BLEClient* pClient;
bool connected = false;

static void notifyCallback(BLERemoteCharacteristic* pRemoteChar, uint8_t* pData, size_t length, bool isNotify) {
  Serial.print("Temperature: ");
  for (size_t i = 0; i < length; i++) Serial.print((char)pData[i]);
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  BLEDevice::init("ESP32 Gateway Node");

  pScan = BLEDevice::getScan();
  pScan->setActiveScan(true);

  pClient = BLEDevice::createClient();
}

void loop() {
  if (!connected) {
    BLEScanResults* results = pScan->start(5);
    Serial.println("Scanning...");

    for (size_t i = 0; i < results->getCount(); i++) {
      BLEAdvertisedDevice device = results->getDevice(i);

      if (device.getName() == "nRF52 Sensor Node") {
        if (pClient->isConnected()) pClient->disconnect();

        if (pClient->connect(&device)) {
          BLERemoteService* service = pClient->getService(BLEUUID((uint16_t)0xFFF0));

          if (service) {
            BLERemoteCharacteristic* pRemoteChar = service->getCharacteristic(BLEUUID((uint16_t)0xFFF1));

            if (pRemoteChar && pRemoteChar->canNotify()) {
              pRemoteChar->registerForNotify(notifyCallback);
              connected = true;
              Serial.println("Connected!");
            }
          }
        }

        break;
      }
    }

    pScan->clearResults();
  }

  if (connected && !pClient->isConnected()) {
    connected = false;
    Serial.println("Disconnected!");
  }

  delay(5000);
}
