#include "MQTTS_SIM7020E.h"
#include "secrets.h"
#include <ClosedCube_HDC1080.h>
#include <ArduinoJson.h>

ClosedCube_HDC1080 hdc1080;

MQTTS_SIM7020E at;
String ipAddress;

void setup() {
  Serial.begin(9600);

  hdc1080.begin(0x40);
  hdc1080.setResolution(HDC1080_RESOLUTION_14BIT, HDC1080_RESOLUTION_14BIT);

//  at.debug = true;

  at.setupModule();
  at.checkModuleReady();
  at.waitForNetwork();
  ipAddress = at.dnsLookup(AWS_IOT_ENDPOINT);
  Serial.println("AWS IOT ENDPOINT = " + ipAddress);
  Serial.println(F("Setting certificates..."));
  at.setCertificate(0, AWS_CERT_CA);
  at.setCertificate(1, AWS_CERT_CRT);
  at.setCertificate(2, AWS_CERT_PRIVATE);
  at.checkCertificates();
  at.connect(ipAddress, F("8883"));
  at.registerClient(THINGNAME);
}

void loop() {
  //  if (Serial.available()) {
  //    at._serial.write(Serial.read());
  //  }
  //
  //  if (at._serial.available()) {
  //    Serial.write(at._serial.read());
  //  }
  StaticJsonDocument<200> doc;
  doc["temperature"] = hdc1080.readTemperature();
  doc["humidity"] = hdc1080.readHumidity();
  char jsonBuffer[500];
  serializeJson(doc, jsonBuffer);
  at.publish(F("YOUR TOPIC"), jsonBuffer);
  delay(5000);
}
