#ifndef _UART_H_
#define _UART_H_
void UARTmessage(int command, char *data, int length, char destination);
uint8_t ipComponent(String ip, int n);
void sendIP(IPAddress ip);
#endif