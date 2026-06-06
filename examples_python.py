"""
Solar Tracker Control - Python API Client
Примеры управления трекером через REST API
"""

import requests
import json
import time

class SolarTrackerClient:
    """Клиент для управления Solar Tracker через WiFi"""
    
    def __init__(self, host="192.168.4.1", port=80):
        """
        Инициализация клиента
        
        Args:
            host: IP адрес трекера (по умолчанию 192.168.4.1)
            port: Порт (по умолчанию 80)
        """
        self.base_url = f"http://{host}:{port}"
        self.timeout = 5
        
    def get_status(self):
        """
        Получить текущее состояние трекера
        
        Returns:
            dict: {'vLeft': float, 'vRight': float, 'mode': int, 'motorDir': int}
        """
        try:
            response = requests.get(f"{self.base_url}/api/status", timeout=self.timeout)
            response.raise_for_status()
            return response.json()
        except requests.RequestException as e:
            print(f"❌ Ошибка подключения: {e}")
            return None
    
    def move_left(self):
        """Повернуть панели влево"""
        try:
            response = requests.post(f"{self.base_url}/api/cmd/left", timeout=self.timeout)
            response.raise_for_status()
            print("✓ Команда: Поворот влево")
            return True
        except requests.RequestException as e:
            print(f"❌ Ошибка: {e}")
            return False
    
    def move_right(self):
        """Повернуть панели вправо"""
        try:
            response = requests.post(f"{self.base_url}/api/cmd/right", timeout=self.timeout)
            response.raise_for_status()
            print("✓ Команда: Поворот вправо")
            return True
        except requests.RequestException as e:
            print(f"❌ Ошибка: {e}")
            return False
    
    def toggle_mode(self):
        """Переключить режим (AUTO <-> MANUAL)"""
        try:
            response = requests.post(f"{self.base_url}/api/cmd/mode", timeout=self.timeout)
            response.raise_for_status()
            print("✓ Команда: Переключение режима")
            return True
        except requests.RequestException as e:
            print(f"❌ Ошибка: {e}")
            return False
    
    def get_mode_name(self, mode_id):
        """Получить название режима"""
        modes = {0: "AUTO", 1: "MANUAL", 2: "CALIBRATION"}
        return modes.get(mode_id, "UNKNOWN")
    
    def get_motor_status(self, motor_id):
        """Получить название статуса мотора"""
        statuses = {0: "STOP", 1: "LEFT ↙", 2: "RIGHT ↗"}
        return statuses.get(motor_id, "UNKNOWN")
    
    def print_status(self):
        """Вывести красивый статус"""
        status = self.get_status()
        if status:
            print("\n" + "="*50)
            print("  🌞 SOLAR TRACKER STATUS")
            print("="*50)
            print(f"  Left Panel Voltage:  {status['vLeft']:6.1f} mV  ☀️")
            print(f"  Right Panel Voltage: {status['vRight']:6.1f} mV  ☀️")
            print(f"  Difference:          {status['vLeft']-status['vRight']:6.1f} mV")
            print(f"  Mode:                {self.get_mode_name(status['mode'])}")
            print(f"  Motor:               {self.get_motor_status(status['motorDir'])}")
            print("="*50 + "\n")
        else:
            print("❌ Не удалось получить статус")


# ═══════════════════════════════════════════════════════════
# ПРИМЕРЫ ИСПОЛЬЗОВАНИЯ
# ═══════════════════════════════════════════════════════════

if __name__ == "__main__":
    # Создаем клиент
    tracker = SolarTrackerClient("192.168.4.1")
    
    print("🚀 Solar Tracker Python API Client\n")
    
    # Пример 1: Получить статус
    print("📊 Пример 1: Получение статуса")
    tracker.print_status()
    
    # Пример 2: Простые команды
    print("🎮 Пример 2: Управление")
    tracker.move_left()
    time.sleep(1)
    tracker.print_status()
    
    # Пример 3: Переключение режима
    print("🔄 Пример 3: Переключение режима")
    tracker.toggle_mode()
    time.sleep(1)
    tracker.print_status()
    
    # Пример 4: Мониторинг
    print("📈 Пример 4: Мониторинг в течение 5 секунд")
    for i in range(5):
        status = tracker.get_status()
        if status:
            diff = status['vLeft'] - status['vRight']
            print(f"[{i+1}] L:{status['vLeft']:6.1f} R:{status['vRight']:6.1f} Δ:{diff:6.1f}  {tracker.get_motor_status(status['motorDir'])}")
        time.sleep(1)
