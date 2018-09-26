
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <connection.h>
#include "timers.h"
#include "uart_rx_tx.h"

const int versione1 = 1;
const int versione2 = 0;

WiFiServer sockServer(8080);

int8_t baud = 3;
bool tryToConnect = false;

int connect(char *ssid, char *pass, byte node, bool keepTrying) {
    byte node_ip;
  WiFi.mode(WIFI_OFF);
  delay(500);
  char emptyIP[] = {0x7F, 0x00, 0x00, 0x01};
  UARTmessage(0xEE00, emptyIP, 4, 0x07);

  /*if (node < 0 || node > 255) {
    node_ip = 0;
  }*/
  node_ip = node;

  WiFi.mode(WIFI_STA);
  WiFi.config(IPAddress(192,168,11,node_ip), IPAddress(192,168,11,254),IPAddress(255,255,255,0));
  WiFi.begin(ssid,pass);
  int i = 0;

  #ifdef DEBUG
    Serial.print("Trying to connect to ");
    Serial.println(ssid);
    Serial.println(pass);
  #endif

  while (WiFi.status() != WL_CONNECTED && (i <20 || keepTrying) ) {
    delay(500);
    #ifdef DEBUG
    Serial.print(".");
    #endif
    i++;
  }
  

  IPAddress myIP;

  if (WiFi.status() != WL_CONNECTED ) {
    myIP = setupAccessPoint(node);
    sendIP(myIP);
    #ifdef DEBUG
    Serial.println("access point");
    #endif
    return -1;
  } else {
    WiFi.setAutoReconnect(true);
    sockServer.close();
    sockServer.begin();
    myIP = WiFi.localIP();
    #ifdef DEBUG
    Serial.print("connected to ");
    Serial.println(ssid);
    Serial.print("with ip ");
    Serial.println(myIP);
    #endif
    sendIP(myIP);
    return 0;
  }
}

IPAddress setupAccessPoint(int n) {
    byte mac[6];
    char APssid[33];
    WiFi.mode(WIFI_OFF);
    delay(500);
    WiFi.mode(WIFI_AP);
    if (n >= 0) {
        sprintf(APssid, "MSG-WiFi v%i.%i # %03i",versione1,versione2, n);
    } else {
        WiFi.macAddress(mac);
        sprintf(APssid, "MSG-WiFi v%i.%i %02X:%02X:%02X:%02X:%02X:%02X", versione1, versione2, mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    }
    WiFi.softAP(APssid);
    
    sockServer.begin();
    IPAddress myIP = WiFi.softAPIP();
    delay(1000*3);
    sendIP(myIP);
    return myIP;
}

bool checkForHiddenNetwork(char *network_ssid)
{
    String ssid;
    char buffer[32];
    uint8_t encryptionType;
    int32_t RSSI;
    uint8_t *BSSID;
    int32_t channel;
    bool isHidden;
    int netcount = WiFi.scanNetworks(false, true);
    for (int n = 0; n < netcount; n++)
    {
        WiFi.getNetworkInfo(n, ssid, encryptionType, RSSI, BSSID, channel, isHidden);
        ssid.toCharArray(buffer, 32);
        #ifdef DEBUG
        Serial.println(String("SSID : ") + ssid);
        Serial.println(String("encryptionType : ") + encryptionType);
        Serial.println(String("RSSI : ") + RSSI);
        Serial.println(String("Channel : ") + channel);
        Serial.println(String("Hidden : ") + isHidden);
        #endif
        //if (isHidden && strcmp(buffer, network_ssid) == 0) {
        if (strcmp(buffer, network_ssid) == 0)
        {
            return true;
        }
    }
    return false;
}
