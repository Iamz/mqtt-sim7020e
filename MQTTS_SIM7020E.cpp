#include "MQTTS_SIM7020E.h"
#include <Arduino.h>

MQTTS_SIM7020E::MQTTS_SIM7020E():
  _serial(1) {}

void MQTTS_SIM7020E::setupModule() {
  pinMode(26, OUTPUT);

  rebootModule();

  _serial.begin(9600, SERIAL_8N1, 16, 17);
}

void MQTTS_SIM7020E::rebootModule() {
  digitalWrite(26, LOW);
  delay(800);
  digitalWrite(26, HIGH);
}

void MQTTS_SIM7020E::checkModuleReady() {
  int count = 0;
  _serial.println(F("AT"));
  int previousCheck = millis();

  while (1) {
    if (_serial.available()) {
      _data_input = _serial.readStringUntil('\n');
      if (_data_input.indexOf(F("OK")) != -1) {
        Serial.println(F("...SIM7020E is Ready"));
        break;
      }
    } else {
      int currentCheck = millis();
      if (currentCheck - previousCheck > 5000) {
        if (count > 5) {
          Serial.print(F("\nCannot connect to SIM7020E, rebooting..."));
        }
        previousCheck = currentCheck;
        Serial.print(F("."));
        rebootModule();
        _serial.println(F("AT"));
        delay(100);
        count++;
      }
      delay(500);
      _serial.println(F("AT"));
      delay(100);
      Serial.print(F("."));
    }
  }
  _flushSerial();
}

void MQTTS_SIM7020E::waitForNetwork() {
  _serial.println(F("AT+CGATT?"));

  while (1) {
    if (_serial.available()) {
      _data_input = _serial.readStringUntil('\n');
      if (debug) Serial.println("<<" + _data_input);
      if (_data_input.indexOf(F("+CGATT: 1")) != -1) {
        delay(1000);
        Serial.println(F("...Network is ready"));
        break;
      }
    } else {
      delay(1000);
      _serial.println(F("AT+CGATT?"));
      delay(100);
      Serial.print(F("."));
    }
  }
  _flushSerial();
}

String MQTTS_SIM7020E::dnsLookup(String hostname) {
  _serial.println("AT+CDNSGIP=\"" + hostname + "\"");
  String data = "";
  while (1) {
    if (_serial.available()) {
      _data_input = _serial.readStringUntil('\n');
      if (debug) Serial.println("<<" + _data_input);
      if (_data_input.indexOf(F("+CDNSGIP: 1")) != -1) {
        int index = _data_input.indexOf(F(","));
        index = _data_input.indexOf(F(","), index + 1);
        data = _data_input.substring(index + 2, _data_input.length() - 2);
        break;
      } else if (_data_input.indexOf(F("+CDNSGIP: 0")) != -1) {
        return "";
      }
    }
  }
  _flushSerial();
  return data;
}

void MQTTS_SIM7020E::setCertificate(int type, String cert) {
  _clearCertificate(type);
  const int batchSize = 1024;
  int certSize = cert.length() + 3;
  int batchCount = cert.length() / batchSize;
  if (cert.length() % 1024 != 0) {
    batchCount++;
  }
  for (int i = 0; i < batchCount; i++) {
    int startIndex = i * batchSize;
    int endIndex = startIndex + batchSize;
    bool lastBatch = i == batchCount - 1;
    String payload = cert.substring(startIndex, endIndex);
    _serial.print(F("AT+CSETCA="));
    _serial.print(type);
    _serial.print(F(","));
    _serial.print(certSize);
    _serial.print(F(","));
    _serial.print(lastBatch ? 0 : 1);
    _serial.print(F(",0,\""));
    _serial.print(payload);
    _serial.println(F("\""));
    while (1) {
      if (_serial.available()) {
        _data_input = _serial.readStringUntil('\n');
        if (debug) Serial.println("<<" + _data_input);
        if (_data_input.indexOf(F("OK")) != -1) {
          break;
        }
        if (_data_input.indexOf(F("ERROR")) != -1) {
          Serial.print(F("ERROR setting certificate type: "));
          Serial.println(type);
          return;
        }
      }
    }
    _flushSerial();
  }
  _flushSerial();
}

void MQTTS_SIM7020E::_clearCertificate(int type) {
  _serial.print(F("AT+CSETCA="));
  _serial.print(type);
  _serial.println(F(",1,0,0,\" \""));
  while (1) {
    if (_serial.available()) {
      _data_input = _serial.readStringUntil('\n');
      if (debug) Serial.println("<<" + _data_input);
      if (_data_input.indexOf(F("OK")) != -1) {
        break;
      }
    }
  }
  _flushSerial();
}

void MQTTS_SIM7020E::checkCertificates() {
  _serial.println(F("AT+CSETCA?"));
  while (1) {
    if (_serial.available()) {
      _data_input = _serial.readStringUntil('\n');
      Serial.println(_data_input);
      if (_data_input.indexOf("OK") != -1) {
        break;
      }
    }
  }
  _flushSerial();
}

void MQTTS_SIM7020E::connect(String ipAddress, String port) {
  _serial.print(F("AT+CMQTTSNEW=\""));
  _serial.print(ipAddress);
  _serial.print(F("\",\""));
  _serial.print(port);
  _serial.println(F("\",60000,1024"));
  int count = 0;
  while (1) {
    if (_serial.available()) {
      _data_input = _serial.readStringUntil('\n');
      if (debug) Serial.println("<<" + _data_input);
      if (_data_input.indexOf("OK") != -1) {
        Serial.println(F("...MQTT connected"));
        break;
      }
      if (_data_input.indexOf("ERROR") != -1) {
        Serial.println(F("ERROR creating new MQTTS connection"));
        break;
      }
    } else if (count < 60) {
      delay(1000);
      Serial.print(F("."));
      count++;
    } else {
      Serial.println(F("Cannot create MQTT connection"));
      break;
    }
  }
  _flushSerial();
}

void MQTTS_SIM7020E::registerClient(String clientName) {
  _serial.print(F("AT+CMQCON=0,4,\""));
  _serial.print(clientName);
  _serial.println(F("\",600,1,0"));
  while (1) {
    if (_serial.available()) {
      _data_input = _serial.readStringUntil('\n');
      if (debug) Serial.println("<<" + _data_input);
      if (_data_input.indexOf("OK") != -1) {
        Serial.println(F("...Client registered"));
        break;
      }
    }
  }
  delay(1000);
  _flushSerial();
}

void MQTTS_SIM7020E::publish(String topic, String payload) {
  bool success = false;
  for (int i = 0; i < 3; i++) {
    _serial.print(F("AT+CMQPUB=0,\""));
    _serial.print(topic);
    _serial.print(F("\",0,0,0,"));
    _serial.print(payload.length() * 2);
    _serial.print(F(",\""));
    for (int j = 0; j < payload.length(); j++) {
      _serial.print(payload[j], HEX);
    }
    _serial.println("\"");
    while (1) {
      if (_serial.available()) {
        _data_input = _serial.readStringUntil('\n');
        if (debug) Serial.println("<<" + _data_input);
        if (_data_input.indexOf("OK") != -1) {
          success = true;
          break;
        } else if (_data_input.indexOf("ERROR") != -1) {
          break;
        }
      }
    }
    if (success) {
      break;
    }
  }
  if (!success) {
    Serial.println(F("ERROR publishing message"));
  }
  _flushSerial();
}

void MQTTS_SIM7020E::disconnect() {
  _serial.println(F("AT+CMQDISCON=0"));
  while (1) {
    if (_serial.available()) {
      _data_input = _serial.readStringUntil('\n');
      if (debug) Serial.println("<<" + _data_input);
      if (_data_input.indexOf("OK") != -1) {
        Serial.println(F("...MQTT disconnected"));
        break;
      }
    }
  }
  _flushSerial();
}

void MQTTS_SIM7020E::_flushSerial() {
  while (1) {
    if (_serial.available()) {
      _serial.readStringUntil('\n');
    } else {
      break;
    }
  }
  _serial.flush();
}
