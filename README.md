
# 🦎 Ch4melBa1t

*Made by b4sh0ne*

A proof-of-concept project for **ESP32 & ESP8266** that creates a sophisticated captive portal. It scans for the strongest nearby Wi-Fi network, mimics its SSID with a special invisible character, and serves a sleek, fake login page to capture credentials for educational and security research purposes.

---

## ⚠️ IMPORTANT: Change the Default Password!

Before deploying, you **MUST** change the default master password. Failure to do so will leave your device's admin panel completely exposed.

1.  Open `esp32.ino` or `esp8266.ino` in the Arduino IDE.
2.  Find this line:
    ```cpp
    const char *web_password = "***"; //CHANGE ME
    ```
3.  Replace `"***"` with your own strong, secret password.

---

## ✨ Features

-   **📡 Dynamic SSID Cloning**: Automatically scans and mimics the SSID of the strongest nearby Wi-Fi network.
-   **👻 SSID Cloaking / Evil Twin Evasion**: Appends a Zero-Width Joiner (`U+200B`) or similar non-printable character to the cloned SSID. This technique, often referred to as SSID Cloaking, creates a visually identical but technically distinct network name, allowing the bypass of certain operating system-level "Evil Twin" detection mechanisms (e.g., on iOS/macOS) which prevent connections to seemingly identical, unauthorized networks.
-   **📱 Modern UI Captive Portal**: Redirects all DNS requests to a clean, mobile-friendly login page inspired by modern OS designs.
-   **💾 Credential Logging**: Saves captured usernames, passwords, and client User-Agent strings to the onboard flash memory (LittleFS).
-   **👑 Web-Based Dashboard**: A password-protected interface to view and manage captured data.
-   **📄 Data Export**: Download all captured logs as a single `.csv` file.
-   **🗑️ Log Management**: Securely clear all stored logs from the web dashboard.
-   **🌐 Captive Portal Detection Bypass**: Hosts common captive portal detection endpoints (e.g., `/generate_204`, `/hotspot-detect.html`) to ensure seamless and rapid connection by client devices, mimicking a legitimate network environment.

## ⚙️ How It Works

1.  **Scan**: The ESP board scans for all available Wi-Fi networks and identifies the one with the strongest signal (highest RSSI).
2.  **Mimic**: It starts its own Access Point, using the SSID of the strongest network plus an invisible character to appear as a legitimate (but separate) hotspot.
3.  **Capture**: A DNS server runs on the device, redirecting any connected client to the captive portal login page, no matter what address they try to visit.
4.  **Log**: When a user enters their credentials, the data is saved to a `logs.txt` file on the device. The user is shown a generic "connection failed" message.
5.  **Administer**: You can access a hidden dashboard by navigating to `/dashboard` and authenticating with your master password. From there, you can view, download, or delete the captured data.

## 🚀 Getting Started

### Prerequisites

-   **Hardware**: An ESP32 or ESP8266-based board (e.g., NodeMCU, Wemos D1 Mini).
-   **Software**:
    -   Arduino IDE
    -   ESP32/ESP8266 Core for Arduino
    -   LittleFS File System Uploader Tool

### Setup & Installation

1.  Install the required software and board definitions in your Arduino IDE.
2.  Open the `esp32.ino` or `esp8266.ino` sketch.
3.  **Change the master password** as described in the warning section above.
4.  Select your ESP board and the correct COM port from the `Tools` menu.
5.  Upload the sketch to your device.

## 🔑 Accessing the Dashboard

1.  Power on the ESP. It will begin broadcasting the cloned Wi-Fi network.
2.  Connect a device (like a phone or laptop) to this network. The captive portal page should appear automatically.
3.  Within the captive portal, enter your master password into the **Password** field (the username/email field is not required for dashboard access) and click **Connect**. Successful authentication will redirect you to the admin dashboard.
5.  You will be redirected to the admin dashboard.

---

### Disclaimer

This tool is intended for **educational and security research purposes only**. The author is not responsible for any misuse or damage caused by this program. Unauthorized use of this tool against networks or individuals is illegal and unethical. Always obtain explicit permission before conducting any security tests.
