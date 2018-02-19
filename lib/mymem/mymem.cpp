#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.H>
#define CHECKADDR   0
#define CHECKADDR_BAUD   128
#define BAUDADDR    129
#define SSIDADDR    1
#define PASSADDR    64
#define CHECKADDR_NODE    160
#define NODEADDR    161

int loadBaudRate() {
    if (EEPROM.read(CHECKADDR_BAUD) == 0xAA) {
        return EEPROM.read(BAUDADDR);
    } else {
        return -1;
    }
}

void saveBaudRate(int8_t b) {
    EEPROM.write(CHECKADDR_BAUD, 0xAA);
    EEPROM.write(BAUDADDR, b);
    EEPROM.commit();
    delay(200);
}

bool savedCredentials() {
    return EEPROM.read(CHECKADDR) == 0xAA;
}

void saveCredentials(char* ssid, char* pass) {
    int len1 = strlen(ssid);
    //Serial.println(len1);
    int len2 = strlen(pass);
    //Serial.println(len2);
    //Serial.println(ssid);
    //Serial.println(pass);

    EEPROM.write(SSIDADDR, len1);
    EEPROM.write(PASSADDR, len2);

    for (int i = 0; i < len1; i++) {
        EEPROM.write(SSIDADDR + 1 + i, (uint8_t)ssid[i]);
    }
    EEPROM.write(SSIDADDR +1 +len1, '\0');
    for (int i = 0; i < len2; i++) {
        EEPROM.write(PASSADDR + 1 + i, (uint8_t)pass[i]);
    }
    EEPROM.write(PASSADDR +1 +len2, '\0');
    EEPROM.write(CHECKADDR, 0xAA);

    EEPROM.commit();
    delay(200);
}


void saveNodeAddress(int8_t addr) {
    EEPROM.write(CHECKADDR_NODE, 0xAA);
    EEPROM.write(NODEADDR, addr);
    EEPROM.commit();
    delay(200);
}

int loadNodeAddress() {
    if (EEPROM.read(CHECKADDR_NODE) == 0xAA) {
        return EEPROM.read(NODEADDR);
    } else {
        return -1;
    }
}