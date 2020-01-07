#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <Ticker.h>

#define LED 2

struct Config
{
  char mqtt_server[40];
  char mqtt_port[6];
  char mqtt_sub[16];
  char mqtt_pub[16];
  char http_username[8] = "admin";
  char http_password[12] = "password20";
};

Config conf;

Ticker ticker;
// Ticker conn;

void ledToggle()
{
  bool state = digitalRead(LED);
  digitalWrite(LED, !state);
}

void chkConn()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    ESP.reset();
  }
}

void configModeCallback(AsyncWiFiManager *myWiFiManager)
{
  ticker.attach(0.2, ledToggle);
  Serial.println(F("Entered config mode"));
  Serial.print(F("SSID: "));
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.print(F("APIP: "));
  Serial.println(WiFi.softAPIP());
}

bool checkAuth(AsyncWebServerRequest *request) {
  return request->authenticate(conf.http_username, conf.http_password);
}

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  ticker.attach(0.5, ledToggle);

  // reset setting
  // wifiManager.resetSettings();

  wifiManager.setAPCallback(configModeCallback);

  // tampilkan atau matikan pesan debug di serial
  wifiManager.setDebugOutput(false);

  if (!wifiManager.autoConnect())
  {
    Serial.println(F("failed to connect and timed out"));
    // delay(500);
    ESP.reset();
    delay(1000);
  }

  // kalau sampai sini berarti udah konek
  Serial.println(F("connected...yeay!!"));

  Serial.print(F("local ip: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("gateway: "));
  Serial.println(WiFi.gatewayIP());
  Serial.print(F("subnetmask: "));
  Serial.println(WiFi.subnetMask());

  // mulai servernya
  server.begin();
  
  // tidak usah kedipkan led kalau dah konek
  ticker.detach();
  digitalWrite(LED, 0);

  // periksa koneksi setiap menit
  ticker.attach(60, chkConn);

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS)
    {
      request->send(200);
    }
    else
    {
      request->send(404);
    }
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hello, world");
  });

  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request) {
    if(!checkAuth(request))
        return request->requestAuthentication();
    // request->send(200, "text/plain", "Login Success!");
    request->redirect("/setup");
  });

  server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!checkAuth(request)) {
      request->redirect("/login");
    }
    request->send(200, "text/plain", "Halaman Setup");
  });

  server.on("/wifireset", HTTP_GET, [](AsyncWebServerRequest *request) {
    // request->redirect("http://" + WiFi.softAPIP().toString());
    wifiManager.resetSettings();
    ESP.reset();
    request->redirect("/");
  });
}

void loop()
{
}