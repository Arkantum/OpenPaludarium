//////----------Declaration Librarie----------//////

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <DHT.h>
#include <time.h>
#include <FastLED.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//////----------Declaration Librarie----------//////

//////----------Declaration PIN----------//////

#define Relais_13 13 //Relai
#define Relais_32 32 //Relai
#define Relais_33 33 //Relai
#define Relais_18 18 //Relai
#define Relais_19 19 //Relai
#define Relais_23 23 //Relai

#define Thermo_16 16 //dht22 (temp and humidity)
#define Thermo_36 36 //dht22 (temp and humidity)
#define Thermo_35 35 //DS18B20 (temp only)
#define Thermo_17 17 //DS18B20 (temp only)

#define LED_4 4 //ws2812b led
#define LED_5 5 //ws2812b led

#define Niveau_2 2 //Capteur analogique pour le niveau d'eau

#define DHTPIN 27
#define DHTPIN2 18
#define ONE_WIRE_BUS 32
#define DATA_PIN 21
#define LED_BUILTIN 2
#define PompePIN 5

//////----------Declaration PIN----------//////

//////----------Setup sensors----------//////

#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define DHTTYPE2 DHT11
DHT dht2(DHTPIN2, DHTTYPE2);

DHT DHT_Thermo_16(Thermo_16, DHT22);
DHT DHT_Thermo_36(Thermo_36, DHT22);

OneWire oneWire_Thermo_17(Thermo_17);
OneWire oneWire_Thermo_35(Thermo_35);

DallasTemperature sensors_Thermo_17(&oneWire_Thermo_17);
DallasTemperature sensors_Thermo_35(&oneWire_Thermo_35);

//////----------Setup sensors----------//////

//////----------Setup LED----------//////

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 60
#define BRIGHTNESS 255

CRGB leds[NUM_LEDS];

//////----------Setup LED----------//////

//////----------Setup Serveur----------//////

const char *ssid = "Nom du reseau";
const char *password = "Mot de passe du reseau";

AsyncWebServer server(80);

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

//////----------Setup Serveur----------//////

//////----------Setup variable----------//////

const int Seconde=1000;
const int Heure=Seconde*3600;

const int TempsDeVaporisation = 30; // TEMPS DE VAPORISATIONS DE BASE EN SECONDE

unsigned long ValeurTempsDeVapo=Seconde*TempsDeVaporisation;

bool etatLedBarre = 0;
bool etatLedBarreVoulu = 0;

unsigned long TempsMinutes = 0;
String TempsActuel = "00:00";
String HeureActuel = "0";
String MinutesActuel = "0";
String SecondeActuel = "0";
int resultHeure = 0;
int resultMinutes = 0;
int resultSeconde = 0;

float temp = 20.0;
float temp2 = 20.0;

//////----------Setup variable----------//////

void ActualisationTempsServeur()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    TempsActuel = "Erreur !";
    return;
  }
  HeureActuel = timeinfo.tm_hour;
  if (timeinfo.tm_hour < 10) {HeureActuel = "0" + HeureActuel;}
  MinutesActuel = timeinfo.tm_min;
  if (timeinfo.tm_min < 10) {MinutesActuel = "0" + MinutesActuel;}
  SecondeActuel = timeinfo.tm_sec;
  TempsMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  TempsActuel = HeureActuel + ":" + MinutesActuel;
}

void sunrise()
{
  static const uint8_t sunriseLength = 30;
  static const uint8_t interval = (sunriseLength * 60) / 200;

  static uint8_t heatIndex = 0;

  CRGB color = ColorFromPalette(HeatColors_p, heatIndex);

  fill_solid(leds, NUM_LEDS, color);

  EVERY_N_SECONDS(interval)
  {
    if (heatIndex < 200)
    {
      heatIndex++;
    }
    if (heatIndex == 200)
    {
      heatIndex = 0;
    }
  }
}

void sunset()
{
  static const uint8_t sunriseLength = 30;
  static const uint8_t interval = (sunriseLength * 60) / 200;

  static uint8_t heatIndex = 200;

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
      heatIndex = 20;
    }
  }
}

void ActivationPompe()
{
  digitalWrite(PompePIN, LOW);
  delay(ValeurTempsDeVapo);
  digitalWrite(PompePIN, HIGH);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\n");

  //////----------Attribution---------//////

  pinMode(Relais_13, OUTPUT);
  pinMode(Relais_18, OUTPUT);
  pinMode(Relais_32, OUTPUT);
  pinMode(Relais_33, OUTPUT);
  pinMode(Relais_23, OUTPUT);
  pinMode(Relais_19, OUTPUT);

  digitalWrite(Relais_13, HIGH); //High = Relais en position basse (logique inversé)
  digitalWrite(Relais_18, HIGH);
  digitalWrite(Relais_32, HIGH);
  digitalWrite(Relais_33, HIGH);
  digitalWrite(Relais_23, HIGH);
  digitalWrite(Relais_19, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);

  DHT_Thermo_16.begin();
  DHT_Thermo_36.begin();

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
      .setCorrection(TypicalLEDStrip)
      .setDither(BRIGHTNESS < 255);

  //////----------Attribution---------//////

  //////----------SPIFFS---------//////

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

  //////----------SPIFFS---------//////

  //////----------WIFI SETUP---------//////

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

  //////----------WIFI SETUP---------//////

  //////----------SERVEUR---------//////

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

  server.on("/lireTemp", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(dht.readTemperature()) + " °C";
    request->send(200, "text/plain", temperature);
  });

  server.on("/lireHumi", HTTP_GET, [](AsyncWebServerRequest *request) {
    String Humidite = String(dht.readHumidity()) + " %";
    request->send(200, "text/plain", Humidite);
  });

  server.on("/lireHumiMoitie", HTTP_GET, [](AsyncWebServerRequest *request) {
    float valHumi2 = dht2.readHumidity();
    String Humidite2 = String(valHumi2) + " %";
    request->send(200, "text/plain", Humidite2);
  });

  server.on("/lireTempEau", HTTP_GET, [](AsyncWebServerRequest *request) {
    float valTempEau = sensors.getTempCByIndex(0);
    String TempEau = String(valTempEau) + " °C";
    request->send(200, "text/plain", TempEau);
  });

  server.on("/lireTempMoitie", HTTP_GET, [](AsyncWebServerRequest *request) {
    float valTempMoitie = dht2.readTemperature();
    String TempMoitie = String(valTempMoitie) + " °C";
    request->send(200, "text/plain", TempMoitie);
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
    etatLedBarre = 0;
    request->send(204);
  });

  server.on("/onPompe", HTTP_GET, [](AsyncWebServerRequest *request) {
    ActivationPompe();
    request->send(204);
  });

  //////----------SERVEUR---------//////

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  server.begin();

  Serial.println("Serveur actif !");
}

void loop()
{
  ActualisationTempsServeur();
  sensors_Thermo_17.requestTemperatures(); 
  delay(2*Seconde);

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

  if (resultHeure == 8 || resultHeure == 14 || resultHeure == 20)
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

  if (TempsMinutes == 1260)
  {
    ESP.restart();
  } 

  FastLED.show();
}
