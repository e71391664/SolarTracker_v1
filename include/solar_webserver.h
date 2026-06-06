#ifndef SOLARTRACKER_WEBSERVER_H
#define SOLARTRACKER_WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "display.h"

// ─── WiFi Конфигурация ─────────────────────────────────────
#define WIFI_SSID     "SolarTracker"    // Точка доступа AP
#define WIFI_PASSWORD "12345678"        // Пароль (минимум 8 символов)
#define WIFI_CHANNEL  1

// ─── Публичные функции ─────────────────────────────────────
void initWebServer();
void handleWebServer();

// ─── Глобальные переменные из main.cpp ─────────────────────
extern Mode currentMode;
extern float vLeft;
extern float vRight;

#endif // SOLARTRACKER_WEBSERVER_H
