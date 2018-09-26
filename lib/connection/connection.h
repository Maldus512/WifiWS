#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

extern WiFiServer sockServer;
extern int8_t baud;

void scanNetworks();

int connect(char *ssid, char* pass, byte node, bool keepTrying);
IPAddress setupAccessPoint(int n);
bool checkForHiddenNetwork(char *network_ssid);

#endif