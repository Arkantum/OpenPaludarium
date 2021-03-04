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
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AsyncElegantOTA.h>
#include <EEPROM.h>
#include <Tone32.h>
#include <Adafruit_BME280.h>

//////----------Declaration Librarie----------//////

//////----------Declaration PIN----------//////

#define Relais_1 16 //Relai 1
#define Relais_2 17 //Relai 2
#define Relais_3 18 //Relai 3
#define Relais_4 19 //Relai 4
#define Relais_5 13 //Relai 5
#define Relais_6 27 //Relai 6
#define Relais_7 23 //Relai 7
#define Relais_8 25 //Relai 8

int Relai_Vapo = Relais_5;
int Relai_Rampe_LED = Relais_8;
int Relai_Brume = Relais_6;

#define Pin_Thermo_1 4  //dht22 (temp and humidity)
#define Pin_Thermo_2 5  //dht22 (temp and humidity)
#define Pin_Thermo_3 32 //DS18B20 (temp only)
#define Pin_Thermo_4 33 //DS18B20 (temp only)

#define BandeauLED 12 //ws2812b led
#define LED_R 14
#define LED_G 15
#define BUZZER 26

#define Niveau_1 36
#define Niveau_2 39

#define EEPROM_SIZE 200

#define HISTORY_FILE "/history.json"

String BOT_TOKEN, USER_ID, USER_ID2;

bool D1, D2, D3, D4, D5, D6, D7, D8;

//////----------Declaration PIN----------//////

//////----------Setup sensors----------//////

DHT Thermo_1(Pin_Thermo_1, DHT22);
DHT Thermo_2(Pin_Thermo_2, DHT22);

OneWire oneWire_1(Pin_Thermo_3);
DallasTemperature Thermo_3(&oneWire_1);

OneWire oneWire_2(Pin_Thermo_4);
DallasTemperature Thermo_4(&oneWire_2);

//////----------Setup sensors----------//////

//////----------Setup Serveur----------//////

const char *ssid = "OpenPaludarium";
const char *password = "Arkantum2020";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);
WiFiClientSecure client;

Adafruit_SSD1306 display(128, 64, &Wire);

UniversalTelegramBot *bot;

Adafruit_BME280 bme;

File root = SPIFFS.open("/");
File file = root.openNextFile();

StaticJsonDocument<1000> data;

JsonObject rootJson = data.to<JsonObject>();

JsonArray Temps = rootJson.createNestedArray("Temps"); // stockage temps actuel

JsonArray T_1 = rootJson.createNestedArray("T_1"); // température dht
JsonArray T_2 = rootJson.createNestedArray("T_2");
JsonArray T_3 = rootJson.createNestedArray("T_3"); // température ds1820b
JsonArray T_4 = rootJson.createNestedArray("T_4");

JsonArray H_1 = rootJson.createNestedArray("H_1"); // humidité
JsonArray H_2 = rootJson.createNestedArray("H_2");

JsonArray P_1 = rootJson.createNestedArray("P_1");

JsonArray barT_1 = rootJson.createNestedArray("barT_1");
JsonArray barH_1 = rootJson.createNestedArray("barH_1");

int sizeHist = 84;

char buffer[1000];

//////----------Setup Serveur----------//////

//////----------Setup variable----------//////

const int Adresse_TempsDeVapo = 0;
const int Adresse_TempsDeBrumi = 1;
const int Adresse_FrequenceDeVapo = 2;
const int Adresse_FrequenceDeBrumi = 3;
const int Adresse_TelegramBOT_ID = 100;
const int Adresse_TelegramID = 50;
const int Adresse_TelegramID2 = 51;

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

unsigned long intervaleHistorique = 4 * Heure;
unsigned long PrecedentTestHistorique = 0;

unsigned long intervaleRoutine = 1 * Minute;
unsigned long PrecedentTestRoutine = 0;

int ValeurBoucle;

float ValeurT = 0;
float ValeurH = 0;
float ValeurP = 0;

float ValeurT_1 = 0;
float ValeurT_2 = 0;
float ValeurT_3 = 0;
float ValeurT_4 = 0;

float ValeurH_1 = 0;
float ValeurH_2 = 0;

float ValeurN_1 = 0;
float ValeurN_2 = 0;

//////----------Setup variable----------//////

void playSuccess()
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
  USER_ID2 = EEPROM.readString(Adresse_TelegramID2);
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
  if (timeinfo.tm_sec < 10)
  {
    SecondeActuel = "0" + SecondeActuel;
  }
  SecondeActuel = timeinfo.tm_sec;
  //TempsMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  TempsActuel = HeureActuel + ":" + MinutesActuel + ":" + SecondeActuel;
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
  initialisation_eeprom();
}

void ecriture_eepromSTRING(int address, String param)
{
  EEPROM.writeString(address, param);
  EEPROM.commit();
  initialisation_eeprom();
}

void LedAlert()
{
  digitalWrite(LED_G, HIGH);
  delay(100);
  digitalWrite(LED_G, LOW);
}

void Message_Recu(int NombreMessagesRecu)
{

  for (int i = 0; i < NombreMessagesRecu; i++)
  {
    // Chat id of the requester
    String chat_id = String(bot->messages[i].chat_id);
    if (chat_id != USER_ID && chat_id != USER_ID2)
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
      Temperature += "BME280   :  " + String(ValeurT) + " ºC \n";
      Temperature += "DHT22    :  " + String(ValeurT_1) + " ºC \n";
      Temperature += "DHT22    :  " + String(ValeurT_2) + " ºC \n";
      Temperature += "DS18B20  :  " + String(ValeurT_3) + " ºC \n";
      Temperature += "DS18B20  :  " + String(ValeurT_4) + " ºC \n";
      bot->sendMessage(chat_id, Temperature, "");
    }

    if (text == "/Hygrometrie")
    {
      String Hygrometrie = "Hygrométrie : \n\n";
      Hygrometrie += "BME280   :  " + String(ValeurH) + " ºC \n";
      Hygrometrie += "DHT22    :  " + String(ValeurH_1) + " % \n";
      Hygrometrie += "DHT22    :  " + String(ValeurH_2) + " % \n";
      bot->sendMessage(chat_id, Hygrometrie, "");
    }

    if (text == "/Pression")
    {
      String Pression = "Pression : \n\n";
      Pression += "BME280   :  " + String(ValeurP) + " hPa \n";
      bot->sendMessage(chat_id, Pression, "");
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

void saveHistory()
{
  File historyFile = SPIFFS.open(HISTORY_FILE, "w");
  serializeJson(rootJson, historyFile);
  historyFile.close();
}

void calcStat(){
  float statT_1[7] = {-999,-999,-999,-999,-999,-999,-999};
  float statH_1[7] = {-999,-999,-999,-999,-999,-999,-999};
  int nbClass = 7;  // Nombre de classes - Number of classes                         
  int currentClass = 0;
  int sizeClass = T_1.size() / nbClass;  // 2
  double temp;
  //
  if ( T_1.size() >= sizeHist ) {
    //Serial.print("taille classe ");Serial.println(sizeClass);
    //Serial.print("taille historique ");Serial.println(hist_t.size());
    for ( int k = 0 ; k < T_1.size() ; k++ ) {
      temp = rootJson["T_1"][k];
      if ( statT_1[currentClass] == -999 ) {
        statT_1[ currentClass ] = temp;
      } else {
        statT_1[ currentClass ] = ( statT_1[ currentClass ] + temp ) / 2;
      }
      temp = rootJson["H_1"][k];
      if ( statH_1[currentClass] == -999 ) {
        statH_1[ currentClass ] = temp;
      } else {
        statH_1[ currentClass ] = ( statH_1[ currentClass ] + temp ) / 2;
      }
         
      if ( ( k + 1 ) > sizeClass * ( currentClass + 1 ) ) {
        //Serial.print("k ");Serial.print(k + 1);Serial.print(" Cellule statTemp = ");Serial.println(statTemp[ currentClass ]);
        currentClass++;
      } else {
        //Serial.print("k ");Serial.print(k + 1);Serial.print(" < ");Serial.println(sizeClass * currentClass);
      }
    }
    
    Serial.println("Histogramme de temperature :"); 
    for ( int i = 0 ; i < nbClass ; i++ ) {
      Serial.print(statT_1[i]);Serial.print('|');
    }
    Serial.println("Histogramme d'humidite :"); 
    for ( int i = 0 ; i < nbClass ; i++ ) {
      Serial.print(statH_1[i]);Serial.print('|');
    }
    Serial.print("");
    if ( barT_1.size() == 0 ) {
      for ( int k = 0 ; k < nbClass ; k++ ) { 
        barT_1.add(statT_1[k]);
        barH_1.add(statH_1[k]);
      }  
    } else {
      for ( int k = 0 ; k < nbClass ; k++ ) { 
        barT_1[k] = statT_1[k];
        barH_1[k] = statH_1[k];
      }  
    }
  }
}

void loadHistory()
{
  File file = SPIFFS.open(HISTORY_FILE, "r");
  if (!file)
  {
    Serial.println("Aucun historique existe - No History Exist");
  }
  else
  {
    size_t size = file.size();
    if (size == 0)
    {
      Serial.println("Fichier historique vide - History file empty !");
    }
    else
    {
      std::unique_ptr<char[]> buf(new char[size]);
      file.readBytes(buf.get(), size);
      DynamicJsonDocument schedulesjson(1300);
      schedulesjson.to<JsonArray>();
      DeserializationError err = deserializeJson(schedulesjson, buf.get());
      if (err)
      {
        Serial.println("Impossible de lire le JSON - Impossible to read JSON file");
      }
      else
      {
        Serial.println("Historique chargé");
        serializeJson(schedulesjson, Serial);
      }
    }
    file.close();
  }
}

void setup()
{

  Serial.begin(115200);
  Serial.println("\n");

  //////----------Attribution---------//////

  pinMode(Relais_1, OUTPUT);
  pinMode(Relais_2, OUTPUT);
  pinMode(Relais_3, OUTPUT);
  pinMode(Relais_4, OUTPUT);
  pinMode(Relais_5, OUTPUT);
  pinMode(Relais_6, OUTPUT);
  pinMode(Relais_7, OUTPUT);
  pinMode(Relais_8, OUTPUT);

  digitalWrite(Relais_1, HIGH); //High = Relais en position basse (logique inversé)
  digitalWrite(Relais_2, HIGH);
  digitalWrite(Relais_3, HIGH);
  digitalWrite(Relais_4, HIGH);
  digitalWrite(Relais_5, HIGH);
  digitalWrite(Relais_6, HIGH);
  digitalWrite(Relais_7, HIGH);
  digitalWrite(Relais_8, HIGH);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);

  digitalWrite(LED_R, HIGH);
  digitalWrite(LED_G, HIGH);

  Thermo_1.begin();
  Thermo_2.begin();

  initialisation_eeprom();

  //////----------Attribution---------//////

  //////----------SPIFFS---------//////

  if (!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    digitalWrite(LED_R, HIGH);
    return;
  }

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
    Serial.println("Ecran non branché");
    playFailed();
  }

  if (!bme.begin(0x76, &Wire))
  {
    Serial.println("Capteur interne non branché");
    playFailed();
  }

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

  Wire.begin();

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

  server.on("/favicon.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/favicon.svg", "image/svg");
  });

  server.on("/graph_temp.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/history_file", "text/json");
  });

  //////----------SERVEUR---------//////

  //////----------SERVEUR COMMANDE---------//////

  server.on("/mesures.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"Temperature\":\"" + String(ValeurT) + " °C" + "\",";
    json += "\"Humidite\":\"" + String(ValeurH) + " %" + "\",";
    json += "\"Pression\":\"" + String(ValeurP) + " hPa" + "\"}";
    request->send(200, "application/json", json);
    Serial.println("Send measures");
  });

  server.on("/Temps", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", TempsActuel);
  });

  server.on("/tabmesures.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "[";
    float temp = rootJson["T_1"][0];
    json += "{\"mesure\":\"T_1\",\"valeur\":\"" + String(ValeurT_1) + "\",\"unite\":\" °C\",\"glyph\":\"glyphicon-indent-left\",\"precedente\":\"" + String(temp) + "\"},";
    temp = rootJson["H_1"][0];
    json += "{\"mesure\":\"H_1\",\"valeur\":\"" + String(ValeurH_1) + "\",\"unite\":\" %\",\"glyph\":\"glyphicon-tint\",\"precedente\":\"" + String(temp) + "\"},";
    temp = rootJson["N_1"][0];
    json += "{\"mesure\":\"N_1\",\"valeur\":\"" + String(ValeurN_1) + "\",\"unite\":\" %\",\"glyph\":\"glyphicon-dashboard\",\"precedente\":\"" + String(temp) + "\"}";
    json += "]";
    request->send(200, "application/json", json);
  });

  server.on("/IDtelegram", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("IDtelegram", true))
    {
      String message = request->getParam("IDtelegram", true)->value();
      Serial.println(message);
      EEPROM.writeString(Adresse_TelegramID, message);
      EEPROM.commit();
      LedAlert();
      playSuccess();
    }
    request->send(204);
  });

  server.on("/IDtelegram2", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("IDtelegram2", true))
    {
      String message = request->getParam("IDtelegram2", true)->value();
      Serial.println(message);
      EEPROM.writeString(Adresse_TelegramID2, message);
      EEPROM.commit();
      LedAlert();
      playSuccess();
    }
    request->send(204);
  });

  server.on("/IDtelegramEnregistre", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", USER_ID);
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
      playSuccess();
    }
    request->send(204);
  });

  server.on("/TOKENtelegramEnregistre", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", BOT_TOKEN);
  });

  server.on("/TempsVaporisation", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("TempsVaporisation", true))
    {
      String message;
      message = request->getParam("TempsVaporisation", true)->value();
      ValeurTempsDeVapo = message.toInt();
      ecriture_eeprom(Adresse_TempsDeVapo, ValeurTempsDeVapo);
    }
    playSuccess();
    request->send(204);
  });

  server.on("/TempsBrumisation", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("TempsBrumisation", true))
    {
      String message;
      message = request->getParam("TempsBrumisation", true)->value();
      ValeurTempsDeBrumi = message.toInt();
      ecriture_eeprom(Adresse_TempsDeBrumi, ValeurTempsDeBrumi);
      playSuccess();
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
    playSuccess();
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
    playSuccess();
    request->send(204);
  });

  //////----------SERVEUR COMMANDE---------//////

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  AsyncElegantOTA.begin(&server);
  server.begin();

  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, HIGH);

  Serial.println("Serveur actif !");

  Serial.println(BOT_TOKEN);
  Serial.println(USER_ID);
  Serial.println(USER_ID2);

  serializeJson(rootJson, buffer);

  TelegramBot();

  delay(1000);
  digitalWrite(LED_G, LOW);

  Serial.println(buffer);
}

void loop()
{
  //////----------Routine----------//////
  AsyncElegantOTA.loop();

  if (millis() > DerniereRequeteBot + DelaiRequeteBot)
  {
    int NombreMessagesRecu = bot->getUpdates(bot->last_message_received + 1);
    while (NombreMessagesRecu)
    {
      Serial.println("Message recu !");
      Message_Recu(NombreMessagesRecu);
      NombreMessagesRecu = bot->getUpdates(bot->last_message_received + 1);
    }
    DerniereRequeteBot = millis();
  }

  if (millis() - intervaleHistorique > intervaleRoutine)
  {
    intervaleHistorique = millis();
    Temps.add(TempsActuel);
    T_1[1] = ValeurT_1;
    H_1[1] = ValeurH_1;
    P_1[1] = ValeurP;
    if (Temps.size() > sizeHist)
    {
      //Serial.println("efface anciennes mesures");
      Temps.remove(0);
      T_1.remove(0);
      H_1.remove(0);
      P_1.remove(0);
    }
    saveHistory();
  }

  if (millis() > intervaleRoutine + PrecedentTestRoutine)
  {
    ValeurT = bme.readTemperature();
    ValeurH = bme.readHumidity();
    ValeurP = bme.readPressure() / 100.0;
    if (!(isnan(Thermo_1.readTemperature())))
    {
      ValeurT_1 = Thermo_1.readTemperature();
    }
    if (!(isnan(Thermo_2.readTemperature())))
    {
      ValeurT_2 = Thermo_2.readTemperature();
    }
    if (!(isnan(Thermo_1.readHumidity())))
    {
      ValeurH_1 = Thermo_1.readHumidity();
    }
    if (!(isnan(ValeurH_2 = Thermo_2.readHumidity())))
    {
      ValeurH_2 = Thermo_2.readHumidity();
    }
    Thermo_3.requestTemperatures();
    Thermo_4.requestTemperatures();
    if (Thermo_3.getTempCByIndex(0) != -127.00)
    {
      ValeurT_3 = Thermo_3.getTempCByIndex(0);
    }
    if (Thermo_4.getTempCByIndex(0) != -127.00)
    {
      ValeurT_4 = Thermo_4.getTempCByIndex(0);
    }

    ValeurN_1 = (analogRead(Niveau_1) / 4096) * 100;
    ValeurN_2 = (analogRead(Niveau_2) / 4096) * 100;
  }

  if (millis() > DerniereRequeteCapteurs + DelaiRequeteCapteurs)
  {
    ActualisationTempsServeur();
    initialisation_eeprom();
    Thermo_3.requestTemperatures();
    Thermo_4.requestTemperatures();

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
      // if (resultMinutes == 2 && resultSeconde < 5)
      // {
      //   ESP.restart();
      // }
    }
    if (resultHeure >= 8 && resultHeure < 20) //Journée
    {
      digitalWrite(Relai_Rampe_LED, LOW);
    }
    else if (resultHeure < 8 && resultHeure >= 20) //Nuit
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