#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <html.h>
#include <EEPROM.H>
#include "mymem.h"
#include "timers.h"
#include "uart_rx_tx.h"

bool haveSavedCredentials = false;
char ssid[32];
char pass[32];
int old_status;


void setup() {
    int node = loadNodeAddress();
    byte mac[6];
    char ssid[33];
    WiFi.mode(WIFI_OFF);
    delay(200);

    EEPROM.begin(512);

    baud = loadBaudRate();
    if (baud < 0) {
        baud = 8;
        saveBaudRate(baud);
    }

    Serial.setRxBufferSize(1024);
    Serial.begin(baudRates[baud]);

    haveSavedCredentials = savedCredentials();
    if (haveSavedCredentials) {
        for (int i = 0; i < 32; i++) {
            ssid[i] = EEPROM.read(2+i);
        }
        ssid[31] = '\0';
        for (int i = 0; i < 32; i++) {
            pass[i] = EEPROM.read(65+i);
        }
        pass[31] = '\0';
        min1AP(); 
    } else {
        WiFi.mode(WIFI_AP);
        if (node >= 0) {
                sprintf(ssid, "MSG-WiFi#%03i", node);
        } else {
            WiFi.macAddress(mac);
            sprintf(ssid, "MSG-WiFi#%02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
        }
        WiFi.softAP(ssid);

        IPAddress myIP = WiFi.softAPIP();
        delay(3*1000);
        sendIP(myIP);
        //Serial.println(myIP);

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
        connect(ssid, pass, true);
    }

    if (old_status != WiFi.status()) {
        if (WiFi.getMode() == WIFI_AP) {
            sendIP(WiFi.softAPIP());
        } else {
            sendIP(WiFi.localIP());
        }
        old_status = WiFi.status();
    }

    if (!client) {
        server.handleClient();
        delay(10);
        return;
    }

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
            client.write(buf, len);
        }
    }
    client.stop();
}