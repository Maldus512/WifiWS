#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "ota.h"



void initOTA() {
    ArduinoOTA.onStart([]() {
        #ifdef DEBUG
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH)
        {
            type = "sketch";
        }
        else
        { // U_SPIFFS
            type = "filesystem";
        }

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
        #endif
    });
    ArduinoOTA.onEnd([]() {
        #ifdef DEBUG
        Serial.println("\nEnd");
        #endif
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        #ifdef DEBUG
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        #endif
    });
    ArduinoOTA.onError([](ota_error_t error) {
        #ifdef DEBUG
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR)
        {
            Serial.println("Auth Failed");
        }
        else if (error == OTA_BEGIN_ERROR)
        {
            Serial.println("Begin Failed");
        }
        else if (error == OTA_CONNECT_ERROR)
        {
            Serial.println("Connect Failed");
        }
        else if (error == OTA_RECEIVE_ERROR)
        {
            Serial.println("Receive Failed");
        }
        else if (error == OTA_END_ERROR)
        {
            Serial.println("End Failed");
        }
        #endif
    });
    ArduinoOTA.begin();
 
}