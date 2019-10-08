#include "DHT.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>  
#include <PubSubClient.h>
#define DHTPIN D4
#define DHTTYPE DHT11
#define SSD1306_LCDHEIGHT 64
#define led D7
#define buzzer D8
// OLED display TWI address
#define OLED_ADDR   0x3C
void callback(char* subtopic, byte* payload, unsigned int payloadLength);
const char* ssid = "YOUJNTU1S12";
const char* password = "YOUJNTU1S12";
//---------DEVICE CRED----------
#define ORG "ks6fk6"
#define DEVICE_TYPE "nodemcu"
#define DEVICE_ID "MAN11"
#define TOKEN "123456789"
String command;
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char subtopic[] = "iot-2/cmd/CSM/fmt/String";
char publishtopic[] = "iot-2/evt/CSM/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;
//Serial.println(clientID);

WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);

Adafruit_SSD1306 display(-1);
float t;
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
DHT dht(DHTPIN,DHTTYPE);
void wifiConnect();
void mqttConnect();
void initManagedDevice();

void setup() {
    wifiConnect();
  Serial.begin(115200);
  dht.begin();
  Serial.println("DHT11:");
  pinMode(D5,INPUT);
  pinMode(D6,INPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(led,OUTPUT);  
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextColor(WHITE);
  t=dht.readTemperature();
  if(isnan(t))
  {
    Serial.println("Failed");
    display.setTextSize(1);
    display.print("Failed to read t from DHT11 sensot");
    display.display();
    display.clearDisplay();
    return ;
  }
  digitalWrite(buzzer,LOW);
  mqttConnect();
}

void loop() {
    display.setTextSize(1);
    display.setCursor(0,10);
    display.print("Temperature is: " +String(t));
    Serial.println("Temperature is: " +String(t));

  int inc=digitalRead(D5);
  int dec=digitalRead(D6);
   if(inc==1 && dec==1)
   {
    t=t;
   }
   else if(inc==0 && dec==1)
   {
    t=t+1;
   }
   else if(inc==1 && dec==0)
   {
    t=t-1;
   }
   else if(inc==0 && dec==0)
   {
    t=t;
   }    

    if(t<5 || t>30)
    {
      display.clearDisplay();
      display.setCursor(0,10);
      display.print("ALERT");
      display.setCursor(10,20);
      display.print("Temperature :"+String(t));
      digitalWrite(led,HIGH);
      tone(buzzer,1000);
      delay(500);
      digitalWrite(led,LOW);
      noTone(buzzer);
      }
    display.display();
    display.clearDisplay();
    PublishData(t);
  if (!client.loop()) 
  {
    mqttConnect();
  }
  delay(100);
}
void wifiConnect() {
  Serial.print("Connecting to "); Serial.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected, IP address: "); 
  Serial.println(WiFi.localIP());
}

void mqttConnect() {
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); 
    Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    initManagedDevice();
    Serial.println();
  }
}
void initManagedDevice() {
  if (client.subscribe(subtopic)) {
    Serial.println("subscribe to cmd OK");
  } else {
    Serial.println("subscribe to cmd FAILED");
  }
}
void callback(char* subtopic, byte* payload, unsigned int payloadLength) {
  Serial.print("callback invoked for sub topic: "); 
  Serial.println(subtopic);
  command="";
  for (int i = 0; i < payloadLength; i++) {
    //Serial.println((char)payload[i]);
    command += (char)payload[i];
  }
  Serial.println(command);
  if(command=="itemp"){
    t=t+1;
    Serial.println("Temperature Increased");
  }
  else if(command=="dtemp"){
    t=t-1;
    Serial.println("Temperature Decreased");
  }
}
void PublishData(int command){ 
 if (!!!client.connected()) {
 Serial.print("Reconnecting client to ");
 Serial.println(server);
 while (!!!client.connect(clientId, authMethod, token)) {
 Serial.print(".");
 delay(500);
 }
 Serial.println();
 }
  String payload = "{\"d\":{\"command\":";
  payload += command;
  payload += "}}";
 Serial.print("Sending payload: ");
 Serial.println(payload);
  
 if (client.publish(publishtopic, (char*) payload.c_str())) {
 Serial.println("Publish ok");
 } else {
 Serial.println("Publish failed");
 }

}
