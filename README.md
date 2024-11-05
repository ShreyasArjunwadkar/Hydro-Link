# Hydro-Link
Noninvasive water-level monitoring system
HydroLink is a campus-wide water level monitoring system designed to provide real-time data on water levels across multiple locations. Leveraging WiFi and RF communication, HydroLink ensures comprehensive coverage and efficient water management. The system integrates an ESP32 microcontroller, Firebase database, and a custom Android app to make monitoring accessible and accurate.

Features 🌟
Real-Time Monitoring 📈: View current water levels instantly via the Android app.
Wide Coverage 🌐: Combines WiFi and RF communication to cover the entire campus.
Data Storage with Firebase ☁️: Uses Firebase as a cloud database for storing water level data and historical records.
ESP32 Microcontroller 🛠️: Handles data collection and transmission efficiently.
User-Friendly Interface 📲: Android app provides a simple, intuitive display of water level data.
System Components 🔧
ESP32 Microcontroller: Reads water level data from ultrasonic sensors and transmits it over WiFi.
RF Communication: Extends the coverage by allowing data to travel across wider areas where WiFi may be limited.
Firebase Database: Stores water level data, enabling real-time updates and historical data tracking.
Android Application: Displays water level information in a clear and interactive interface for users.
How It Works ⚙️
Data Collection: The ESP32 collects water level data from ultrasonic sensors.
Data Transmission: Collected data is sent to Firebase via WiFi or RF communication, depending on the location.
Data Display: The Android app retrieves data from Firebase and displays real-time water levels to the user.

HydroLink is an innovative step toward smarter water management solutions. Contributions and suggestions are welcome!
