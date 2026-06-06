/**
 * Solar Tracker Control - JavaScript API Client
 * Примеры управления трекером через REST API
 * 
 * Использование в браузере или Node.js (с npm install node-fetch)
 */

class SolarTrackerClient {
    constructor(host = "192.168.4.1", port = 80) {
        this.baseUrl = `http://${host}:${port}`;
        this.timeout = 5000;
    }

    /**
     * Получить текущее состояние тракера
     * @returns {Promise<Object>}
     */
    async getStatus() {
        try {
            const response = await fetch(`${this.baseUrl}/api/status`);
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            return await response.json();
        } catch (error) {
            console.error("❌ Ошибка подключения:", error.message);
            return null;
        }
    }

    /**
     * Повернуть панели влево
     * @returns {Promise<boolean>}
     */
    async moveLeft() {
        try {
            const response = await fetch(`${this.baseUrl}/api/cmd/left`, {
                method: 'POST'
            });
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            console.log("✓ Команда: Поворот влево");
            return true;
        } catch (error) {
            console.error("❌ Ошибка:", error.message);
            return false;
        }
    }

    /**
     * Повернуть панели вправо
     * @returns {Promise<boolean>}
     */
    async moveRight() {
        try {
            const response = await fetch(`${this.baseUrl}/api/cmd/right`, {
                method: 'POST'
            });
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            console.log("✓ Команда: Поворот вправо");
            return true;
        } catch (error) {
            console.error("❌ Ошибка:", error.message);
            return false;
        }
    }

    /**
     * Переключить режим (AUTO <-> MANUAL)
     * @returns {Promise<boolean>}
     */
    async toggleMode() {
        try {
            const response = await fetch(`${this.baseUrl}/api/cmd/mode`, {
                method: 'POST'
            });
            if (!response.ok) throw new Error(`HTTP ${response.status}`);
            console.log("✓ Команда: Переключение режима");
            return true;
        } catch (error) {
            console.error("❌ Ошибка:", error.message);
            return false;
        }
    }

    /**
     * Получить название режима
     * @param {number} modeId - ID режима
     * @returns {string}
     */
    getModeName(modeId) {
        const modes = { 0: "AUTO", 1: "MANUAL", 2: "CALIBRATION" };
        return modes[modeId] || "UNKNOWN";
    }

    /**
     * Получить название статуса мотора
     * @param {number} motorId - ID мотора
     * @returns {string}
     */
    getMotorStatus(motorId) {
        const statuses = { 0: "STOP", 1: "LEFT ↙", 2: "RIGHT ↗" };
        return statuses[motorId] || "UNKNOWN";
    }

    /**
     * Вывести красивый статус
     */
    async printStatus() {
        const status = await this.getStatus();
        if (status) {
            const separator = "=".repeat(50);
            console.log("\n" + separator);
            console.log("  🌞 SOLAR TRACKER STATUS");
            console.log(separator);
            console.log(`  Left Panel Voltage:  ${status.vLeft.toFixed(1).padStart(6)} mV  ☀️`);
            console.log(`  Right Panel Voltage: ${status.vRight.toFixed(1).padStart(6)} mV  ☀️`);
            console.log(`  Difference:          ${(status.vLeft - status.vRight).toFixed(1).padStart(6)} mV`);
            console.log(`  Mode:                ${this.getModeName(status.mode)}`);
            console.log(`  Motor:               ${this.getMotorStatus(status.motorDir)}`);
            console.log(separator + "\n");
        } else {
            console.log("❌ Не удалось получить статус");
        }
    }
}

// ═══════════════════════════════════════════════════════════
// ПРИМЕРЫ ИСПОЛЬЗОВАНИЯ
// ═══════════════════════════════════════════════════════════

// Инициализация
const tracker = new SolarTrackerClient("192.168.4.1");

// Пример 1: Получить и вывести статус
async function example1() {
    console.log("📊 Пример 1: Получение статуса");
    await tracker.printStatus();
}

// Пример 2: Управление
async function example2() {
    console.log("🎮 Пример 2: Управление");
    await tracker.moveLeft();
    await new Promise(r => setTimeout(r, 1000));
    await tracker.printStatus();
}

// Пример 3: Переключение режима
async function example3() {
    console.log("🔄 Пример 3: Переключение режима");
    await tracker.toggleMode();
    await new Promise(r => setTimeout(r, 1000));
    await tracker.printStatus();
}

// Пример 4: Мониторинг
async function example4() {
    console.log("📈 Пример 4: Мониторинг в течение 5 секунд");
    for (let i = 0; i < 5; i++) {
        const status = await tracker.getStatus();
        if (status) {
            const diff = status.vLeft - status.vRight;
            console.log(
                `[${i + 1}] L:${status.vLeft.toFixed(1).padStart(6)} ` +
                `R:${status.vRight.toFixed(1).padStart(6)} ` +
                `Δ:${diff.toFixed(1).padStart(6)}  ` +
                `${tracker.getMotorStatus(status.motorDir)}`
            );
        }
        await new Promise(r => setTimeout(r, 1000));
    }
}

// Запуск примеров
async function runExamples() {
    console.log("🚀 Solar Tracker JavaScript API Client\n");
    
    try {
        await example1();
        await new Promise(r => setTimeout(r, 1000));
        
        await example2();
        await new Promise(r => setTimeout(r, 1000));
        
        await example3();
        await new Promise(r => setTimeout(r, 1000));
        
        await example4();
    } catch (error) {
        console.error("Ошибка при выполнении примеров:", error);
    }
}

// Выполнить примеры (раскомментируйте для запуска)
// runExamples();

// ═══════════════════════════════════════════════════════════
// HTML ПРИМЕР (используется в веб-интерфейсе)
// ═══════════════════════════════════════════════════════════
/*
<!-- Простой пример на HTML+JS -->
<!DOCTYPE html>
<html>
<head>
    <title>Solar Tracker Control</title>
</head>
<body>
    <button onclick="moveLeft()">← LEFT</button>
    <button onclick="moveRight()">RIGHT →</button>
    <button onclick="toggleMode()">AUTO/MANUAL</button>
    
    <pre id="status">Loading...</pre>
    
    <script>
        const tracker = new SolarTrackerClient("192.168.4.1");
        
        async function moveLeft() {
            await tracker.moveLeft();
        }
        
        async function moveRight() {
            await tracker.moveRight();
        }
        
        async function toggleMode() {
            await tracker.toggleMode();
        }
        
        async function updateStatus() {
            const status = await tracker.getStatus();
            if (status) {
                document.getElementById('status').textContent = JSON.stringify(status, null, 2);
            }
        }
        
        updateStatus();
        setInterval(updateStatus, 500);
    </script>
</body>
</html>
*/
