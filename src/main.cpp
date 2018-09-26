#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <connection.h>
#include <ArduinoOTA.h>
#include "timers.h"
#include "uart_rx_tx.h"
#include "ota.h"


extern "C" {
  #include<user_interface.h>
}

bool haveSavedCredentials = false;
char default_SSID[] = "MSLaundry";
char default_PASS[] = "@4SbMKe4";
char SSID[32];
char pass[32];
int old_status;
byte node, old_node;


void setup() {
    int baud;
    node = 0;
    WiFi.mode(WIFI_OFF);
    delay(200);

    baud = 115200;
    delay(3*1000);

    Serial.setRxBufferSize(1024);
    Serial.begin(baud);
    Serial.setTimeout(500);

    node = askNodeAddress();
    old_node = node;

    #ifdef DEBUG
    Serial.println("Comincio");
    #endif
    memset(SSID, 0, 32);
    memset(pass, 0, 32);
    strcpy(SSID, default_SSID);
    strcpy(pass, default_PASS);
    connect(SSID, pass, node, false);
    old_status = WiFi.status();
    initOTA();
    user_init();
}

void loop() {
    WiFiClient client = sockServer.available();
    WiFiClient extra;
    byte node;
    int devices;

    /* Every 60 seconds try to connect */
    if (tickOccured) {
        devices = wifi_softap_get_station_num();
        //default_network_found = checkForHiddenNetwork(default_SSID);
        if ((WiFi.getMode() == WIFI_AP 
                || (WiFi.getMode() == WIFI_STA && WiFi.status() != WL_CONNECTED))
                && !client && devices == 0 ) {
            node = askNodeAddress();
            old_node = node;
            connect(SSID, pass, node, false);
        }
        tickOccured = false;
        #ifdef DEBUG
        Serial.println("periodic connection attempt");
        if (WiFi.getMode() == WIFI_AP) {
            Serial.println("Access Point");
        }
        else if (WiFi.getMode() == WIFI_STA){
            Serial.println("Station");
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Connected");
        }
        else {
            Serial.println("Not connected");
        }
        Serial.print("Connected devices: ");
        Serial.println(devices);
        Serial.print("default network found: ");
        Serial.println(default_network_found);
        #endif
        return;
    }


    /* Se un client non e' connesso gestisci il web server, i comandi
        per il modulo e l'OTA */
    if (!client) {// && (WiFi.getMode() == WIFI_AP || WiFi.status() == WL_CONNECTED)) {
        //OTA
        ArduinoOTA.handle();

        // Comandi
        if (Serial.available()) {
            uint8_t buf[LEN_LOCAL];

            int len;
            
            len = Serial.readBytesUntil(0x04, buf, LEN_LOCAL);
            buf[len] = 0x04;
            len++;
            if (manageMessage((char*)buf, len) == 0) {
                delay(5);
                node = askNodeAddress();
                if (node != old_node) {
                    old_node = node;
                    WiFi.disconnect();
                    connect(SSID, pass, node, false);
                }
            }

            /* Se ci sono piu' comandi in coda, leggi solo il primo e butta tutti gli altri */
            do {
                len = Serial.readBytesUntil(0x04, buf, 2*LEN_LOCAL);
            } while (Serial.available());
        }
        delay(1);
        return;
    }

    /* Se il client e' connesso smetti di fare qualsiasi altra cosa e gestisci la connessione */
    if (client) {
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

            /* Rifiuta ogni altra connessione */
            extra =  sockServer.available();
            if (extra) {
                extra.stop();
            }
        }
        //Serial.println("Client disconnesso");
        client.stop();
    }
}