

#include <ESP8266WiFi.h>


unsigned long int crc_32b(char *buffer, int length){
    int i, j;
    unsigned long int byte, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;

    while (i < length) {
        byte = (unsigned long int) buffer[i]; // Get next byte.
        crc = crc ^ byte;

        for (j = 7; j >= 0; j--)
        { // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }
    return ~crc;
}


uint8_t ipComponent(String ip, int n) {
    String s = "";
    int section = 0;
    uint8_t res;
    for (int i = 0; i < ip.length(); i++) {
        if (ip[i] == '.') {
            section++;
        } else if (section == n) {
            s += ip[i];
        } else if (section > n) {
            break;
        }
    }

    res = (uint8_t) s.toInt();
    return res;
}


void UARTmessage(int command, char *data, int length, char destination){
    char message[length + 12];

    message[0] = 0x01;
    message[1] = 0x07;
    message[2] = destination;
    message[3] =((command & 0xFF00) >> 8);
    message[4] = (command & 0x00FF);
    message[5] = (length >> 8) & 0xff;
    message[6] = (length) & 0xff;
    memcpy(message + 7, data, length);

    unsigned long int crc = crc_32b(message, length + 7);

    message[length + 7] = (crc >> 24);
    message[length + 8] = ((crc >> 16) & 0x000000FF);
    message[length + 9] = ((crc >> 8) & 0x000000FF);
    message[length + 10] = (crc & 0x000000FF);
    message[length + 11] = 0x04;

    unsigned int pktlen = length + 12;

    if (pktlen > 2048+12)
    {
        //TODO pacchetto troppo grande
        return;
    }

      Serial.write(message, pktlen);
}


void sendIP(IPAddress ip) {
  char ipAdd[4];
  ipAdd[0] = ipComponent(ip.toString(), 0);
  ipAdd[1] = ipComponent(ip.toString(), 1);
  ipAdd[2] = ipComponent(ip.toString(), 2);
  ipAdd[3] = ipComponent(ip.toString(), 3);
  UARTmessage(0xEE00, ipAdd, 4, 7);
}