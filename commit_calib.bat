@echo off
cd /d c:\Users\sasha\Documents\PlatformIO\Projects\SolarTracker
"C:\Program Files\Git\cmd\git.exe" add src\main.cpp
"C:\Program Files\Git\cmd\git.exe" commit -m "Add panel calibration module with EEPROM storage"
"C:\Program Files\Git\cmd\git.exe" log --oneline -1
