#ifndef WEBSERVER_H
#define WEBSERVER_H

void setupWiFi();
void updateServer();
void setupServer();
void sendInt(uint8_t value);
void sendString(String value);
void broadcastInt(String name, uint8_t value);
void broadcastString(String name, String value);

#endif
