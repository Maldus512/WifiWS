#ifndef _UART_H_
#define _UART_H_

#define IP_COMMAND      0xFFFA

extern int node_number;

void UARTmessage(int command, char *data, int length, char destination);
uint8_t ipComponent(String ip, int n);
void sendIP(IPAddress ip);
#endif