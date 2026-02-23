#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>

void fetchServerVersion();
void processOTA(bool force);

#endif
