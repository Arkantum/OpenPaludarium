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
#define Relais_16 16 //Relai Rampe LED
#define Relais_17 17 //Relai
#define Relais_18 18 //Relai
#define Relais_19 19 //Relai
#define Relais_23 23 //Relai
#define Relais_25 25 //Relai
#define Relais_27 27 //Relai

int Relai_Pompe = Relais_13;
int Relai_Rampe_LED = Relais_16;
int Relai_Brume = Relais_18;

#define Thermo_4 4   //dht22 (temp and humidity)
#define Thermo_5 5   //dht22 (temp and humidity)
#define Thermo_32 32 //DS18B20 (temp only)
#define Thermo_33 33 //DS18B20 (temp only)

#define BandeauLED 12 //ws2812b led
#define LED_R 14
#define LED_G 15

#define Niveau_36 36 //Capteur analogique pour le niveau d'eau
#define Niveau_39 39

#define LED_BUILTIN 2

//////----------Declaration PIN----------//////

//////----------Setup sensors----------//////

DHT DHT_Thermo_4(Thermo_4, DHT22);
DHT DHT_Thermo_5(Thermo_5, DHT22);

OneWire oneWire_Thermo_32(Thermo_32);
DallasTemperature sensors_Thermo_32(&oneWire_Thermo_32);

OneWire oneWire_Thermo_33(Thermo_33);
DallasTemperature sensors_Thermo_33(&oneWire_Thermo_33);

//////----------Setup sensors----------//////

//////----------Setup Serveur----------//////

const char *ssid = "Nom du reseau WIFI";
const char *password = "Mot de passe du reseau WIFI";

#define CHAT_ID "ID de l'utilisateur"
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
const int Minute = 60 * Seconde;
const int Heure = Seconde * 3600;

const int TempsDeVaporisation = 30; // TEMPS DE VAPORISATIONS DE BASE EN SECONDE
const int FrequenceDeVapo = 4;      // Frequence de vaporisation en une journée

unsigned long ValeurTempsDeVapo = Seconde * TempsDeVaporisation;
unsigned long ValeurFrequenceDeVapo = FrequenceDeVapo;

bool Rampe_Eclairage = 0;
bool Rampe_Eclairage_Temporaire = 1;
bool Info_Relai_Pompe = 0;

unsigned long TempsMinutes = 0;
unsigned long TempsTemporaire = 1500;

int DelaiRequeteBot = 1000;
unsigned long DerniereRequeteBot;

int DelaiRequeteCapteurs = 10000;
unsigned long DerniereRequeteCapteurs;

int DelaiRequetePompe = 1000;
unsigned long DerniereRequetePompe;

String TempsActuel = "00:00";
String HeureActuel = "0";
String MinutesActuel = "0";
String SecondeActuel = "0";
int resultHeure = 0;
int resultMinutes = 0;
int resultSeconde = 0;
int ValeurBoucle;

int ValeurDHT_Thermo_4;
int ValeurDHT_Thermo_5;
int ValeurSensor_Thermo_32;
int ValeurSensor_Thermo_33;
int ValeurDHT_Humi_4;
int ValeurDHT_Humi_5;

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
  while (sensors_Thermo_32 .getTempCByIndex(0) == -127.00)
  {
    ValeurBoucle = sensors_Thermo_32.getTempCByIndex(0);
  }
  message += "Température DS18B20 : " + String(sensors_Thermo_32.getTempCByIndex(0)) + " ºC \n";
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

    if (text == "/info")
    {
      String readings = getReadings();
      bot.sendMessage(chat_id, readings, "");
    }
    if (text == "/options")
    {
      String keyboardJson = "[[\"/info\", \"/start\"],[\"/restart\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Que voulez vous ?", "", keyboardJson, true);
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
  pinMode(Relais_16, OUTPUT);
  pinMode(Relais_17, OUTPUT);
  pinMode(Relais_18, OUTPUT);
  pinMode(Relais_19, OUTPUT);
  pinMode(Relais_23, OUTPUT);
  pinMode(Relais_25, OUTPUT);
  pinMode(Relais_27, OUTPUT);

  digitalWrite(Relais_13, HIGH); //High = Relais en position basse (logique inversé)
  digitalWrite(Relais_16, HIGH);
  digitalWrite(Relais_17, HIGH);
  digitalWrite(Relais_18, HIGH);
  digitalWrite(Relais_19, HIGH);
  digitalWrite(Relais_23, HIGH);
  digitalWrite(Relais_25, HIGH);
  digitalWrite(Relais_27, HIGH);

  pinMode(LED_BUILTIN, OUTPUT);

  DHT_Thermo_4.begin();
  //DHT_Thermo_5.begin();

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
    Serial.print("Fichier : ");
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
    String temperature = String(DHT_Thermo_4.readTemperature());
    request->send(200, "text/plain", temperature);
  });
  server.on("/Temp_Thermo_36", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(DHT_Thermo_5.readTemperature());
    request->send(200, "text/plain", temperature);
  });
  server.on("/Temp_Thermo_17", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(sensors_Thermo_32.getTempCByIndex(0));
    request->send(200, "text/plain", temperature);
  });
  server.on("/Temp_Thermo_35", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(sensors_Thermo_33.getTempCByIndex(0));
    request->send(200, "text/plain", temperature);
  });

  server.on("/Humi_Thermo_16", HTTP_GET, [](AsyncWebServerRequest *request) {
    String Humidite = String(DHT_Thermo_4.readHumidity());
    request->send(200, "text/plain", Humidite);
  });
  server.on("/Humi_Thermo_36", HTTP_GET, [](AsyncWebServerRequest *request) {
    String Humidite = String(DHT_Thermo_5.readHumidity());
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
    digitalWrite(Relai_Pompe, LOW);
    Info_Relai_Pompe = 1;
    DerniereRequetePompe = millis();
    request->send(204);
  });

  server.on("/TempsBrumisation", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("TempsBrumisation", true))
    {
      String message;
      message = request->getParam("TempsBrumisation", true)->value();
      ValeurTempsDeVapo = message.toInt() * Seconde;
    }
    request->send(204);
  });

  server.on("/FrequenceBrumisation", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("FrequenceBrumisation", true))
    {
      String message;
      message = request->getParam("FrequenceBrumisation", true)->value();
      ValeurFrequenceDeVapo = message.toInt();
    }
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
    sensors_Thermo_32.requestTemperatures();
    sensors_Thermo_33.requestTemperatures();
    ValeurSensor_Thermo_32 = sensors_Thermo_32.getTempCByIndex(0);
    ValeurSensor_Thermo_33 = sensors_Thermo_33.getTempCByIndex(0);
    ValeurDHT_Thermo_4 = DHT_Thermo_4.readTemperature();
    ValeurDHT_Thermo_5 = DHT_Thermo_5.readTemperature();
    ValeurDHT_Humi_4 = DHT_Thermo_4.readHumidity();
    ValeurDHT_Humi_5 = DHT_Thermo_5.readHumidity();
    DerniereRequeteCapteurs = millis();
  }

  //////----------Routine----------//////

  //////----------Routine temporelle----------//////

  if (resultHeure == 8 || resultHeure == 12 || resultHeure == 16 || resultHeure == 20)
  {
    if (resultMinutes == 0 && resultSeconde < 11)
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

  // if (Rampe_Eclairage_Temporaire == 1 && Rampe_Eclairage == 0)
  // {
  //   TempsMinutes = TempsTemporaire;
  //   Rampe_Eclairage_Temporaire = 0;
  //   Rampe_Eclairage = 1;
  // }

  // if (Rampe_Eclairage_Temporaire == 0 || TempsMinutes >= TempsTemporaire + TempsActivationManuel)
  // {
  //   Rampe_Eclairage = 0;
  //   TempsTemporaire = 1500; //Temps minutes ne pourra jamais etre superieur a 1500
  // }

  if (Rampe_Eclairage)
  {
    digitalWrite(Relai_Rampe_LED, LOW);
  }
  else
  {
    digitalWrite(Relai_Rampe_LED, HIGH);
  }

  if (Info_Relai_Pompe)
  {
    delay(ValeurTempsDeVapo);
    digitalWrite(Relai_Pompe, HIGH);
    Info_Relai_Pompe = 0;
  }

  //////----------Eclairage----------//////

  //////----------Alerte Telegram----------//////

  if (millis() > DerniereRequetePompe + DelaiRequetePompe)
  {
    DerniereRequetePompe = millis();
  }

  //////----------Alerte Telegram----------//////
}