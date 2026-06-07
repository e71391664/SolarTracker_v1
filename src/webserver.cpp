#include "solar_webserver.h"
#include "motor.h"
#include "display.h"

WebServer server(80);

// ═══════════════════════════════════════════════════════════
// HTML Интерфейс
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="ru">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Solar Tracker Control</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            padding: 40px;
            max-width: 500px;
            width: 100%;
        }
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 10px;
            font-size: 28px;
        }
        .subtitle {
            text-align: center;
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }
        .status-section {
            background: #f5f5f5;
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 30px;
        }
        .status-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin: 12px 0;
            padding: 8px 0;
            border-bottom: 1px solid #eee;
        }
        .status-item:last-child {
            border-bottom: none;
        }
        .status-label {
            color: #666;
            font-weight: 500;
            font-size: 14px;
        }
        .status-value {
            color: #333;
            font-weight: 600;
            font-size: 16px;
            background: white;
            padding: 6px 12px;
            border-radius: 8px;
            min-width: 80px;
            text-align: right;
        }
        .panel-readings {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 15px;
            margin-bottom: 20px;
        }
        .panel-card {
            background: #f9f9f9;
            border-radius: 12px;
            padding: 15px;
            text-align: center;
            border: 2px solid #e0e0e0;
            transition: all 0.3s ease;
        }
        .panel-card:hover {
            border-color: #667eea;
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.2);
        }
        .panel-label {
            font-size: 12px;
            color: #999;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 8px;
        }
        .panel-value {
            font-size: 24px;
            font-weight: bold;
            color: #667eea;
            margin-bottom: 4px;
        }
        .panel-unit {
            font-size: 12px;
            color: #999;
        }
        .controls-section {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            border-radius: 15px;
            padding: 25px;
            margin-bottom: 20px;
        }
        .controls-title {
            color: white;
            text-align: center;
            margin-bottom: 20px;
            font-size: 16px;
            font-weight: 600;
        }
        .button-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 12px;
            margin-bottom: 12px;
        }
        .btn {
            padding: 12px 20px;
            border: none;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        .btn:active {
            transform: scale(0.95);
        }
        .btn-left {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            color: white;
            grid-column: 1;
        }
        .btn-left:hover {
            box-shadow: 0 6px 20px rgba(245, 87, 108, 0.4);
        }
        .btn-right {
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            color: white;
            grid-column: 2;
        }
        .btn-right:hover {
            box-shadow: 0 6px 20px rgba(79, 172, 254, 0.4);
        }
        .btn-mode {
            background: linear-gradient(135deg, #43e97b 0%, #38f9d7 100%);
            color: white;
            grid-column: 1 / -1;
        }
        .btn-mode:hover {
            box-shadow: 0 6px 20px rgba(67, 233, 123, 0.4);
        }
        .btn-calibrate {
            background: linear-gradient(135deg, #ff9a8b 0%, #ff6a88 100%);
            color: white;
            grid-column: 1 / -1;
        }
        .btn-calibrate:hover {
            box-shadow: 0 6px 20px rgba(255, 106, 136, 0.4);
        }
        .mode-indicator {
            text-align: center;
            padding: 12px;
            background: rgba(255,255,255,0.2);
            border-radius: 8px;
            color: white;
            font-weight: 600;
            font-size: 13px;
        }
        .mode-auto {
            background: rgba(67, 233, 123, 0.3);
        }
        .mode-manual {
            background: rgba(255, 159, 64, 0.3);
        }
        .mode-calib {
            background: rgba(255, 87, 108, 0.3);
        }
        .status-notice {
            text-align: center;
            padding: 12px 16px;
            margin-top: 14px;
            background: rgba(255,255,255,0.12);
            border-radius: 10px;
            color: #fff;
            font-size: 13px;
            font-weight: 600;
            box-shadow: inset 0 0 0 1px rgba(255,255,255,0.1);
        }
        .diff-meter {
            background: white;
            border-radius: 12px;
            padding: 15px;
            margin-top: 20px;
        }
        .meter-label {
            font-size: 12px;
            color: #999;
            text-transform: uppercase;
            margin-bottom: 8px;
            text-align: center;
        }
        .meter-bar {
            height: 20px;
            background: #e0e0e0;
            border-radius: 10px;
            overflow: hidden;
            position: relative;
        }
        .meter-fill {
            height: 100%;
            background: linear-gradient(90deg, #f5576c, #ffea00, #43e97b);
            border-radius: 10px;
            transition: width 0.2s ease;
        }
        .meter-center {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            width: 2px;
            height: 100%;
            background: rgba(0,0,0,0.3);
        }
        .update-time {
            text-align: center;
            font-size: 12px;
            color: #999;
            margin-top: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>☀️ Solar Tracker</h1>
        <p class="subtitle">ESP32-C3 Tracking Control</p>
        
        <div class="status-section">
            <div class="panel-readings">
                <div class="panel-card">
                    <div class="panel-label">Left Calibrated</div>
                    <div class="panel-value" id="left-voltage-cal">0</div>
                    <div class="panel-unit">mV</div>
                </div>
                <div class="panel-card">
                    <div class="panel-label">Right Calibrated</div>
                    <div class="panel-value" id="right-voltage-cal">0</div>
                    <div class="panel-unit">mV</div>
                </div>
                <div class="panel-card">
                    <div class="panel-label">Left Raw</div>
                    <div class="panel-value" id="left-voltage-raw">0</div>
                    <div class="panel-unit">mV</div>
                </div>
                <div class="panel-card">
                    <div class="panel-label">Right Raw</div>
                    <div class="panel-value" id="right-voltage-raw">0</div>
                    <div class="panel-unit">mV</div>
                </div>
            </div>
            
            <div class="diff-meter">
                <div class="meter-label">Difference (Left - Right)</div>
                <div class="meter-bar">
                    <div class="meter-fill" id="diff-meter" style="width: 50%"></div>
                    <div class="meter-center"></div>
                </div>
                <div style="text-align: center; margin-top: 8px; font-size: 14px; font-weight: 600;">
                    <span id="diff-value">0</span> mV
                </div>
            </div>
            
            <div class="status-item" style="margin-top: 20px;">
                <span class="status-label">Motor Status</span>
                <span class="status-value" id="motor-status">STOP</span>
            </div>
        </div>
        
        <div class="controls-section">
            <div class="controls-title">Tracking Controls</div>
            <div class="button-grid">
                <button class="btn btn-left" onclick="moveLeft()">← LEFT</button>
                <button class="btn btn-right" onclick="moveRight()">RIGHT →</button>
                <button class="btn btn-mode" onclick="toggleMode()">AUTO / MANUAL</button>
                <button class="btn btn-calibrate" onclick="startCalibration()">CALIBRATE</button>
            </div>
            <div class="mode-indicator" id="mode-indicator" class="mode-auto">MODE: AUTO</div>
        </div>
        
        <div class="status-notice" id="status-notice">Готово</div>
        <div class="update-time">Last update: <span id="last-update">--:--:--</span></div>
    </div>

    <script>
        let autoUpdate = true;
        
        function updateStatus() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    // Напряжения
                    const leftCal = data.vLeft.toFixed(1);
                    const rightCal = data.vRight.toFixed(1);
                    const leftRaw = data.vLeftRaw.toFixed(1);
                    const rightRaw = data.vRightRaw.toFixed(1);
                    document.getElementById('left-voltage-cal').textContent = leftCal;
                    document.getElementById('right-voltage-cal').textContent = rightCal;
                    document.getElementById('left-voltage-raw').textContent = leftRaw;
                    document.getElementById('right-voltage-raw').textContent = rightRaw;
                    
                    // Разница
                    const diff = data.vLeft - data.vRight;
                    document.getElementById('diff-value').textContent = diff.toFixed(1);
                    
                    // Полоса разницы (0-100%, 50% в центре)
                    let percent = 50 + (diff / 500) * 50;  // Нормализация на 500мВ
                    percent = Math.max(0, Math.min(100, percent));
                    document.getElementById('diff-meter').style.width = percent + '%';
                    
                    // Режим
                    const modeText = data.mode === 0 ? 'AUTO' : (data.mode === 1 ? 'MANUAL' : 'CALIBRATION');
                    const modeClass = data.mode === 0 ? 'mode-auto' : (data.mode === 1 ? 'mode-manual' : 'mode-calib');
                    document.getElementById('mode-indicator').textContent = 'MODE: ' + modeText;
                    document.getElementById('mode-indicator').className = 'mode-indicator ' + modeClass;
                    
                    // Уведомления
                    const notification = data.statusMessage || 'Готово';
                    document.getElementById('status-notice').textContent = notification;
                    
                    // Статус мотора
                    const motorText = data.motorDir === 0 ? 'STOP' : (data.motorDir === 1 ? 'LEFT ↙' : 'RIGHT ↗');
                    document.getElementById('motor-status').textContent = motorText;
                    
                    // Время обновления
                    const now = new Date();
                    const timeStr = now.getHours().toString().padStart(2,'0') + ':' +
                                   now.getMinutes().toString().padStart(2,'0') + ':' +
                                   now.getSeconds().toString().padStart(2,'0');
                    document.getElementById('last-update').textContent = timeStr;
                })
                .catch(error => console.log('Update error:', error));
        }
        
        function moveLeft() {
            fetch('/api/cmd/left', {method: 'POST'})
                .then(() => setTimeout(updateStatus, 100));
        }
        
        function moveRight() {
            fetch('/api/cmd/right', {method: 'POST'})
                .then(() => setTimeout(updateStatus, 100));
        }
        
        function toggleMode() {
            fetch('/api/cmd/mode', {method: 'POST'})
                .then(() => setTimeout(updateStatus, 100));
        }

        function startCalibration() {
            fetch('/api/cmd/calibrate', {method: 'POST'})
                .then(() => setTimeout(updateStatus, 100));
        }

        // Начальное обновление и периодическое обновление
        updateStatus();
        setInterval(updateStatus, 500);
    </script>
</body>
</html>
)rawliteral";

// ═══════════════════════════════════════════════════════════
// REST API Обработчики
void handleStatusAPI() {
    String msg = statusMessage;
    msg.replace("\"", "\\\"");

    String json = "{";
    json += "\"vLeft\":" + String(vLeft, 1) + ",";
    json += "\"vRight\":" + String(vRight, 1) + ",";
    json += "\"vLeftRaw\":" + String(vLeftRaw, 1) + ",";
    json += "\"vRightRaw\":" + String(vRightRaw, 1) + ",";
    json += "\"mode\":" + String((int)currentMode) + ",";
    json += "\"motorDir\":" + String((int)getMotorDir()) + ",";
    json += "\"statusMessage\":\"" + msg + "\"";
    json += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Content-Type", "application/json");
    server.send(200, "application/json", json);
}

void handleCmdLeft() {
    if (currentMode == CALIBRATION) {
        server.send(200, "text/plain", "OK");
        return;
    }
    motorLeft();
    setMotorStopTime(millis() + MOTOR_RUN_MS);
    server.send(200, "text/plain", "OK");
}

void handleCmdRight() {
    if (currentMode == CALIBRATION) {
        server.send(200, "text/plain", "OK");
        return;
    }
    motorRight();
    setMotorStopTime(millis() + MOTOR_RUN_MS);
    server.send(200, "text/plain", "OK");
}

void handleCmdMode() {
    if (currentMode == CALIBRATION) {
        Serial.println("CMD_MODE ignored: калибровка в процессе");
        server.send(200, "text/plain", "OK");
        return;
    }

    if (currentMode == AUTO) {
        currentMode = MANUAL;
        motorStop();
    } else if (currentMode == MANUAL) {
        currentMode = AUTO;
    } else {
        currentMode = MANUAL;
    }
    server.send(200, "text/plain", "OK");
}

void handleCmdCalibrate() {
    if (currentMode != MANUAL) {
        statusMessage = "Калибровка возможна только из MANUAL";
        server.send(200, "text/plain", "OK");
        return;
    }

    if (currentMode == CALIBRATION) {
        statusMessage = "Калибровка уже запущена";
        server.send(200, "text/plain", "OK");
        return;
    }

    motorStop();
    modeAfterCalibration = MANUAL;
    currentMode = CALIBRATION;
    calibrationStartTime = millis();
    calibrationComboLockout = true;
    statusMessage = "Режим калибровки активирован через веб";

    server.send(200, "text/plain", "OK");
}

void handleRoot() {
    server.sendHeader("Content-Type", "text/html; charset=utf-8");
    server.send_P(200, "text/html", HTML_PAGE);
}

void handleNotFound() {
    server.send(404, "text/plain", "404 Not Found");
}

// ═══════════════════════════════════════════════════════════
// Инициализация WebServer
void initWebServer() {
    Serial.println("\n=== WiFi Mode (Access Point) ===");
    
    // Отключаем все предыдущие подключения
    WiFi.disconnect(true);  // true = отключить STA
    delay(100);
    
    // Включаем только AP режим
    WiFi.mode(WIFI_MODE_AP);
    delay(100);

    // Явно задаём локальный IP для AP и DHCP-сеть
    IPAddress localIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(localIP, gateway, subnet);
    delay(50);

    // Запускаем точку доступа
    bool apStarted = WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, false, 4);

    if (apStarted) {
        Serial.println("✓ AP режим включен успешно");
    } else {
        Serial.println("✗ ОШИБКА: не удалось включить AP режим!");
    }

    delay(500);  // Даём WiFi стабилизироваться

    // Получаем IP и выводим информацию
    IPAddress apIP = WiFi.softAPIP();
    Serial.printf("AP SSID: %s\n", WIFI_SSID);
    Serial.printf("AP Password: %s\n", WIFI_PASSWORD);
    Serial.printf("AP IP Address: %s\n", apIP.toString().c_str());
    Serial.printf("AP MAC Address: %s\n", WiFi.softAPmacAddress().c_str());
    
    // Проверяем количество подключённых устройств
    Serial.printf("Подключённых устройств: %d\n", WiFi.softAPgetStationNum());
    Serial.println("▶ Подключитесь к WiFi и откройте браузер на http://192.168.4.1");
    
    // Регистрация обработчиков
    server.on("/", handleRoot);
    server.on("/api/status", handleStatusAPI);
    server.on("/api/cmd/left", HTTP_POST, handleCmdLeft);
    server.on("/api/cmd/right", HTTP_POST, handleCmdRight);
    server.on("/api/cmd/mode", HTTP_POST, handleCmdMode);
    server.on("/api/cmd/calibrate", HTTP_POST, handleCmdCalibrate);
    server.onNotFound(handleNotFound);
    
    server.begin();
    Serial.println("✓ WebServer запущен на порту 80\n");
}

// ═══════════════════════════════════════════════════════════
// Обработка входящих запросов (вызывать в loop)
void handleWebServer() {
    server.handleClient();
}
