#ifndef SOLARTRACKER_WEBSERVER_CONFLICT_H
#define SOLARTRACKER_WEBSERVER_CONFLICT_H

#include <Arduino.h>
#include <WiFi.h>
#include_next <WebServer.h>

// ─── WiFi Конфигурация ─────────────────────────────────────
#define WIFI_SSID     "SolarTracker"    // Точка доступа AP
#define WIFI_PASSWORD "12345678"        // Пароль (минимум 8 символов)
#define WIFI_CHANNEL  1

// ─── Публичные функции ─────────────────────────────────────
void initWebServer();
void handleWebServer();

// ─── Функции для передачи данных из main.cpp ───────────────
// Эти переменные должны быть доступны из main.cpp
extern float g_vLeft;        // Напряжение левой панели (мВ)
extern float g_vRight;       // Напряжение правой панели (мВ)
extern int g_currentMode;    // Режим: 0=AUTO, 1=MANUAL, 2=CALIBRATION
extern int g_motorDir;       // Направление мотора: 0=STOP, 1=LEFT, 2=RIGHT

#endif // SOLARTRACKER_WEBSERVER_CONFLICT_H
