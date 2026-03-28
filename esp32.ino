#include <BLEDevice.h>
#include <WiFi.h>
#include <WebServer.h>

BLEScan* pScan;
BLEClient* pClient;

bool connected = false;
String temperature = "";

WebServer server(80);

static void notifyCallback(BLERemoteCharacteristic* pRemoteChar, uint8_t* pData, size_t length, bool isNotify) {
  temperature = "";
  for (size_t i = 0; i < length; i++) temperature += (char)pData[i];

  Serial.print("Temperature: ");
  Serial.println(temperature);
}

void setup() {
  Serial.begin(115200);

  BLEDevice::init();

  pScan = BLEDevice::getScan();
  pScan->setActiveScan(true);

  pClient = BLEDevice::createClient();

  WiFi.softAP("ESP32 Gateway Node", "12345678");

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", [] {
    server.send(200, "text/plain", "Current temperature: " + temperature);
  });

  server.begin();
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

  server.handleClient();
}
