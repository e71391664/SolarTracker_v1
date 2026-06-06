# 🏗️ Архитектура WiFi модуля Solar Tracker

## 📋 Обзор

WiFi модуль добавляет возможность управления Solar Tracker через интернет-браузер на базе ESP32-C3 встроенного WiFi.

## 🏛️ Компоненты системы

```
┌─────────────────────────────────────────────────────────────┐
│                    БРАУЗЕР (Телефон/ПК)                    │
│              (HTML5 + CSS3 + JavaScript)                    │
└────────────────────────┬────────────────────────────────────┘
                         │ HTTP (REST API)
                         ▼
┌─────────────────────────────────────────────────────────────┐
│               WebServer (ESP32 встроенный)                  │
│  ┌──────────────────────────────────────────────────────┐  │
│  │  Handler: GET  /              → HTML Page          │  │
│  │  Handler: GET  /api/status    → JSON Data          │  │
│  │  Handler: POST /api/cmd/left  → Motor Control      │  │
│  │  Handler: POST /api/cmd/right → Motor Control      │  │
│  │  Handler: POST /api/cmd/mode  → Mode Toggle        │  │
│  └──────────────────────────────────────────────────────┘  │
└────────────────────────┬────────────────────────────────────┘
                         │
        ┌────────────────┼────────────────┐
        ▼                ▼                ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│  Main Loop   │  │ Motor Control│  │ Sensor Data  │
│              │  │              │  │              │
│ • AUTO mode  │  │ • PWM L298N  │  │ • INA226 L   │
│ • MANUAL mode│  │ • Direction  │  │ • INA226 R   │
│ • CAL mode   │  │ • Stop timer │  │ • Averaging  │
└──────────────┘  └──────────────┘  └──────────────┘
```

## 📁 Файлы проекта

### Новые файлы

| Файл | Описание |
|------|---------|
| `include/webserver.h` | Заголовки веб-сервера |
| `src/webserver.cpp` | Реализация HTTP сервера, REST API, HTML интерфейс |
| `WIFI_INTERFACE.md` | Документация по использованию |
| `ARCHITECTURE.md` | Этот файл |
| `examples_python.py` | Примеры управления на Python |
| `examples_javascript.js` | Примеры управления на JavaScript |

### Модифицированные файлы

| Файл | Изменения |
|------|----------|
| `src/main.cpp` | Добавлены: `#include "webserver.h"`, вызовы `initWebServer()` и `handleWebServer()` |
| `platformio.ini` | Добавлены зависимости: `WiFi`, `WebServer` |

## 🔌 WiFi Конфигурация

### Access Point (AP) Режим
```cpp
#define WIFI_SSID     "SolarTracker"    // Имя сети
#define WIFI_PASSWORD "12345678"        // Пароль
#define WIFI_CHANNEL  1                 // Канал 2.4GHz
```

### Сетевые параметры
- **IP адрес**: 192.168.4.1
- **Маска подсети**: 255.255.255.0
- **Порт HTTP**: 80
- **Количество клиентов**: до 4 одновременно

## 🌐 REST API Структура

### 1. GET /
Возвращает HTML страницу интерфейса

**Content-Type**: `text/html; charset=utf-8`

### 2. GET /api/status
Получить текущее состояние трекера

**Ответ (JSON)**:
```json
{
  "vLeft": 345.2,      // Напряжение левой панели (мВ)
  "vRight": 342.1,     // Напряжение правой панели (мВ)
  "mode": 0,           // 0=AUTO, 1=MANUAL, 2=CALIBRATION
  "motorDir": 0        // 0=STOP, 1=LEFT, 2=RIGHT
}
```

**Периодичность**: 500 мс (автоматическое обновление в браузере)

### 3. POST /api/cmd/left
Команда поворота влево

**Параметры**: нет

**Ответ**: `"OK"` (200 OK)

**Действие**:
1. Запуск мотора влево
2. Установка таймера остановки (MOTOR_RUN_MS = 600 мс)
3. Мотор автоматически остановится через 600 мс

### 4. POST /api/cmd/right
Команда поворота вправо

**Параметры**: нет

**Ответ**: `"OK"` (200 OK)

**Действие**: Аналогично LEFT

### 5. POST /api/cmd/mode
Переключить режим

**Параметры**: нет

**Ответ**: `"OK"` (200 OK)

**Логика переключения**:
```
AUTO        →  MANUAL
MANUAL      →  AUTO  
CALIBRATION →  MANUAL
```

## 🎯 Интеграция с Main Loop

```cpp
void loop() {
    // 1. Обработка веб-запросов (приоритет)
    handleWebServer();
    
    // 2. Кнопки управления
    // ... обработка кнопок ...
    
    // 3. Замеры датчиков
    if (now - lastMeasure >= MEASURE_INTERVAL_MS) {
        readVoltages();
        // ...
    }
    
    // 4. Обновление мотора
    updateMotor(now);
}
```

### Приоритеты выполнения:
1. **WebServer** - обработка входящих HTTP запросов (non-blocking)
2. **Buttons** - опрос физических кнопок (10 мс debounce)
3. **Sensors** - чтение датчиков (500 мс интервал)
4. **Motor** - управление мотором (на основе режима)

## 🎨 HTML/CSS Интерфейс

### Структура страницы
```html
<container>
  ├── Header (название, версия)
  ├── Status Section
  │  ├── Panel Readings (левая/правая панель)
  │  ├── Difference Meter (визуализация)
  │  └── Motor Status
  ├── Controls Section
  │  ├── Button Grid (LEFT, RIGHT, MODE)
  │  └── Mode Indicator
  └── Update Time
```

### Стили
- **Gradient backgrounds**: Purple to blue
- **Responsive design**: Mobile-friendly
- **Smooth animations**: 0.3s transitions
- **Real-time updates**: 500 мс refresh rate

### JavaScript функции
```javascript
updateStatus()      // Получить данные с API
moveLeft()          // POST /api/cmd/left
moveRight()         // POST /api/cmd/right
toggleMode()        // POST /api/cmd/mode
```

## 📊 Производительность

### Потребление памяти
- HTML страница (PROGMEM): ~15 KB
- JSON буфер: ~0.5 KB
- WebServer объект: ~20 KB
- **Итого**: ~35-40 KB

### Пропускная способность
- Запрос статуса: ~100 bytes
- Ответ JSON: ~50 bytes
- Команда (POST): ~50 bytes
- **На 500 мс интервал**: ~1-2 KB/s

### Задержки
- Обработка запроса: <10 мс
- Отправка HTML: ~50-100 мс
- Отправка JSON: <1 мс
- **RTT (Round Trip Time)**: 50-150 мс в локальной сети

## 🔐 Безопасность

### Текущая реализация
- ❌ Нет шифрования (HTTP, не HTTPS)
- ❌ Нет аутентификации
- ✅ Пароль на подключение к WiFi

### Рекомендации для продакшена
- 🔒 Добавить HTTPS (потребует сертификата)
- 🔑 Добавить API токен
- 📡 Использовать STATION режим с моршифрованием
- 🛡️ Ограничить доступ по IP адресу

## 🧪 Тестирование

### Проверка подключения
```bash
ping 192.168.4.1
curl http://192.168.4.1/
curl http://192.168.4.1/api/status
```

### Отправка команд
```bash
curl -X POST http://192.168.4.1/api/cmd/left
curl -X POST http://192.168.4.1/api/cmd/right
curl -X POST http://192.168.4.1/api/cmd/mode
```

## 🚀 Будущие улучшения

- [ ] HTTPS и SSL сертификат
- [ ] Аутентификация и авторизация
- [ ] WebSocket для real-time обновлений
- [ ] История данных (логирование)
- [ ] Настройка параметров через WEB
- [ ] Интеграция с облачными сервисами
- [ ] Мобильное приложение
- [ ] Поддержка MQTT

## 📝 Техническая спецификация

| Параметр | Значение |
|----------|---------|
| Платформа | ESP32-C3 Super Mini |
| WiFi стандарт | 802.11 b/g/n (2.4 GHz) |
| Максимальная дальность | ~50-100 м (в помещении) |
| Скорость обновления | 500 мс |
| Максимальные клиенты | 4 |
| Время отклика | <150 мс |
| Потребление памяти | ~40 KB |
| Пропускная способность | ~2 KB/s |

## 🐛 Известные ограничения

1. **Только один браузер**: Может быть несколько клиентов, но управление может быть конфликтным
2. **Нет очереди команд**: Команды выполняются немедленно
3. **Нет логирования**: История действий не сохраняется
4. **Нет таймаутов**: Подключение может зависнуть
5. **Нет авторизации**: Любой может управлять трекером

## 📚 Ссылки

- [ESP32-C3 WebServer](https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer)
- [ESP32 WiFi API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html)
- [REST API Best Practices](https://restfulapi.net/)

---

**Версия**: 1.0.0  
**Дата обновления**: 2026-06-06  
**Автор**: Solar Tracker Development Team
