#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <connection.h>
#include <EEPROM.H>
#include <ArduinoOTA.h>
#include "mymem.h"
#include "timers.h"
#include "uart_rx_tx.h"
#include "ota.h"

bool haveSavedCredentials = false;
char default_SSID[] = "MSLaundry";
char default_PASS[] = "@4SbMKe4";
char SSID[32];
char pass[32];
int old_status;
int node, old_node;


void setup() {
    node = -1;
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

    // delete all existing WiFi stuff in EEPROM
/*       WiFi.disconnect();
       WiFi.softAPdisconnect();
       WiFi.mode(WIFI_OFF);
       delay(500);*/


    node = askNodeAddress();
    old_node = node;

    #ifdef DEBUG
    Serial.println("Comincio");
    haveSavedCredentials = savedCredentials();
    if (haveSavedCredentials) {
        for (int i = 0; i < 32; i++) {
            SSID[i] = EEPROM.read(2+i);
        }
        SSID[31] = '\0';
        for (int i = 0; i < 32; i++) {
            pass[i] = EEPROM.read(65+i);
        }
        pass[31] = '\0';
    } else {
    #endif
        memset(SSID, 0, 32);
        memset(pass, 0, 32);
        strcpy(SSID, default_SSID);
        strcpy(pass, default_PASS);
    #ifdef DEBUG
    }
    #endif
    connect(SSID, pass, node, false);
    old_status = WiFi.status();
    initOTA();
    user_init();
}

void loop() {
    WiFiClient client = sockServer.available();
    WiFiClient extra;
    int node;

    /* Every 60 seconds try to connect */
    if (tickOccured) {
        if (WiFi.getMode() == WIFI_AP && !client) {
            //Serial.println("periodic connection try");
            node = askNodeAddress();
            old_node = node;
            connect(SSID, pass, node, false);
        }
        tickOccured = false;
        return;
    }


    /* Se un client non e' connesso gestisci il web server, i comandi
        per il modulo e l'OTA
        TODO: verificare che sia veramente utile: potremmo voler avere sempre lo stesso baudrate */
    if (!client) {// && (WiFi.getMode() == WIFI_AP || WiFi.status() == WL_CONNECTED)) {
        // Server
        server.handleClient();

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