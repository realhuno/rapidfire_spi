#include <WiFi.h>
#include <WiFiMulti.h>
#include <esp_wifi.h>
#include <PubSubClient.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

//OTA
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//WEB
#include <WebServer.h>
#include "index.h"  //Web page header file
 
WebServer server(80);



#define WIFI_AP_NAME "Chorus32 LapTimer"

#define mqtt_rx "rx/cv1/" //rapidfire ID for mqtt



WiFiMulti wifiMulti;
// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "10.0.0.81";

WiFiClient espClient;
PubSubClient client(espClient);
//#define SPI_DATA_PIN 19   
//#define SPI_SS_PIN 5
//#define SPI_CLOCK_PIN 18

#define SPI_DATA_PIN 12
#define SPI_SS_PIN 13
#define SPI_CLOCK_PIN 15

#define delaytime 70
#define MAX_BUF 1500
char buf[MAX_BUF];
uint32_t buf_pos = 0;
String global;
int mqtt=0;
//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
 
void handleADC() {
 
 //String adcValue = String(WiFi.localIP());
 
 server.send(200, "text/plane", global); //Send ADC value only to client ajax request
}

 
void handleLED() {

 String t_state = server.arg("LEDstate"); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
 
  if(t_state.charAt(0) == 'S') {
  buzzer();
  }
  if(t_state.charAt(0) == 'O') {
  osdmode(t_state.charAt(2));
  }
  if(t_state.charAt(0) == 'C') {
  channel(t_state.charAt(2));
  }  
  if(t_state.charAt(0) == 'B') {
  band(t_state.charAt(2));
  }  
  if(t_state.charAt(0) == 'T') {
  text(t_state);
  }  
 Serial1.println(t_state);
 Serial.println(t_state);
 global=t_state;
 server.send(200, "text/plane", t_state); //Send web page

}

void led() {

 String t_state = server.arg("led"); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
 if(t_state=="on"){
 pinMode(4,OUTPUT);
 digitalWrite(4,HIGH);
 }else{
   pinMode(4,OUTPUT);
 digitalWrite(4,LOW);
 }

 server.send(200, "text/plane", t_state); //Send web page

}

void handleTime() {

 //int t_state = server.arg("time").toInt(); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
 int timems=server.arg("time").toInt();

 


 String stringOne =  String(timems, HEX); 
 
 Serial1.println("S1L01000"+stringOne);


 //server.send(200, "text/plane", t_state); //Send web page
}


byte bitBangData(byte _send)  // This function transmit the data via bitbanging
{

// for(int i=0; i<8; i++)  // 8 bits in a byte
  for(int i=7; i>=0; i--)  // 8 bits in a byte
  {
    
    digitalWrite(SPI_DATA_PIN, bitRead(_send, i));   
    delayMicroseconds(delaytime);
    digitalWrite(SPI_CLOCK_PIN, HIGH);  
    //delayMicroseconds(delaytime);
    //digitalWrite(SPI_DATA_PIN, LOW);
    delayMicroseconds(delaytime);
    digitalWrite(SPI_CLOCK_PIN, LOW); 
    delayMicroseconds(delaytime);
      
  } 
 
}
void callback(char* topic, byte* message, unsigned int length) {
   if(mqtt==1){
 
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String line;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    line += (char)message[i];
  }
  Serial.println("Topic:");

   Serial.println(String(topic));
   if (String(topic) == "rx/cv1/cmd_esp_all") {
  Serial.println("MQTT request from Rotorhazard");
   client.publish("rxcn/1", "1");



     if(line.charAt(0) == 'L') {
     if(line.charAt(2)=='0'){
     pinMode(4,OUTPUT);
     digitalWrite(4,HIGH);
     }
     if(line.charAt(2)=='1'){
     pinMode(4,OUTPUT);
     digitalWrite(4,HIGH);
     }
    
  } 

  }
 
 

 
  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    
  if(line.charAt(0) == 'S') {
  buzzer();
  Serial.println("BUZZER");
  }
  if(line.charAt(0) == 'O') {
  osdmode(line.charAt(2));
    Serial.println("OSD");

  }
  if(line.charAt(0) == 'C') {
  channel(line.charAt(2));
    Serial.println("Channel");

  }     
  if(line.charAt(0) == 'B') {
  band(line.charAt(2));
    Serial.println("Band");

  } 
  if(line.charAt(0) == 'T') {
      Serial.println("Text");
  text(line);
  } 
}
  }
}
void setup(void) {

  Serial.begin(115200);

  //WiFi.begin(WIFI_AP_NAME);
  
  wifiMulti.addAP("Chorus32 LapTimer", "");
    wifiMulti.addAP("Laptimer", "laptimer");
    wifiMulti.addAP("A1-7FB051", "hainz2015");

    Serial.println("Connecting Wifi...");
    if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
  






  delay(500);

  memset(buf, 0, 1500 * sizeof(char));
  //OTA
    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

//----------------------------------------------------------------
 
  server.on("/", handleRoot);      //This is display page
  server.on("/readADC", handleADC);//To get update of ADC Value only
  server.on("/setLED", handleLED);
  server.on("/setTime", handleTime);
 server.on("/reconnect", reconnect);
  server.on("/led", led);
 
  server.begin();                  //Start server
  Serial.println("HTTP server started");


  ArduinoOTA.begin();


  //INIT SPI 
  pinMode(SPI_SS_PIN, OUTPUT);
  pinMode(SPI_DATA_PIN, OUTPUT);
  pinMode(SPI_CLOCK_PIN, OUTPUT);

  
  digitalWrite(SPI_SS_PIN, HIGH);
  digitalWrite(SPI_CLOCK_PIN, HIGH);
  digitalWrite(SPI_DATA_PIN, HIGH);
  delay(200); //Let Rapidfire init SPI       
  
  digitalWrite(SPI_SS_PIN, LOW);
  digitalWrite(SPI_CLOCK_PIN, LOW);
  digitalWrite(SPI_DATA_PIN, LOW);
  delay(1000);
  digitalWrite(SPI_SS_PIN, HIGH);
  Serial.begin(115200);
  Serial.println("SPI Start");
  memset(buf, 0, 1500 * sizeof(char));
  delay(1000); //Wait again 
MDNS.begin("radio0");
MDNS.addService("http", "tcp", 80);
// client.setServer(mqtt_server, 1883);
//  client.setCallback(callback);
}
// Not working.... another methode...
void rapidsend(String text){
int len = strlen(text.c_str());
int checksum=0;
 Serial.print("Len:");
 Serial.println(len);
 if(len<=4){
   bitBangData(int(text.charAt(0)));
   Serial.println(int(text.charAt(0)));
   bitBangData(int(text.charAt(1)));
   Serial.println(int(text.charAt(1)));

   checksum=int(text.charAt(0))+int(text.charAt(1));
   bitBangData(00);
   Serial.println(int(0));
   bitBangData(checksum);
   Serial.println(checksum);
 }else{
   bitBangData(int(text.charAt(0)));
   Serial.println(int(text.charAt(0)));
   bitBangData(int(text.charAt(1)));
   Serial.println(int(text.charAt(1)));
   bitBangData(1);//len-4
   Serial.println(1);//len-4
   checksum=int(text.charAt(0))+int(text.charAt(1));
   int testi=text.charAt(2); //len-3
   checksum=141+testi;
   bitBangData(checksum);
   Serial.println(checksum);
/*
   for(int i=3; i<(len-3); i++){
   
   bitBangData(int(text.charAt(i)));
   Serial.println(int(text.charAt(i)));
   checksum=checksum+int(text.charAt(i));
 }
 */
   bitBangData(text.charAt(2));
   Serial.println(text.charAt(2));

 }
}
void buzzer(){
digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);
bitBangData(83); // data transmission   Let Buzzer Beep every second
bitBangData(62); // data transmission
bitBangData(00); // data transmission
bitBangData(91); // data transmission
delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);
}

void text(String text){
digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);  
bitBangData(84); // data transmission   OSD MODE
bitBangData(61); // data transmission
bitBangData(5); // data transmission
bitBangData(138); // data transmission
bitBangData(72); // data transmission
bitBangData(101); // data transmission
bitBangData(108); // data transmission
bitBangData(108); // data transmission
bitBangData(111); // data transmission
delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);
}

void text_probleme(String text){
int len = strlen(text.c_str());
digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);  
Serial.print("_");
Serial.print(145+(len-2));
Serial.println("_");
bitBangData(84); // data transmission   OSD MODE
bitBangData(61); // data transmission
bitBangData(len-2); // data transmission
bitBangData(145+(len-2)); // data transmission 138
   for(int i=2; i<len; i++){
   Serial.println(int(text.charAt(i)));
   bitBangData(int(text.charAt(i)));


 }
delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);
}  


void osdmode(int i){
  digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);
bitBangData(79); // data transmission
bitBangData(61); // data transmission
bitBangData(1); // data transmission
bitBangData(141+(i-48)); // data transmission  145 oder so pal // 147  146      141
bitBangData(i-48); // data transmission  3               //     6    5    0
delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);
Serial.println(i-48);
//0=clear
//1=lock
//2=default
//3=lock+standard
//4=internal
}

void channel(int i){
  digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);
bitBangData(67); // data transmission
bitBangData(61); // data transmission
bitBangData(1); // data transmission
bitBangData(129+(i-48)); // data transmission  145 oder so pal // 147  146      141
bitBangData(i-48); // data transmission  3               //     6    5    0
delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);
Serial.println(i-48);
//0=clear
//1=lock
//2=default
//3=lock+standard
//4=internal
}


void band(int i){
  digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);
bitBangData(66); // data transmission
bitBangData(61); // data transmission
bitBangData(1); // data transmission
bitBangData(128+(i-48)); // data transmission  145 oder so pal // 147  146      141
bitBangData(i-48); // data transmission  3               //     6    5    0
delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);
Serial.println(i-48);
//0=clear
//1=lock
//2=default
//3=lock+standard
//4=internal
}
void reconnect() {
  // Loop until we're reconnected
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      mqtt=1;
      Serial.println("connected");
      // Subscribe
      //client.subscribe("esp32/output");
      client.subscribe("rx/cv1/cmd_esp_all");
      global="MQTT Connected";
 
    } else {
      mqtt=0;
      Serial.print("failed, rc=");
      Serial.print(client.state());
      global="MQTT not Connected";
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  
}
void loop(void) {
  ArduinoOTA.handle();

  server.handleClient();
  delay(1);
    if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        delay(1000);
    }




/*
  if(line.charAt(0) == 'S') {
  buzzer();
  }
  if(line.charAt(0) == 'O') {
  osdmode(line.charAt(2));
  }
  if(line.charAt(0) == 'C') {
  channel(line.charAt(2));
  }     
  if(line.charAt(0) == 'B') {
  band(line.charAt(2));
  } 
  if(line.charAt(0) == 'T') {
  text(line);

  
  }  
  */
  
    IPAddress myip = WiFi.localIP();
    
while(Serial.available()) {
    if(buf_pos >= MAX_BUF -1 ) {
      buf_pos = 0; // clear buffer when full
      break;
    }
    buf[buf_pos++] = Serial.read();
    if(buf[buf_pos - 1] == '\n') {
     //Serial.println(buf);
     if(buf[0] == 'S') {


     buzzer();

     }
     if(buf[0] == 'O') {
     osdmode(buf[2]);
     }
     if(buf[0] == 'C') {
     channel(buf[2]);
     }  
     if(buf[0] == 'B') {
     band(buf[2]);
     }    
     if(buf[0] == 'T') {
     text(buf);
     }  
     buf_pos = 0;
     memset(buf, 0, 1500 * sizeof(char));
    }
  }

    
  if (!client.connected()) {
    //reconnect();
  }
  if(mqtt=1){
  client.loop();
  }



}
