
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <html.h>
#include "mymem.h"
#include "timers.h"
#include "uart_rx_tx.h"

ESP8266WebServer server(80);
WiFiServer sockServer(8080);

const int baudRates[BAUDRATES] = { 4800, 7200, 9600, 19200, 38400, 56000, 57600, 76800, 115200 };
int8_t baud = 3;
extern int counter;
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


void scanNetworks() {
  int n = WiFi.scanNetworks();
  int len = 0;
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");

      len += WiFi.SSID(i).length();
    }
  }
  Serial.println("");
}

extern int node;

void connect(char *ssid, char *pass, bool keepTrying) {
  WiFi.mode(WIFI_OFF);
  delay(200);
  char emptyIP[] = {0x7F, 0x00, 0x00, 0x01};
  UARTmessage(0xEE00, emptyIP, 4, 0x07);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,pass);
  int i = 0;

  while (WiFi.status() != WL_CONNECTED && (i <30 || keepTrying) ) {
    delay(500);
    //Serial.print(".");
    i++;
  }
  //Serial.print("connected");

  IPAddress myIP;

  if (WiFi.status() != WL_CONNECTED ) {
    node = askNodeAddress();
    myIP = min1AP(node);
  } else {
    myIP = WiFi.localIP();
    WiFi.setAutoReconnect(true);
    sendIP(myIP);
  }
}

IPAddress min1AP(int n) {
    byte mac[6];
    char APssid[33];
    WiFi.mode(WIFI_AP);
    if (n >= 0) {
        sprintf(APssid, "MSG-WiFi # %03i", n);
    } else {
        WiFi.macAddress(mac);
        sprintf(APssid, "MSG-WiFi %02X:%02X:%02X:%02X:%02X:%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
    }
    WiFi.softAP(APssid);
    counter = 0;
    user_init();
    server.on("/", handleRoot);
    server.on("/baud", handleBaud);
    server.begin();

    sockServer.begin();
    IPAddress myIP = WiFi.softAPIP();
    delay(1000*3);
    sendIP(myIP);
    return myIP;
}

void handleRoot() {
  counter = 0;
  if (server.hasArg("ssid")) {
    String nssid = server.arg("ssid");
    String npass = server.arg("pass");
    char ssid1[nssid.length()+1];
    char pass[npass.length()+1];
    nssid.toCharArray(ssid1, nssid.length() + 1);
    npass.toCharArray(pass, npass.length() + 1);
    saveCredentials(ssid1, pass);
    connect(ssid1, pass, false);
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
  counter = 0;
  server.send(200, "text/html", baudPage);
}

