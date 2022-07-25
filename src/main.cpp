#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <LittleFS.h>

//declare credential wifi
const char* ssid = "MasterAce"; //change with your ssid wifi
const char* password = "arin26112001"; //change with your password wifi

//open connection web server
AsyncWebServer server(80);

//declare pin smart parking system
#define trig 5 //D1
#define echo 4 //D2
#define ledH 5 //D1
#define ledM 2 //D1
#define buzzer 14 //D1

//declare variable
long waktu;
float jarak = 0;

void initFS() {
  if(!LittleFS.begin()) {
    Serial.println("Error to connection LittleFS");
  }
  Serial.println("LittleFS mounted successfuly");
}

void initWiFi(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED) {
    Serial.println(".");
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

String getStatus(){
  //suatu parkiran memiliki range 0 - 30 
  //jika kendaraan masuk dalam range 0-30 : posisi terisi
  // jika diluar range ex 31,32, dst : posisi kosong
  if(jarak > 0 && jarak < 30){
    return "Posisi terisi";
  }
  return "Posisi kosong";
}


String getDistance(){
  //pembacaan sensor ultrasonic &mendapatkan nilai jarak
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  waktu = pulseIn(echo, HIGH);
  jarak = waktu * 0.034 / 2;

  if (isnan(waktu)) {
    Serial.println("Failed to read ultrasoni sensor");
    return "";
  }
  return String(jarak);
}

void handleRequest(){
  //handle request web -> menampilkan file index html 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request){
    request->send(LittleFS, "/index.html", "text/html");
  });

  //handle request nilai jarak
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest * request){
    request->send_P(200, "text/plain", getDistance().c_str());
  });

  //handle requst nilai status
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest * request){
    request->send_P(200,"text/plain", getStatus().c_str());
  });

  server.serveStatic("/", LittleFS, "/");
  server.begin();
}

void smartParking(){
  // range 0 - 30 : berdasarkan nilai jarak
  //(1) jika kendaraan dalam range 6 - 30 cm maka posisi aman dan led hijau hidup
  //(2) jika kendaraan < 6 cm maka posisi berbahaya, led berkedip & buzzer hidup

  if (jarak > 6 && jarak < 30 ) {
    digitalWrite(ledH, HIGH);
    digitalWrite(ledM, LOW);
  } else if (jarak > 0 && jarak < 6) {
    digitalWrite(buzzer, HIGH);
    digitalWrite(ledH, HIGH);
    digitalWrite(ledM, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);
    digitalWrite(ledH, LOW);
    digitalWrite(ledM, LOW);
    delay(30);
  } else {
    digitalWrite(buzzer, LOW);
    digitalWrite(ledH, LOW);
    digitalWrite(ledM, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  initFS();
  handleRequest();
  
  // initialize pin komponen
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(ledH, OUTPUT);
  pinMode(ledM, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // pemberian nilai pin
  digitalWrite(ledH, LOW);
  digitalWrite(ledM, LOW);
  digitalWrite(buzzer, LOW);
}

void loop() {
  smartParking();
  // put your main code here, to run repeatedly:
}