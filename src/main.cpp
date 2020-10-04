//////----------Declaration Librarie----------//////

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <DHT.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

//////----------Declaration Librarie----------//////

//////----------Declaration PIN----------//////

#define Relais_13 13 //Relai Pompe
#define Relais_32 32 //Relai Rampe LED
#define Relais_33 33 //Relai
#define Relais_18 18 //Relai
#define Relais_19 19 //Relai
#define Relais_23 23 //Relai

int Relai_Pompe = Relais_13;
int Relai_Rampe_LED = Relais_32;

#define Thermo_16 16 //dht22 (temp and humidity)
#define Thermo_36 36 //dht22 (temp and humidity)
#define Thermo_35 35 //DS18B20 (temp only)
#define Thermo_17 17 //DS18B20 (temp only)

#define LED_4 4 //ws2812b led
#define LED_5 5 //ws2812b led

#define Niveau_2 2 //Capteur analogique pour le niveau d'eau

#define LED_BUILTIN 2


//////----------Declaration PIN----------//////

//////----------Setup sensors----------//////

DHT DHT_Thermo_16(Thermo_16, DHT22);
DHT DHT_Thermo_36(Thermo_36, DHT22);

OneWire oneWire_Thermo_17(Thermo_17);
OneWire oneWire_Thermo_35(Thermo_35);

DallasTemperature sensors_Thermo_17(&oneWire_Thermo_17);
DallasTemperature sensors_Thermo_35(&oneWire_Thermo_35);

//////----------Setup sensors----------//////

//////----------Setup Serveur----------//////

const char *ssid = "Nom du reseau";
const char *password = "Mot de passe";

#define CHAT_ID "Chat ID de l'utilisateur"
#define BOTtoken "Token du bot"

AsyncWebServer server(80);

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

//////----------Setup Serveur----------//////

//////----------Setup variable----------//////

const int Seconde = 1000;
const int Heure = Seconde * 3600;

const int TempsDeVaporisation = 20; // TEMPS DE VAPORISATIONS DE BASE EN SECONDE

unsigned long ValeurTempsDeVapo = Seconde * TempsDeVaporisation;

bool Rampe_Eclairage = 0;
bool Rampe_Eclairage_Temporaire = 1;

unsigned long TempsMinutes = 0;
unsigned long TempsTemporaire = 1500;
unsigned long TempsActivationManuel = 5;

int DelaiRequeteBot = 1000;
unsigned long DerniereRequeteBot;

int DelaiRequeteCapteurs = 1000;
unsigned long DerniereRequeteCapteurs;

int DelaiRequeteAlerte = 1000;
unsigned long DerniereRequeteAlerte;

String TempsActuel = "00:00";
String HeureActuel = "0";
String MinutesActuel = "0";
String SecondeActuel = "0";
int resultHeure = 0;
int resultMinutes = 0;
int resultSeconde = 0;

//////----------Setup variable----------//////

void ActualisationTempsServeur()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    TempsActuel = "Erreur temps !";
    return;
  }
  HeureActuel = timeinfo.tm_hour;
  if (timeinfo.tm_hour < 10)
  {
    HeureActuel = "0" + HeureActuel;
  }
  MinutesActuel = timeinfo.tm_min;
  if (timeinfo.tm_min < 10)
  {
    MinutesActuel = "0" + MinutesActuel;
  }
  SecondeActuel = timeinfo.tm_sec;
  TempsMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  TempsActuel = HeureActuel + ":" + MinutesActuel;
}

String getReadings()
{
  String message = "Données actualisée à " + TempsActuel + " (Heure locale) \n";
  message += "Température DS18B20 : " + String(sensors_Thermo_35.getTempCByIndex(0)) + " ºC \n";
  return message;
}

void handleNewMessages(int numNewMessages)
{
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start")
    {
      String welcome = "Bienvenue, " + from_name + ".\n";
      welcome += "Afin de pouvoir lire les valeurs, entrez :.\n\n";
      welcome += "/readings \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/readings")
    {
      String readings = getReadings();
      bot.sendMessage(chat_id, readings, "");
    }
  }
}

void ActivationPompe()
{
  digitalWrite(Relai_Pompe, LOW);
  delay(ValeurTempsDeVapo);
  digitalWrite(Relai_Pompe, HIGH);
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

  server.on("/gauge.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/gauge.min.js", "text/javascript");
  });

  //////----------SERVEUR---------//////

  //////----------SERVEUR COMMANDE---------//////

  server.on("/Temp_Thermo_16", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(DHT_Thermo_16.readTemperature()) + " °C";
    request->send(200, "text/plain", temperature);
  });
  server.on("/Temp_Thermo_36", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(DHT_Thermo_36.readTemperature()) + " °C";
    request->send(200, "text/plain", temperature);
  });
  server.on("/Temp_Thermo_17", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(sensors_Thermo_17.getTempCByIndex(0)) + " °C";
    request->send(200, "text/plain", temperature);
  });
  server.on("/Temp_Thermo_35", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(sensors_Thermo_35.getTempCByIndex(0)) + " °C";
    request->send(200, "text/plain", temperature);
  });

  server.on("/Humi_Thermo_16", HTTP_GET, [](AsyncWebServerRequest *request) {
    String Humidite = String(DHT_Thermo_16.readHumidity()) + " %";
    request->send(200, "text/plain", Humidite);
  });
  server.on("/Humi_Thermo_36", HTTP_GET, [](AsyncWebServerRequest *request) {
    String Humidite = String(DHT_Thermo_36.readHumidity()) + " %";
    request->send(200, "text/plain", Humidite);
  });

  server.on("/Temps", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", TempsActuel);
  });

  server.on("/Rampe_Eclairage_On", HTTP_GET, [](AsyncWebServerRequest *request) {
    Rampe_Eclairage_Temporaire = 1;
    request->send(204);
  });
  server.on("/Rampe_Eclairage_Off", HTTP_GET, [](AsyncWebServerRequest *request) {
    Rampe_Eclairage_Temporaire = 0;
    request->send(204);
  });

  server.on("/Pompe_Activation", HTTP_GET, [](AsyncWebServerRequest *request) {
    ActivationPompe();
    request->send(204);
  });

  //////----------SERVEUR COMMANDE---------//////

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  server.begin();

  Serial.println("Serveur actif !");
}

void loop()
{
  //////----------Routine----------//////

  int resultHeure = HeureActuel.toInt();
  int resultMinutes = MinutesActuel.toInt();
  int resultSeconde = SecondeActuel.toInt();

  if (millis() > DerniereRequeteBot + DelaiRequeteBot)
  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages)
    {
      Serial.println("Reponse recu");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    DerniereRequeteBot = millis();
  }

  if (millis() > DerniereRequeteCapteurs + DelaiRequeteCapteurs)
  {
    ActualisationTempsServeur();
    sensors_Thermo_17.requestTemperatures();
    sensors_Thermo_35.requestTemperatures();
    int ValeurDHT_Thermo_16 = DHT_Thermo_16.readTemperature();
    int ValeurDHT_Thermo_36 = DHT_Thermo_36.readTemperature();
    int ValeurSensor_Thermo_17 = sensors_Thermo_17.getTempCByIndex(0);
    int ValeurSensor_Thermo_35 = sensors_Thermo_35.getTempCByIndex(0);
    int ValeurHumiDHT_Thermo_16 = DHT_Thermo_16.readHumidity();
    int ValeurHumiDHT_Thermo_36 = DHT_Thermo_36.readHumidity();
    DerniereRequeteCapteurs = millis();
  }

  //////----------Routine----------//////

  //////----------Routine temporelle----------//////

  if (resultHeure == 8 || resultHeure == 14 || resultHeure == 20)
  {
    if (resultMinutes == 0 && resultSeconde < 5)
    {
      ActivationPompe();
    }
  }

  if (TempsMinutes >= 480 && TempsMinutes < 1200) //Journée
  {
    Rampe_Eclairage = 1;
  }

  if (TempsMinutes < 480 && TempsMinutes >= 1200) //Nuit
  {
    Rampe_Eclairage = 0;
  }

  //////----------Routine temporelle----------//////

  //////----------Eclairage----------//////

  if (Rampe_Eclairage_Temporaire == 1 && Rampe_Eclairage == 0)
  {
    TempsMinutes = TempsTemporaire;
    Rampe_Eclairage_Temporaire = 0;
    Rampe_Eclairage = 1;
  }

  if (Rampe_Eclairage_Temporaire == 0 || TempsMinutes >= TempsTemporaire + TempsActivationManuel)
  {
    Rampe_Eclairage = 0;
    TempsTemporaire = 1500; //Temps minutes ne pourra jamais etre superieur a 1500
  }

  if (Rampe_Eclairage)
  {
    digitalWrite(Relai_Rampe_LED, LOW);
  }
  else
  {
    digitalWrite(Relai_Rampe_LED, HIGH);
  }

  //////----------Eclairage----------//////

  //////----------Alerte Telegram----------//////

  if (millis() > DerniereRequeteAlerte + DelaiRequeteAlerte)
  {
    DerniereRequeteAlerte = millis();
  }

  //////----------Alerte Telegram----------////// 
   
}