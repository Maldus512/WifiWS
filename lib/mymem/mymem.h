

int loadSsid(char* ssid, int max);
int loadPass(char* pass, int max);

void saveCredentials(char* ssid, char* pass);
bool savedCredentials();

int loadBaudRate();
void saveBaudRate(int8_t);