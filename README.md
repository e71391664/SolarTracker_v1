# SolarTracker Project

## Описание
SolarTracker — это проект на основе PlatformIO для платы **Seeed XIAO ESP32C3**, разработанный для отслеживания положения солнца и управления солнечными панелями.

## Технические характеристики

### Платформа
- **Плата**: Seeed XIAO ESP32C3
- **Платформа**: ESP32 (Espressif)
- **Фреймворк**: Arduino

### Структура проекта
```
SolarTracker/
├── src/              # Исходный код приложения
│   └── main.cpp      # Основной файл программы
├── include/          # Заголовочные файлы
├── lib/              # Внешние библиотеки
├── test/             # Тесты
├── platformio.ini    # Конфигурация PlatformIO
└── README.md         # Данный файл
```

## Конфигурация сборки

### Флаги сборки
- Настраиваются в файле `platformio.ini` в разделе `[env:seeed_xiao_esp32c3]`

### Загрузка прошивки
- Использует стандартный протокол загрузки ESP32
- Подробнее: https://docs.platformio.org/page/projectconf.html

## Зависимости
- Библиотеки проекта находятся в директории `lib/`

## Документация
- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Arduino Framework](https://docs.espressif.com/projects/arduino-esp32/)
- [Seeed XIAO ESP32C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/)

## Разработка
Для начала разработки:
1. Установите PlatformIO
2. Откройте проект в VS Code
3. Отредактируйте код в папке `src/`
4. Используйте команду "Build" для сборки проекта
5. Используйте "Upload" для загрузки на плату

## Лицензия
Укажите лицензию проекта здесь.

## Автор
Разработчик: Sasha

---
*Последнее обновление: 29 мая 2026 г.*
