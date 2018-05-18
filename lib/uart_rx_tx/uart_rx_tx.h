#ifndef _UART_H_
#define _UART_H_

#define IP_COMMAND      0xFFFA
#define LEN_LOCAL       13
#define REQUEST_NODE    0x0104
#define ACK             0xF500

extern int node_number;

void UARTmessage(int command, char *data, int length, char destination);
uint8_t ipComponent(String ip, int n);
void sendIP(IPAddress ip);
int manageMessage(char* msg, int len);
int askNodeAddress();
#endif