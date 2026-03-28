#include <bluefruit.h>

BLEService uartService = BLEService(0xFFF0);
BLECharacteristic txChar = BLECharacteristic(0xFFF1);

void setup() {
  Serial.begin(115200);

  Bluefruit.begin();
  Bluefruit.setName("nRF52 Sensor Node");
  Bluefruit.ScanResponse.addName();

  uartService.begin();

  txChar.setProperties(CHR_PROPS_READ | CHR_PROPS_NOTIFY);
  txChar.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  txChar.setFixedLen(2);
  txChar.begin();

  Bluefruit.Advertising.addService(uartService);
  Bluefruit.Advertising.start();
}

void loop() {
  byte temperature = 30 + random(-10, 11);

  txChar.notify(String(temperature).c_str());

  Serial.print("TX: ");
  Serial.println(temperature);

  delay(5000);
}
