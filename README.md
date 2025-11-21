
# Ch4melBa1t
*Made by b4sh0ne*

A proof-of-concept project for the ESP8266 that creates a sophisticated captive portal. It scans for the strongest nearby Wi-Fi network, mimics its SSID, and serves a fake login page to capture credentials for educational and security research purposes.

---

## ⚠️ IMPORTANT: Change the Default Password!

Before deploying this project, you **MUST** change the default master password. Failure to do so will leave your device's admin panel completely exposed.

1.  Open the `main.ino` file in the Arduino IDE.
2.  Find the following line of code:
    ```cpp
    const char *web_password = "***"; //CHANGE ME
    ```
3.  Replace `"***"` with your own strong, secret password.

---

## Features

-   **Dynamic SSID Cloning**: Automatically scans and mimics the SSID of the strongest nearby Wi-Fi network.
-   **Modern UI Captive Portal**: Redirects all DNS requests to a clean, mobile-friendly login page.
-   **Credential Logging**: Saves captured usernames, passwords, and client User-Agent strings to the onboard flash memory (LittleFS).
-   **Web-Based Dashboard**: A password-protected interface to view and manage captured data.
-   **Data Export**: Download all captured logs as a single `.csv` file.
-   **Log Management**: Securely clear all stored logs from the web dashboard.

## How It Works

1.  **Scan**: The ESP8266 scans for all available Wi-Fi networks and identifies the one with the strongest signal (highest RSSI).
2.  **Mimic**: It then starts its own Access Point, using the SSID of the strongest network to appear as a legitimate hotspot.
3.  **Capture**: A DNS server runs on the device, redirecting any connected client to the captive portal login page, regardless of the address they try to visit.
4.  **Log**: When a user enters their credentials on the fake login page, the data is saved to a `logs.txt` file on the device. The user is shown a generic "connection failed" message.
5.  **Administer**: You can access a hidden dashboard by navigating to `/dashboard` and authenticating with the master password. From here, you can view, download, or delete the captured data.

## Getting Started

### Prerequisites

-   **Hardware**: An ESP8266-based board (e.g., NodeMCU, Wemos D1 Mini).
-   **Software**:
    -   Arduino IDE
    -   ESP8266 Core for Arduino
    -   LittleFS File System Uploader Tool (for managing files if needed)

### Setup & Installation

1.  Install the required software and board definitions in your Arduino IDE.
2.  Open the `main.ino` sketch.
3.  **Change the master password** as described in the warning section above.
4.  Select your ESP8266 board and the correct COM port from the `Tools` menu.
5.  Upload the sketch to your ESP8266.

## Accessing the Dashboard

1.  Power on the ESP8266. It will begin broadcasting the cloned Wi-Fi network.
2.  Connect a device (like a phone or laptop) to this network. The captive portal page should appear automatically.
3.  To access the admin panel, manually open a web browser and go to:
    ```
    http://192.168.4.1/dashboard
    ```
4.  The page will look like the standard login page. Enter your master password in the **Password** field (the username can be anything) and click **Connect**.
5.  You will be redirected to the admin dashboard where you can manage the logs.

---

### Disclaimer

This tool is intended for **educational and security research purposes only**. The author is not responsible for any misuse or damage caused by this program. Unauthorized use of this tool against networks or individuals is illegal and unethical. Always obtain explicit permission before conducting any security tests.
