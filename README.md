# SmartIOF: Environment Guardian 🌾🍏

An IoT-based early spoilage detection system designed to monitor agricultural storage units. This system uses an ESP32 to track Temperature, Humidity, and Ethylene/CO2 levels, providing a real-time web dashboard for farmers.

## 🌟 Features
* **Real-time Monitoring:** Tracks temp, humidity, and gas levels via a local web server.
* **Early Spoilage Detection:** Converts raw MQ-135 gas data into an easy-to-read 1-5 Threat Level scale.
* **Glassmorphism UI:** Features a modern, mobile-responsive dashboard with an animated progress bar.
* **Smart Alert System:** * Level 1: Safe (Solid Green LED)
  * Level 2-4: Warning (Blinking Green LED)
  * Level 5 / High Temp: Spoilage Detected (Solid Red LED / Active Ventilation Trigger)

## 🛠️ Hardware Requirements
* ESP32 Development Board (NodeMCU)
* DHT22 Temperature & Humidity Sensor
* MQ-135 Air Quality / Gas Sensor
* 2x LEDs (Red and Green)
* Breadboard & Jumper Wires

## 🚀 How to Run
1. Open `SmartIOF_Monitor.ino` in the Arduino IDE.
2. Install the **DHT sensor library** by Adafruit via the Library Manager.
3. Update lines 10 & 11 with your local `WIFINAME` and `WIFIPASS`.
4. Upload to the ESP32.
5. Open the Serial Monitor (115200 baud) to find the local IP address.
6. Enter the IP address into any web browser connected to the same network to view the dashboard.
