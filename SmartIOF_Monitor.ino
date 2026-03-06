#include <WiFi.h>
#include <WebServer.h>
#include "DHT.h"

// ==========================================
// 1. CONFIGURATION 
// ==========================================
const char* ssid = "WIFINAME";       
const char* password = "WIFIPASS"; 

// ==========================================
// 2. PIN DEFINITIONS & SETTINGS
// ==========================================
#define DHTPIN 4        
#define DHTTYPE DHT22   
#define GAS_PIN 34      
#define GREEN_LED 18    
#define RED_LED 19      

// Thresholds
float tempThreshold = 40.5; 

// Global Objects
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

// Timers for non-blocking logic
unsigned long previousMillisLED = 0;
unsigned long previousMillisSensor = 0;
int ledState = LOW;

// Variables to store sensor data
float t = 0.0;
float h = 0.0;
int gasRaw = 0;
int level = 1;

// Helper: Convert Raw Gas Value to Level 1-5
int getEthyleneLevel(int rawValue) {
  if (rawValue < 800) return 1;       // Safe
  else if (rawValue < 1000) return 2;  // Low Warning
  else if (rawValue < 1400) return 3;  // High Warning
  else if (rawValue < 1800) return 4;  // Critical Warning
  else return 5;                       // SPOILAGE DETECTED
}

void setup() {
  Serial.begin(115200);
  
  // Initialize Hardware
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GAS_PIN, INPUT);
  dht.begin();

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected!");
  Serial.print("DASHBOARD URL: http://");
  Serial.println(WiFi.localIP());

  // Start Server
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient(); // Keep website responsive
  unsigned long currentMillis = millis();

  // --- A. READ SENSORS (Every 200ms to allow smooth blinking) ---
  if (currentMillis - previousMillisSensor >= 200) {
    previousMillisSensor = currentMillis;
    
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();
    
    // Only update if sensor read is valid (avoids NaN errors)
    if (!isnan(newT)) t = newT;
    if (!isnan(newH)) h = newH;
    
    gasRaw = analogRead(GAS_PIN);
    level = getEthyleneLevel(gasRaw);
  }

  // --- B. LED & ALERT LOGIC ---
  
  // PRIORITY 1: CRITICAL (Level 5 OR High Heat) -> Solid RED
  if (level >= 5 || t > tempThreshold) {
    digitalWrite(RED_LED, HIGH);   
    digitalWrite(GREEN_LED, LOW);  
  } 
  
  // PRIORITY 2: SAFE (Level 1) -> Solid GREEN
  else if (level == 1) {
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH); 
  }
  
  // PRIORITY 3: WARNING LOW (Level 2) -> Slow Blink GREEN (600ms)
  else if (level == 2) {
    digitalWrite(RED_LED, LOW);
    if (currentMillis - previousMillisLED >= 600) { 
      previousMillisLED = currentMillis;
      ledState = (ledState == LOW) ? HIGH : LOW; 
      digitalWrite(GREEN_LED, ledState);
    }
  }
  
  // PRIORITY 4: WARNING HIGH (Level 3 & 4) -> Fast Blink GREEN (150ms)
  else { 
    digitalWrite(RED_LED, LOW);
    if (currentMillis - previousMillisLED >= 150) { 
      previousMillisLED = currentMillis;
      ledState = (ledState == LOW) ? HIGH : LOW; 
      digitalWrite(GREEN_LED, ledState);
    }
  }
}

// --- C. WEB DASHBOARD (HTML/CSS) ---
void handleRoot() {
  String status, statusColor;
  int percentage = level * 20; // 20%, 40%, 60%, 80%, 100%

  // Determine Status Text & Color
  if (level == 1) { status = "SAFE ENVIRONMENT"; statusColor = "#2ecc71"; } 
  else if (level == 2) { status = "WARNING (Low)"; statusColor = "#f1c40f"; } 
  else if (level == 3) { status = "WARNING (Rising)"; statusColor = "#e67e22"; } 
  else if (level == 4) { status = "DANGER RISING"; statusColor = "#d35400"; } 
  else { status = "SPOILAGE DETECTED"; statusColor = "#e74c3c"; } 

  // HTML Construction
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Environment Guardian</title>";
  html += "<style>";
  
  // 1. Wallpaper & Body
  html += "body { font-family: 'Segoe UI', sans-serif; text-align: center; margin: 0; padding: 20px; color: white;";
  html += "background-image: url('https://images.unsplash.com/photo-1518173946687-a4c8892bbd9f?q=80&w=1000&auto=format&fit=crop');";
  html += "background-size: cover; background-position: center; background-attachment: fixed; min-height: 100vh; }";
  
  // 2. Glassmorphism Cards
  html += ".container { max-width: 400px; margin: auto; padding-top: 20px; }";
  html += ".card { background: rgba(0, 0, 0, 0.75); backdrop-filter: blur(8px); padding: 25px; border-radius: 20px; border: 1px solid rgba(255,255,255,0.1); margin-bottom: 20px; box-shadow: 0 8px 32px 0 rgba(0,0,0,0.37); }";
  
  // 3. Animated Progress Bar CSS
  html += ".progress-container { width: 100%; background-color: rgba(255,255,255,0.2); border-radius: 15px; margin: 25px 0; height: 35px; overflow: hidden; position: relative; }";
  html += ".progress-bar { height: 100%; width: " + String(percentage) + "%; background-color: " + statusColor + ";";
  html += "background-image: linear-gradient(45deg,rgba(255,255,255,.15) 25%,transparent 25%,transparent 50%,rgba(255,255,255,.15) 50%,rgba(255,255,255,.15) 75%,transparent 75%,transparent);";
  html += "background-size: 1rem 1rem; transition: width 0.6s ease; animation: stripes 1s linear infinite; box-shadow: 0 0 10px " + statusColor + "; }";
  html += "@keyframes stripes { 0% { background-position: 1rem 0; } 100% { background-position: 0 0; } }";

  html += "</style><meta http-equiv='refresh' content='2'></head><body>";
  
  html += "<div class='container'>";
  html += "<h1 style='text-shadow: 2px 2px 5px black; letter-spacing: 1px;'>Environment Guardian</h1>";
  
  // -- MAIN CARD --
  html += "<div class='card'>";
  html += "<h2 style='color:" + statusColor + "; margin-top:0; text-shadow: 1px 1px 2px black;'>" + status + "</h2>";
  
  // Progress Bar
  html += "<div style='display:flex; justify-content:space-between; font-size:14px; opacity:0.8;'><span>Safe</span><span>Critical</span></div>";
  html += "<div class='progress-container'><div class='progress-bar'></div></div>";
  html += "<div style='font-size:20px; font-weight:bold;'>Threat Level: " + String(level) + " / 5</div>";
  
  html += "<hr style='border-color:rgba(255,255,255,0.2); margin: 25px 0;'>";
  
  // Sensor Grid
  html += "<div style='display:flex; justify-content:space-around;'>";
  html += "<div><div style='font-size:32px; font-weight:bold;'>" + String(t, 1) + "&deg;C</div><div style='font-size:13px; opacity:0.7'>Temperature</div></div>";
  html += "<div><div style='font-size:32px; font-weight:bold;'>" + String(h, 0) + "%</div><div style='font-size:13px; opacity:0.7'>Humidity</div></div>";
  html += "</div>";
  
  html += "<div style='margin-top:20px; font-size:12px; opacity:0.4;'>Sensor Raw Input: " + String(gasRaw) + "</div>";
  html += "</div>"; // End Card
  
  html += "<div style='font-size:12px; text-shadow: 1px 1px 2px black;'>System Online • SmartIOF v1.0</div>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}
