#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <DHT.h>
#include <time.h>
#include <FastLED.h>

#define DHTPIN 27
#define DHTTYPE DHT22

#define DATA_PIN 21
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 60
#define BRIGHTNESS 255

#define LED_BUILTIN 2

const char *ssid = "Home";
const char *password = "zimmermann@jel@14";

const int ledBarre = 21;
const int ledLampe = 32;
const int PompePIN = 5;

const int Seconde = 1000;
const int Heure = Seconde * 3600;

bool etatLedBarre = 0;
bool etatLedBarreVoulu = 0;
bool etatLedLampe = 0;
bool etatLedLampeVoulu = 0;

unsigned long TempsTemporaire = 1000000000;
unsigned long previousLampe = 0;
unsigned long previousBarre = 0;
unsigned long ValeurTempsDeVapo = 30 * Seconde;
unsigned long TempsMinutes = 0;
String TempsActuel = "00:00";
String HeureActuel = "0";
String MinutesActuel = "0";
String SecondeActuel = "0";
int resultHeure = 0;
int resultMinutes = 0;
int resultSeconde = 0;

float temp = 20.0;

AsyncWebServer server(80);

DHT dht(DHTPIN, DHTTYPE);

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

CRGB leds[NUM_LEDS];

void printLocalTimeServeur()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    TempsActuel = "Erreur !";
    return;
  }
  HeureActuel = timeinfo.tm_hour;
  MinutesActuel = timeinfo.tm_min;
  SecondeActuel = timeinfo.tm_sec;
  TempsMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  TempsActuel = HeureActuel + ":" + MinutesActuel;
}

void sunrise()
{
  static const uint8_t sunriseLength = 30;
  static const uint8_t interval = (sunriseLength * 60) / 256;

  static uint8_t heatIndex = 0;

  CRGB color = ColorFromPalette(HeatColors_p, heatIndex);

  fill_solid(leds, NUM_LEDS, color);

  EVERY_N_SECONDS(interval)
  {
    if (heatIndex < 255)
    {
      heatIndex++;
    }
    if (heatIndex == 255)
    {
      heatIndex = 0;
    }
  }
}

void sunset()
{
  static const uint8_t sunriseLength = 30;
  static const uint8_t interval = (sunriseLength * 60) / 256;

  static uint8_t heatIndex = 255;

  CRGB color = ColorFromPalette(HeatColors_p, heatIndex);

  fill_solid(leds, NUM_LEDS, color);

  EVERY_N_SECONDS(interval)
  {
    if (heatIndex > 0)
    {
      heatIndex--;
    }
    if (heatIndex == 0)
    {
      heatIndex = 255;
    }
  }
}

void ActivationPompe()
{
  digitalWrite(PompePIN, LOW);
  delay(ValeurTempsDeVapo);
  digitalWrite(PompePIN, HIGH);
}

// void LumiereDeJournee()
// {
//   FastLED.setBrightness(255);
//   fill_solid(leds, NUM_LEDS, CRGB(255, 255, 255));
// }

// void LumiereDeNuit()
// {
//   FastLED.setBrightness(255);
//   fill_solid(leds, NUM_LEDS, CRGB(25, 25, 112));
// }

void setup()
{

  //----------------------------------------------------Serial

  Serial.begin(115200);
  Serial.println("\n");

  pinMode(ledBarre, OUTPUT);
  digitalWrite(ledBarre, HIGH);

  pinMode(ledLampe, OUTPUT);
  digitalWrite(ledLampe, HIGH);

  pinMode(PompePIN, OUTPUT);
  digitalWrite(PompePIN, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);

  dht.begin();

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip)
      .setDither(BRIGHTNESS < 255);

  //----------------------------------------------------SPIFFS

  if (!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while (file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }

  //----------------------------------------------------WIFI

  WiFi.begin(ssid, password);
  Serial.print("Tentative de connexion...");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }

  Serial.println("\n");
  Serial.println("Connexion etablie!");
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.localIP());

  //----------------------------------------------------SERVER

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/w3.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  server.on("/jquery-3.5.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/jquery-3.5.1.min.js", "text/javascript");
  });

  server.on("/VapoParJours", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ValeurVapoParJours", true))
    {
      String message;
      message = request->getParam("ValeurVapoParJours", true)->value();
      ValeurTempsDeVapo = Seconde * message.toInt();
    }
    request->send(204);
  });

  server.on("/lireTemp", HTTP_GET, [](AsyncWebServerRequest *request) {
    float val = dht.readTemperature();
    String temperature = String(val) + " °C";
    request->send(200, "text/plain", temperature);
  });

  server.on("/lireHumi", HTTP_GET, [](AsyncWebServerRequest *request) {
    float valHumi = dht.readHumidity();
    String Humidite = String(valHumi) + " %";
    request->send(200, "text/plain", Humidite);
  });

  server.on("/lireTempEau", HTTP_GET, [](AsyncWebServerRequest *request) {
    float valTempEau = 0;
    String TempEau = String(valTempEau) + " °C";
    request->send(200, "text/plain", TempEau);
  });

  server.on("/lireTemps", HTTP_GET, [](AsyncWebServerRequest *request) {
    String TempsActu = TempsActuel;
    request->send(200, "text/plain", TempsActu);
  });

  server.on("/onBarre", HTTP_GET, [](AsyncWebServerRequest *request) {
    etatLedBarreVoulu = 1;
    request->send(204);
  });

  server.on("/offBarre", HTTP_GET, [](AsyncWebServerRequest *request) {
    etatLedBarreVoulu = 0;
    digitalWrite(ledBarre, LOW);
    etatLedBarre = 0;
    request->send(204);
  });

  server.on("/onPompe", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(PompePIN, LOW);
    delay(ValeurTempsDeVapo);
    digitalWrite(PompePIN, HIGH);
    request->send(204);
  });

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  server.begin();

  Serial.println("Serveur actif !");
}

void loop()
{
  printLocalTimeServeur();
  delay(2 * Seconde);

  if (etatLedBarreVoulu == 1)
  {
    int PreTemps = TempsMinutes;
    fill_solid(leds, NUM_LEDS, CRGB(255, 255, 255));
    if (PreTemps == TempsMinutes + 30)
    {
      etatLedBarreVoulu = 0;
      FastLED.clear();
    }
  }

  int resultHeure = HeureActuel.toInt();
  int resultMinutes = MinutesActuel.toInt();
  int resultSeconde = SecondeActuel.toInt();

  if (resultHeure == 8 && resultMinutes < 30)
  {
    sunrise();
  }

  if (resultHeure == 20 && resultMinutes < 30)
  {
    sunset();
  }

  if (resultHeure == 2 || resultHeure == 8 || resultHeure == 14 || resultHeure == 20)
  {
    if (resultMinutes == 0 && resultSeconde < 5)
    {
      ActivationPompe();
    }
  }
  if (TempsMinutes > 510 && TempsMinutes < 1200)
  {
    fill_solid(leds, NUM_LEDS, CRGB(255, 255, 255));
  }

  if (TempsMinutes < 480 && TempsMinutes > 1230)
  {
    FastLED.clear();
  }

  FastLED.show();
}
