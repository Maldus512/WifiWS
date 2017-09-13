#define BAUDRATES 9


extern const char * html;
extern const char * baudPage;

extern ESP8266WebServer server;
extern WiFiServer sockServer;
extern int8_t baud;
extern const int baudRates[BAUDRATES];

void handleRoot();
void handleBaud();
void scanNetworks();


void connect(char *ssid, char* pass, bool keepTrying);
IPAddress min1AP();
