#ifndef WEBSERVER_H
#define WEBSERVER_H

void setupWiFi();
void updateServer();
void setupServer();
byte hexToByte(String hex);
byte hexCharToNum(char letter);

#endif
