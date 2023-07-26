#include <ESP8266WiFi.h>
#include "ThingSpeak.h"
#include <Wire.h>
#include <FirebaseESP8266.h>
#include <SFE_BMP180.h>
#include <dht11.h>

#define FIREBASE_HOST "**" // Degistirilecek
#define FIREBASE_AUTH "**" // Degistirilecek

#define DHT11PIN D0 
#define BUZZERPIN D8
#define YESILLED D6
#define MAVILED D5
#define LED1 D3
#define LED2 D4
#define LED3 D7
#define WLAN_SSID  "omer"
#define WLAN_PASS  "12345678"
#define rakim 31.0 

const char* thingSpeakHost = "api.thingspeak.com";
String writeAPIKey = "**"; // Degistirilecek


WiFiClient client;
SFE_BMP180 bmp180;

dht11 DHT11;

int sicaklik, nem;
int mq135 = A0; 
int havaKalitesi = 0; 

double basinc; 
double a, P; //a: altitude , p: pressure


unsigned long lastTime = millis();

FirebaseData veritabanim;

void setup() {
  Serial.begin(9600);
  bool baglanti = bmp180.begin();

  if (bmp180.begin())
    Serial.println("BMP180 Baglanildi1");
  else
  {
    Serial.println("BMP180 Baglanilamadi!\n\n");
    while (1);
  }
  basinc = getPressure();
 
  pinMode(BUZZERPIN,OUTPUT);
  pinMode(YESILLED,OUTPUT);
  pinMode(MAVILED,OUTPUT);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  pinMode(LED3,OUTPUT);
  digitalWrite(YESILLED,HIGH);
  digitalWrite(MAVILED,LOW);
  digitalWrite(BUZZERPIN,LOW);

  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
     delay(500);
     Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
}

void loop() {
    unsigned long elapsedTime = millis() - lastTime;
    lastTime = lastTime + elapsedTime;
  
    getValuesFromFirebase();
    sendDataToThingspeak(elapsedTime);
    
}

void sendDataToThingspeak(long elapsedMS){
  static long waitTime = 0;
  waitTime = waitTime + elapsedMS;

  if(waitTime >= 1000){
    P = getPressure();
    a = bmp180.altitude(P, basinc) + rakim;
    readData();
    if (client.connect(thingSpeakHost,80))  
      {                  
       String postStr = writeAPIKey;
       postStr +="&field1=";
       postStr += String(sicaklik);
       postStr +="&field2=";
       postStr += String(nem);
       postStr +="&field3=";
       postStr += String(havaKalitesi);
       postStr +="&field4=";
       postStr += String(P);
       postStr +="&field5=";
       postStr += String(a);
       client.print("POST /update HTTP/1.1\n");
       client.print("Host: api.thingspeak.com\n");
       client.print("Connection: close\n");
       client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
       client.print("Content-Type: application/x-www-form-urlencoded\n");
       client.print("Content-Length: ");
       client.print(postStr.length());
       client.print("\n\n");
       client.print(postStr);
       Serial.print("Sicaklik: ");
       Serial.println(sicaklik);
       Serial.print("Nem: ");
       Serial.println(nem);
       Serial.print("Hava Kalitesi: ");
       Serial.println(havaKalitesi);
       Serial.print("Basınç: ");
       Serial.print(P);
       Serial.println("hpa");
       Serial.print("Rakım: ");
       Serial.print(a);
       Serial.println("metre");
       Serial.println("-------------------");
    }
    Serial.println("Waiting...");
    waitTime = waitTime - 1000;  
  }
}

void readData(){
  DHT11.read(DHT11PIN);
  sicaklik = DHT11.temperature;
  nem = DHT11.humidity;
  havaKalitesi = analogRead(mq135);
  if(havaKalitesi>250){
     digitalWrite(YESILLED,LOW);
     digitalWrite(BUZZERPIN,HIGH);
     digitalWrite(MAVILED,HIGH);
     delay(500);
     digitalWrite(BUZZERPIN,LOW);
     digitalWrite(MAVILED,LOW);
  }
  else{
    digitalWrite(YESILLED,HIGH);
  }
}

double getPressure()
{
  char status;
  double T, P, p0, a;


  status = bmp180.startTemperature();
  if (status != 0)
  {
    delay(status);
    status = bmp180.getTemperature(T);
    if (status != 0)
    {
      
      status = bmp180.startPressure(3);
      if (status != 0)
      {
        delay(status);
        status = bmp180.getPressure(P, T);
        if (status != 0)
        {
          return (P);
        }

        else Serial.println("Basınç ölçümünde hata alındı\n");
      }

      else Serial.println("Basınç Ölçümü başlatılamadı\n");
    }

    else Serial.println("Sıcaklık değeri alınamadı\n");
  }

  else Serial.println("Sıcaklık ölçümü başlatılamadı\n");
}


void getValuesFromFirebase(){
  if(Firebase.getString(veritabanim, "/led1")){
     if (veritabanim.stringData()=="1"){
       digitalWrite(LED1,HIGH);
     }
     else {
       digitalWrite(LED1,LOW);
     }
     
  }
  if (Firebase.getString(veritabanim, "/led2")){
     if (veritabanim.stringData()=="1"){
       digitalWrite(LED2,HIGH);
     }
     else {
       digitalWrite(LED2,LOW);
     }
     
  }
  if (Firebase.getString(veritabanim, "/led3")){
     if (veritabanim.stringData()=="1"){
       digitalWrite(LED3,HIGH);
     }
     else {
       digitalWrite(LED3,LOW);
     }
     
  }
  
}
