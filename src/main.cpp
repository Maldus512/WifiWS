#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <html.h>



/*
const char *ssid = "HSW";
const char *password = "zannazanna";*/

const char *ssid = "WS2016";
WiFiServer sockServer(8080);



ESP8266WebServer server(80);

void handleRoot() {
  if (server.hasArg("ssid")) {
    String nssid = server.arg("ssid");
    String npass = server.arg("pass");
    //Serial.println(nssid);
    //Serial.println(npass);
  }
  server.send(200, "text/html", html);
}


void setup() {

  delay(1000);

  Serial.begin(115200);

  Serial.println();

  //Serial.println("Configuring wifi...");
  WiFi.mode(WIFI_OFF);  
  delay(200);

  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid);
 /* WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }*/

  IPAddress myIP = WiFi.softAPIP();
  /*Serial.print("IP address: ");
  Serial.println(myIP);
    Serial.println("begin");*/

  server.on("/", handleRoot);
  server.begin();

  sockServer.begin();
}

void loop() {
  WiFiClient client = sockServer.available();
  char recv;

  if (!client) {
    server.handleClient();
    delay(10);
    return;
  }

  while (client.connected()) {
    //Wait for data while managing stuff
    if (!client.available() && !Serial.available()) {
      server.handleClient();
      delay(1);
      continue;
    }

    if (client.available()) {
      int x = client.available();
      while (x-- > 0) {
        recv = client.read();
        Serial.write(recv);
      }
    }

    if (Serial.available()) {
      int x = Serial.available();
      char buf[x];
      Serial.readBytes(buf, x);
      client.print(buf);
    }
  }
}
