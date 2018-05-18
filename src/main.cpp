#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <html.h>
#include <EEPROM.H>
#include "mymem.h"
#include "timers.h"
#include "uart_rx_tx.h"

bool haveSavedCredentials = false;
char SSID[32];
char pass[32];
int old_status;
int node;


void setup() {
    node = -1;
    byte mac[6];
    char APssid[33];
    WiFi.mode(WIFI_OFF);
    delay(200);

    EEPROM.begin(512);

    baud = loadBaudRate();
    if (baud < 0) {
        baud = 8;
        saveBaudRate(baud);
    }
    delay(3*1000);

    Serial.setRxBufferSize(1024);
    Serial.begin(baudRates[baud]);
    Serial.setTimeout(500);

    //saveCredentials("HSW", "zannazanna");
    haveSavedCredentials = savedCredentials();
    node = askNodeAddress();
    if (haveSavedCredentials) {
        for (int i = 0; i < 32; i++) {
            SSID[i] = EEPROM.read(2+i);
        }
        SSID[31] = '\0';
        for (int i = 0; i < 32; i++) {
            pass[i] = EEPROM.read(65+i);
        }
        pass[31] = '\0';
        min1AP(node); 
    } else {
        WiFi.mode(WIFI_AP);
        if (node >= 0) {
                sprintf(APssid, "MSG-WiFi # %03i", node);
        } else {
            WiFi.macAddress(mac);
            sprintf(APssid, "MSG-WiFi %02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
        }
        WiFi.softAP(APssid);

        IPAddress myIP = WiFi.softAPIP();
        sendIP(myIP);

        server.on("/", handleRoot);
        server.on("/baud", handleBaud);
        server.begin();

        sockServer.begin();
    }
    old_status = WiFi.status();
}

int counter = 0;

void loop() {
    WiFiClient client = sockServer.available();
    char recv;
    if (tickOccured) {
        tickOccured = false;
        counter++;
    }

    if (counter > 60 && WiFi.getMode() == WIFI_AP && haveSavedCredentials) {
        tickOccured = false;
        counter = 0;
        connect(SSID, pass, true);
    }

    if (old_status != WiFi.status()) {
        if (WiFi.getMode() == WIFI_AP) {
            sendIP(WiFi.softAPIP());
        } else {
            sendIP(WiFi.localIP());
        }
        old_status = WiFi.status();
    }

    if (!client && (WiFi.getMode() == WIFI_AP || WiFi.status() == WL_CONNECTED)) {
        server.handleClient();
        if (Serial.available()) {
            uint8_t buf[LEN_LOCAL];

            int len = Serial.readBytesUntil(0x04, buf, LEN_LOCAL);
            buf[len] = 0x04;
            len++;
            manageMessage((char*)buf, len);
        }
        delay(1);
        return;
    }

    if (client) {
        Serial.println("Client presente, chiedo dati");
        while (client.connected()) {
        //check UART for data
            if (client.available()) {
                size_t len = client.available();
                uint8_t sbuf[len];
                client.readBytes(sbuf, len);
                Serial.write(sbuf, len);            
            }

            while (Serial.available()) {
                size_t len = Serial.available();
                uint8_t buf[len];

                Serial.readBytes(buf, len);
                // if the message is meant for me don't pass it
                client.write(buf, len);
            }
        }
        Serial.println("Client disconnesso");
        client.stop();
    }
}