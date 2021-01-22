//////----------Declaration Librarie----------//////

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <SPIFFS.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AsyncElegantOTA.h>
#include <EEPROM.h>
#include <Tone32.h>

//////----------Declaration Librarie----------//////

//////----------Declaration PIN----------//////

#define Relais_13 13 //Relai 5
#define Relais_16 16 //Relai 1
#define Relais_17 17 //Relai 2
#define Relais_18 18 //Relai 3
#define Relais_19 19 //Relai 4
#define Relais_23 23 //Relai 7
#define Relais_25 25 //Relai 8
#define Relais_27 27 //Relai 6

int Relai_Vapo = Relais_13;
int Relai_Rampe_LED = Relais_25;
int Relai_Brume = Relais_27;

#define Thermo_4 4   //dht22 (temp and humidity)
#define Thermo_5 5   //dht22 (temp and humidity)
#define Thermo_32 32 //DS18B20 (temp only)
#define Thermo_33 33 //DS18B20 (temp only)

#define BandeauLED 12 //ws2812b led
#define LED_R 14
#define LED_G 15
#define BUZZER 26

#define Niveau_36 36 //Capteur analogique pour le niveau d'eau
#define Niveau_39 39

#define LED_BUILTIN 2

#define EEPROM_SIZE 200

String BOT_TOKEN;
String USER_ID;

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

const char *ssid = "OpenPaludarium";
const char *password = "Arkantum667";

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);
WiFiClientSecure client;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

Adafruit_SSD1306 display(128, 64, &Wire);

UniversalTelegramBot *bot;

//////----------Setup Serveur----------//////

//////----------Setup variable----------//////

const int Adresse_TempsDeVapo = 0;
const int Adresse_TempsDeBrumi = 1;
const int Adresse_FrequenceDeVapo = 2;
const int Adresse_FrequenceDeBrumi = 3;
const int Adresse_TelegramBOT_ID = 100;
const int Adresse_TelegramID = 50;

const int Seconde = 1000;
const int Minute = 60 * Seconde;
const int Heure = Seconde * 3600;

int ValeurTempsDeVapo;
int ValeurTempsDeBrumi;
int ValeurFrequenceDeVapo;
int ValeurFrequenceDeBrumi;

int TempsDeVapo;
int TempsDeBrumi;
int FrequenceDeVapo;
int FrequenceDeBrumi;

bool Info_Relai_LED = 0;
bool Info_Relai_Vaporisation = 0;
bool Info_Relai_Brumisation = 0;
bool Info_Relai_Pompe = 0;

unsigned long TempsMinutes = 0;

int DelaiRequeteBot = 1000;
int DelaiRequeteCapteurs = 1000;

unsigned long DerniereRequeteBot;
unsigned long DerniereRequeteCapteurs;
unsigned long DerniereRequeteVaporisation;
unsigned long DerniereRequeteBrumisation;

int ListeHeuresVapo[12];
int HeureIndiceVapo;
int ListeHeuresBrumi[12];
int HeureIndiceBrumi;
int HeureAllumage = 12;

String TempsActuel = "00:00";
String HeureActuel = "0";
String MinutesActuel = "0";
String SecondeActuel = "0";
int resultHeure = 0;
int resultMinutes = 0;
int resultSeconde = 0;
int ValeurBoucle;

float lastTemp_32;
float lastTemp_33;

//////----------Setup variable----------//////

void playPassed()
{
  tone(BUZZER, NOTE_B4, 500, 0);
  noTone(BUZZER, 0);
}

void playFailed()
{
  tone(BUZZER, NOTE_C4, 500, 0);
  noTone(BUZZER, 0);
}

void TelegramBot()
{
  EEPROM.begin(EEPROM_SIZE);
  for (int k = Adresse_TelegramBOT_ID; k < Adresse_TelegramBOT_ID + 46; ++k)
  {
    BOT_TOKEN += char(EEPROM.read(k));
  }
  USER_ID = EEPROM.readString(Adresse_TelegramID);
  bot = new UniversalTelegramBot(BOT_TOKEN, client);
}

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
  //TempsMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  TempsActuel = HeureActuel + ":" + MinutesActuel;
  resultHeure = HeureActuel.toInt();
  resultMinutes = MinutesActuel.toInt();
  resultSeconde = SecondeActuel.toInt();
}

void ActivationVapo()
{
  digitalWrite(Relai_Vapo, LOW);
  Info_Relai_Vaporisation = 1;
  DerniereRequeteVaporisation = millis();
}

void ActivationBrume()
{
  digitalWrite(Relai_Brume, LOW);
  Info_Relai_Brumisation = 1;
  DerniereRequeteBrumisation = millis();
}

void initialisation_eeprom()
{
  TempsDeVapo = EEPROM.read(Adresse_TempsDeVapo);
  TempsDeBrumi = EEPROM.read(Adresse_TempsDeBrumi);
  FrequenceDeVapo = EEPROM.read(Adresse_FrequenceDeVapo);
  FrequenceDeBrumi = EEPROM.read(Adresse_FrequenceDeBrumi);
}

void ecriture_eeprom(int address, int param)
{
  EEPROM.write(address, param);
  EEPROM.commit();
}

void LedAlert()
{
  digitalWrite(LED_G, HIGH);
  delay(100);
  digitalWrite(LED_G, LOW);
}

void Message_Recu(int NombreMessagesRecu)
{
  Serial.println(String(NombreMessagesRecu));

  for (int i = 0; i < NombreMessagesRecu; i++)
  {
    // Chat id of the requester
    String chat_id = String(bot->messages[i].chat_id);
    if (chat_id != USER_ID)
    {
      bot->sendMessage(chat_id, "Utilisateur non enregistrée", "");
      continue;
    }

    // Print the received message
    String text = bot->messages[i].text;
    Serial.println(text);

    String from_name = bot->messages[i].from_name;

    if (text == "/start")
    {
      String keyboardJson = "[[\"/Hygrometrie\", \"/Temperature\"],[\"/Vaporisation\"],[\"/Brume\"],[\"/Lumiere\"]]";
      bot->sendMessageWithReplyKeyboard(chat_id, "Que voulez vous ?", "", keyboardJson, true);
    }

    if (text == "/Temperature")
    {
      String Temperature = "Température : \n\n";
      Temperature += "DS18B20  :  " + String(lastTemp_32) + " ºC \n";
      Temperature += "DS18B20  :  " + String(lastTemp_33) + " ºC \n";
      Temperature += "DHT22    :  " + String(DHT_Thermo_4.readTemperature()) + " ºC \n";
      Temperature += "DHT22    :  " + String(DHT_Thermo_5.readTemperature()) + " ºC \n";
      bot->sendMessage(chat_id, Temperature, "");
    }

    if (text == "/Hygrometrie")
    {
      String Hygrometrie = "Hygrométrie : \n\n";
      Hygrometrie += "DHT22    :  " + String(DHT_Thermo_4.readHumidity()) + " % \n";
      Hygrometrie += "DHT22    :  " + String(DHT_Thermo_5.readHumidity()) + " % \n";
      bot->sendMessage(chat_id, Hygrometrie, "");
    }

    if (text == "/Vaporisation")
    {
      String pompe = "Vaporisation activée pendant : " + String(TempsDeVapo) + " secondes";
      bot->sendMessage(chat_id, pompe, "");
      ActivationVapo();
    }

    if (text == "/Brume")
    {
      String pompe = "Brumisation activée pendant : " + String(TempsDeBrumi) + " minutes";
      bot->sendMessage(chat_id, pompe, "");
      ActivationBrume();
    }
  }
}

void setup()
{
  digitalWrite(BUILTIN_LED, HIGH);

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
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);

  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);

  DHT_Thermo_4.begin();
  DHT_Thermo_5.begin();

  //////----------Attribution---------//////

  //////----------SPIFFS---------//////

  if (!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    digitalWrite(LED_R, HIGH);
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
  Serial.begin(115200);
  Serial.println("\n");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    digitalWrite(LED_R, HIGH);
    delay(1000);
    digitalWrite(LED_R, LOW);
  }

  initialisation_eeprom();
  TelegramBot();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.clearDisplay();
  display.setCursor(10, 0);
  display.println("Info de connexion");
  display.setCursor(0, 25);
  display.println("SSID : OpenPaludarium");
  display.setCursor(10, 45);
  display.println("MDP : Arkantum667");
  display.display();

  digitalWrite(LED_R, HIGH);
  wifiManager.autoConnect(ssid, password);

  Serial.println("Connexion etablie !");
  Serial.print("Adresse IP: ");

  display.clearDisplay();
  display.setCursor(40, 0);
  display.println("IPv6 info");
  display.setCursor(30, 25);
  display.println(WiFi.localIP());
  Serial.println(WiFi.localIP());
  display.setCursor(15, 45);
  display.println(WiFi.macAddress());
  display.display();

  //////----------WIFI SETUP---------//////

  //////----------SERVEUR---------//////

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/reglage", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/datamodifier.html", "text/html");
  });

  server.on("/courbe", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/curb.html", "text/html");
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

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/favicon.png", "image/png");
  });

  //////----------SERVEUR---------//////

  //////----------SERVEUR COMMANDE---------//////

  server.on("/Temp_Thermo_4", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(DHT_Thermo_4.readTemperature());
    request->send(200, "text/plain", temperature);
  });
  server.on("/Temp_Thermo_5", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(DHT_Thermo_5.readTemperature());
    request->send(200, "text/plain", temperature);
  });
  server.on("/Temp_Thermo_32", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(lastTemp_32);
    request->send(200, "text/plain", temperature);
  });
  server.on("/Temp_Thermo_33", HTTP_GET, [](AsyncWebServerRequest *request) {
    String temperature = String(lastTemp_33);
    request->send(200, "text/plain", temperature);
  });

  server.on("/Humi_Thermo_4", HTTP_GET, [](AsyncWebServerRequest *request) {
    String Humidite = String(DHT_Thermo_4.readHumidity());
    request->send(200, "text/plain", Humidite);
  });
  server.on("/Humi_Thermo_5", HTTP_GET, [](AsyncWebServerRequest *request) {
    String Humidite = String(DHT_Thermo_5.readHumidity());
    request->send(200, "text/plain", Humidite);
  });

  server.on("/Temps", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", TempsActuel);
  });

  server.on("/Info_Relai_LED_On", HTTP_GET, [](AsyncWebServerRequest *request) {
    Info_Relai_LED = 1;
    request->send(204);
  });
  server.on("/Info_Relai_LED_Off", HTTP_GET, [](AsyncWebServerRequest *request) {
    Info_Relai_LED = 0;
    request->send(204);
  });

  server.on("/Pompe_Activation", HTTP_GET, [](AsyncWebServerRequest *request) {
    ActivationVapo();
    request->send(204);
  });

  server.on("/TempsVaporisation", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("TempsVaporisation", true))
    {
      String message;
      message = request->getParam("TempsVaporisation", true)->value();
      ValeurTempsDeVapo = message.toInt();
      ecriture_eeprom(Adresse_TempsDeVapo, ValeurTempsDeVapo);
    }
    request->send(204);
  });

  server.on("/TempsBrumisation", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("TempsBrumisation", true))
    {
      String message;
      message = request->getParam("TempsBrumisation", true)->value();
      ValeurTempsDeBrumi = message.toInt();
      ecriture_eeprom(Adresse_TempsDeBrumi, ValeurTempsDeBrumi);
    }
    request->send(204);
  });

  server.on("/FrequenceVaporisation", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("FrequenceVaporisation", true))
    {
      String message;
      message = request->getParam("FrequenceVaporisation", true)->value();
      ValeurFrequenceDeVapo = message.toInt();
      ecriture_eeprom(Adresse_FrequenceDeVapo, ValeurFrequenceDeVapo);
    }
    request->send(204);
  });

  server.on("/FrequenceBrumisation", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("FrequenceBrumisation", true))
    {
      String message;
      message = request->getParam("FrequenceBrumisation", true)->value();
      ValeurFrequenceDeBrumi = message.toInt();
      ecriture_eeprom(Adresse_FrequenceDeBrumi, ValeurFrequenceDeBrumi);
    }
    request->send(204);
  });

  server.on("/IDtelegram", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("IDtelegram", true))
    {
      String message = request->getParam("IDtelegram", true)->value();
      Serial.println(message);
      EEPROM.writeString(Adresse_TelegramID, message);
      EEPROM.commit();
      LedAlert();
      playFailed();
    }
    request->send(204);
  });

  server.on("/TOKENtelegram", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("TOKENtelegram", true))
    {
      String message = request->getParam("TOKENtelegram", true)->value();
      Serial.println(message);
      for (int i = 0; i < message.length(); ++i)
      {
        EEPROM.write(Adresse_TelegramBOT_ID + i, message[i]);
      }
      EEPROM.commit();
      LedAlert();
      playPassed();
    }
    request->send(204);
  });

  //////----------SERVEUR COMMANDE---------//////

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  AsyncElegantOTA.begin(&server);
  server.begin();

  digitalWrite(BUILTIN_LED, LOW);
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, HIGH);

  Serial.println("Serveur actif !");

  Serial.println(BOT_TOKEN);
  Serial.println(USER_ID);

  delay(1000);
  digitalWrite(LED_G, LOW);
}

void loop()
{
  //////----------Routine----------//////
  AsyncElegantOTA.loop();

  delay(100);

  if (millis() > DerniereRequeteBot + DelaiRequeteBot)
  {
    int NombreMessagesRecu = bot->getUpdates(bot->last_message_received + 1);
    while (NombreMessagesRecu)
    {
      Serial.println("Reponse recu");
      Message_Recu(NombreMessagesRecu);
      NombreMessagesRecu = bot->getUpdates(bot->last_message_received + 1);
    }
    DerniereRequeteBot = millis();
  }

  if (millis() > DerniereRequeteCapteurs + DelaiRequeteCapteurs)
  {
    ActualisationTempsServeur();
    initialisation_eeprom();
    sensors_Thermo_32.requestTemperatures();
    sensors_Thermo_33.requestTemperatures();

    if (sensors_Thermo_32.getTempCByIndex(0) != -127.00)
    {
      lastTemp_32 = sensors_Thermo_32.getTempCByIndex(0);
    }

    if (sensors_Thermo_33.getTempCByIndex(0) != -127.00)
    {
      lastTemp_33 = sensors_Thermo_33.getTempCByIndex(0);
    }

    HeureIndiceVapo = HeureAllumage / FrequenceDeVapo;
    for (int i = 0; i < 12; ++i)
    {
      ListeHeuresVapo[i] = int(8 + i * HeureIndiceVapo);
    }
    HeureIndiceBrumi = HeureAllumage / FrequenceDeBrumi;
    for (int i = 0; i < 12; ++i)
    {
      ListeHeuresBrumi[i] = int(8 + i * HeureIndiceBrumi);
    }
    if (resultHeure == 20)
    {
      if (resultMinutes == 2 && resultSeconde < 5)
      {
        ESP.restart();
      }
    }
    if (resultHeure >= 8 && resultHeure < 20) //Journée
    {
      digitalWrite(Relai_Rampe_LED, LOW);
    }
    if (resultHeure < 8 && resultHeure >= 20) //Nuit
    {
      digitalWrite(Relai_Rampe_LED, HIGH);
    }
    DerniereRequeteCapteurs = millis();
  }

  //////----------Routine----------//////

  //////----------Routine temporelle----------//////
  for (int Indice = 0; Indice < 12; ++Indice)
  {
    if (resultHeure == ListeHeuresVapo[Indice])
    {
      if (resultMinutes == 0 && resultSeconde < 5)
      {
        ActivationVapo();
      }
    }
    if (resultHeure == ListeHeuresBrumi[Indice])
    {
      if (resultMinutes == 0 && resultSeconde < 5)
      {
        ActivationBrume();
      }
    }
  }

  //////----------Routine temporelle----------//////

  //////----------Eclairage----------//////

  if (Info_Relai_Vaporisation)
  {
    if (millis() > DerniereRequeteVaporisation + (EEPROM.read(Adresse_TempsDeVapo) * Seconde))
    {
      digitalWrite(Relai_Vapo, HIGH);
      Info_Relai_Vaporisation = 0;
      DerniereRequeteVaporisation = 24 * Heure;
    }
  }

  if (Info_Relai_Brumisation)
  {
    if (millis() > DerniereRequeteBrumisation + (EEPROM.read(Adresse_TempsDeBrumi) * Minute))
    {
      digitalWrite(Relai_Brume, HIGH);
      Info_Relai_Brumisation = 0;
      DerniereRequeteBrumisation = 24 * Heure;
    }
  }

  //////----------Eclairage----------//////
}