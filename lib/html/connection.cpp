
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <connection.h>
#include "mymem.h"
#include "timers.h"
#include "uart_rx_tx.h"

ESP8266WebServer server(80);
WiFiServer sockServer(8080);

const int baudRates[BAUDRATES] = { 4800, 7200, 9600, 19200, 38400, 56000, 57600, 76800, 115200 };
int8_t baud = 3;
bool tryToConnect = false;


const char * mainPage1 = "<!DOCTYPE HTML> \\
<html> \\
<head> \\
<meta name='viewport' content='width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0'> \\
<title>LAVANDERIA</title> \\
<style> \\
'body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }' \\
</style> \\
</head> \\
<body> \\
<h1>Inserire le credenziali del wifi</h1> \\
<form action='/' method='post'> \\
<p> \\
<input type='text' name='ssid' value='network name'><br>" ;

char * networks;


const char * mainPage2 = "<input type='text' name='pass' value='password'><br> \\
<input type='submit' value='Send'> <input type='reset'> \\
</p> \\
</form> \\
</body> \\
</html>";


const char * baudPage = "<!DOCTYPE HTML> \\
<html> \\
<head> \\
<meta name='viewport' content='width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0'> \\
<title>BAUDRATE</title> \\
<style> \\
'body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }' \\
</style> \\
</head> \\
<body> \\
<h1>Inserire il baudrate richiesto</h1> \\
<form action='/' method='post'> \\
<p> \\
<select name='baud'> \\
    <option value='0'>4800</option> \\
    <option value='1'>7200</option> \\
    <option value='2'>9600</option> \\
    <option value='3'>19200</option> \\
    <option value='4'>38400</option> \\
    <option value='5'>56000</option> \\
    <option value='6'>57600</option> \\
    <option value='7'>76800</option> \\
    <option value='8'>115200</option> \\
</select> \\
<input type='submit' value='Send'> \\
</p> \\
</form> \\
</body> \\
</html>";

int connect(char *ssid, char *pass, int node, bool keepTrying) {
    int node_ip;
  WiFi.mode(WIFI_OFF);
  delay(500);
  char emptyIP[] = {0x7F, 0x00, 0x00, 0x01};
  UARTmessage(0xEE00, emptyIP, 4, 0x07);

  if (node < 0 || node > 255) {
    node_ip = 0;
  }

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
    server.close();
    //server.on("/", handleRoot);
    server.on("/baud", handleBaud);
    server.begin();
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
        sprintf(APssid, "MSG-WiFi # %03i", n);
    } else {
        WiFi.macAddress(mac);
        sprintf(APssid, "MSG-WiFi %02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    }
    WiFi.softAP(APssid);
    
    //server.on("/", handleRoot);
    server.on("/baud", handleBaud);
    server.begin();

    sockServer.begin();
    IPAddress myIP = WiFi.softAPIP();
    delay(1000*3);
    sendIP(myIP);
    return myIP;
}

void handleRoot() {
  int node;
  if (server.hasArg("ssid")) {
    String nssid = server.arg("ssid");
    String npass = server.arg("pass");
    char ssid1[nssid.length()+1];
    char pass1[npass.length()+1];
    nssid.toCharArray(ssid1, nssid.length() + 1);
    npass.toCharArray(pass1, npass.length() + 1);
    saveCredentials(ssid1, pass1);
    node = askNodeAddress();
    connect(ssid1, pass1,node,false);
    return;
  } else if (server.hasArg("baud")) {
    String requestedBaud = server.arg("baud");
    int8_t b = requestedBaud.toInt();
    if (b >= 0 && b < BAUDRATES) {
      baud = b;
      saveBaudRate(baud);
      Serial.flush ();   // wait for send buffer to empty
      delay (2);    // let last character be sent
      Serial.end ();      // close serial
      Serial.setRxBufferSize(1024);
      Serial.begin(baudRates[baud]);
    }
  }
  int len1 = strlen(mainPage1);
  int len2 = strlen(mainPage2);
  char html[len1 + len2];
  strcpy(html, mainPage1);
  strcpy(html + len1, mainPage2);

  server.send(200, "text/html", html);
}

void handleBaud() {
  server.send(200, "text/html", baudPage);
}

