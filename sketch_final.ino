#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- Konfigurasi WiFi ---
const char* ssid = "Network";
const char* password = "password123";

// --- LTA API ---
const char* apiKey = "YGrvClJOSTqVt4YEuA9gIQ==";
const char* busStopCode = "52009";

// --- NTP ---
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600; // GMT+8 (Singapore)
const int daylightOffset_sec = 0;

// --- OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Struktur data bus
struct BusInfo {
  String serviceNo;
  int minutes;
};
BusInfo buses[30];
int busCount = 0;

int scrollIndex = 0;       // index scrolling
unsigned long lastScroll = 0;
const int scrollDelay = 2000; // ganti baris tiap 2 detik

// Variable update data per 1 menit
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 60000; // 1 menit (60.000 ms)

void showError(const char* msg) {
  Serial.println(msg);
  display.clearDisplay();
  display.setCursor(0, 20);
  display.println(msg);
  display.display();
}

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  
  display.clearDisplay();
  
  String line1 = "Bus Arrival";
  String line2 = "Information";
  String line3 = "Toa Payoh Int";
  String line4 = "(52009)";
  
  String lines[4] = { line1, line2, line3, line4 };
  
  int16_t x1, y1;
  uint16_t w, h;
  
  // Tinggi font ukuran 1 kira-kira 8 pixel
  int lineHeight = 10;
  
  // Cari total tinggi teks (4 baris)
  int totalHeight = 4 * lineHeight;
  
  // Mulai dari posisi Y biar vertikal center
  int startY = (display.height() - totalHeight) / 2;
  
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  for (int i = 0; i < 4; i++) {
    display.getTextBounds(lines[i], 0, 0, &x1, &y1, &w, &h);
    int16_t x = (display.width() - w) / 2;
    int16_t y = startY + i * lineHeight;
    display.setCursor(x, y);
    display.println(lines[i]);
  }
  
  display.display();
  delay(5000);

  WiFi.begin(ssid, password);
  int timeout = 0;

  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Connecting to WiFi");
    display.display();
    delay(3000);
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi connected!");
    display.display();
    delay(5000);
  } else {
    showError("WiFi Failed!");
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  waitForTimeSync(); // Memastikan waktu NTP sudah sinkron
  fetchBusData();      // ambil data pertama
  lastUpdate = millis(); // mulai timer
}

void waitForTimeSync() {
  time_t now;
  struct tm timeinfo;
  while (true) {
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year > (2016 - 1900)) { // cek kalau tahun sudah valid (>=2016)
      Serial.println("Time synchronized!");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Time synchronized!");
      display.display();
      delay(5000);
      break;
    }
    Serial.println("Waiting for NTP time...");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Waiting for NTP time...");
    display.display();
    delay(500);
  }
}

void fetchBusData() {
  HTTPClient http;
  String url = "https://datamall2.mytransport.sg/ltaodataservice/v3/BusArrival?BusStopCode=" + String(busStopCode);
  http.begin(url);
  http.addHeader("AccountKey", apiKey);
  http.addHeader("accept", "application/json");

  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    String payload = http.getString();

    StaticJsonDocument<8192> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      showError("Parse Error!");
      http.end();
      return;
    }

    JsonArray services = doc["Services"];
    busCount = 0;

    for (JsonObject service : services) {
      const char* serviceNo = service["ServiceNo"];
      const char* etaStr = service["NextBus"]["EstimatedArrival"];

      if (strlen(etaStr) > 0) {
        struct tm tm;
        if (strptime(etaStr, "%Y-%m-%dT%H:%M:%S", &tm)) {
          time_t etaTime = mktime(&tm);

          time_t now;
          time(&now);

          int minutes = (etaTime - now) / 60;
          if (busCount < 30) {
            buses[busCount].serviceNo = String(serviceNo);
            buses[busCount].minutes = minutes;
            busCount++;
          }
        }
      }
    }

    // sort nomor bus
    for (int i = 0; i < busCount - 1; i++) {
      for (int j = i + 1; j < busCount; j++) {
        if (buses[i].serviceNo.toInt() > buses[j].serviceNo.toInt()) {
          BusInfo temp = buses[i];
          buses[i] = buses[j];
          buses[j] = temp;
        }
      }
    }

    Serial.println("=== Bus Arrival Info ===");
    for (int i = 0; i < busCount; i++) {
      if (buses[i].minutes <= 0) {
        Serial.printf("Bus %s -> Arr\n", buses[i].serviceNo.c_str());
      } else {
        Serial.printf("Bus %s -> %d min\n", buses[i].serviceNo.c_str(), buses[i].minutes);
      }
    }
    Serial.println("=========================");
  } else {
    String err = "API Error: " + String(httpResponseCode);
    showError(err.c_str());
  }
  http.end();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    showError("WiFi Disconnected");
    delay(2000);
    return; // skip loop
  }

  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis(); // reset timer
    fetchBusData();   // update tiap 1 menit
  }

  // --- Tampilkan ke OLED dengan scroll ---
  if (millis() - lastScroll > scrollDelay && busCount > 0) {
    lastScroll = millis();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Bus Arrival Info");

    // tampilkan 6 baris mulai dari scrollIndex
    for (int i = 0; i < 6; i++) {
      int idx = scrollIndex + i;
      if (idx < busCount) {
        display.setCursor(0, (i + 1) * 10);
        if (buses[idx].minutes <= 0) {
          display.printf("Bus %s -> Arr\n", buses[idx].serviceNo.c_str());
        } else {
          display.printf("Bus %s -> %d min\n", buses[idx].serviceNo.c_str(), buses[idx].minutes);
        }
      }
    }
    display.display();

    scrollIndex++;
    if (scrollIndex >= busCount) scrollIndex = 0; // ulang lagi dari awal
  }

  delay(500); // kecil, biar scroll tetap smooth
}