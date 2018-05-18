

#include <ESP8266WiFi.h>
#include "uart_rx_tx.h"

int node_number = 0x0;

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
    if (data != NULL)
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
  UARTmessage(0xEE00, ipAdd, 4, 255);
}


int manageMessage(char* msg, int len) {
    int command = 0x0;
    int data_len = len - 5;

    char test[32];

    if (len > LEN_LOCAL || len < 12) 
        return -1;
    if (msg[0] != 0x01 || msg[len-1] != 0x04)
        return -1;


    unsigned long int crc = crc_32b(msg, data_len);

    if (msg[data_len] != (crc>>24) 
        || msg[data_len+1] != ((crc>>16)&0x000000FF) 
        || msg[data_len+2] != ((crc>>8)&0x000000FF) 
        || msg[data_len+3] != (crc & 0x000000FF))
        return -1;

    command |= msg[3] << 8;
    command |= msg[4];

    if (command == IP_COMMAND || true) {
        IPAddress myIP;
        sprintf(test, "0.0.0.%i", len);

        if (WiFi.getMode() == WIFI_AP) {
            myIP = WiFi.softAPIP();
        } else {
            if (WiFi.status() != WL_CONNECTED ) {
                myIP.fromString("0.0.0.0");
            } else {
                myIP = WiFi.localIP();
            }
        }
        //myIP.fromString(test);
        sendIP(myIP);
        return 0;
    }
    return -1;
}


int askNodeAddress() {
    int node;
    int count = 0, len=13, command = 0x0, data_len;
    char buf[len];
    unsigned long int crc;
    while (count++ <= 10) {
        UARTmessage(REQUEST_NODE, NULL, 0, 255);

        len = Serial.readBytes(buf, len);
        if (len != 13) {
            delay(700);
            continue;
        }

        command = 0x0;
        data_len = len - 5;

        if (buf[0] != 0x01 || buf[len-1] != 0x04) {
            delay(700);
            continue;
        }

        crc = crc_32b(buf, data_len);

        if (buf[data_len] != (crc>>24) 
            || buf[data_len+1] != ((crc>>16)&0x000000FF) 
            || buf[data_len+2] != ((crc>>8)&0x000000FF) 
            || buf[data_len+3] != (crc & 0x000000FF))
            return -1;

        command |= buf[3] << 8;
        command |= buf[4];

        if (command == ACK) {
            node = buf[7];
            return node;
        }
        delay(700);
    }

    return -1;
}