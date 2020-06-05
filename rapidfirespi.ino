#include <WiFi.h>
#include <WiFiMulti.h>
#include <esp_wifi.h>
#include <PubSubClient.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <ArduinoJson.h>
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

int heartbeat=0;

WiFiMulti wifiMulti;
// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "10.0.0.81";

WiFiClient espClient;
PubSubClient client(espClient);
//#define SPI_DATA_PIN 19   
//#define SPI_SS_PIN 5
//#define SPI_CLOCK_PIN 18

#define SPI_DATA_PIN 13 //12 
#define SPI_SS_PIN 15 // 13
#define SPI_CLOCK_PIN 12 //15

#define delaytime 70
#define MAX_BUF 1500
char buf[MAX_BUF];
uint32_t buf_pos = 0;
String global;
int mqtt=0;
long lastReconnectAttempt = 0;
String mqttserver;
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

 
void handleLED() {                                        //Handle webrequests

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
  if(t_state.charAt(0) == 'D') {
   rfmodes(t_state.charAt(2));
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

unsigned long hexToDec(String hexString) {
  unsigned long decValue = 0;
  char nextInt;
  for ( long i = 0; i < hexString.length(); i++ ) {
    nextInt = toupper(hexString[i]);
    if( isxdigit(nextInt) ) {
        if (nextInt >= '0' && nextInt <= '9') nextInt = nextInt - '0';
        if (nextInt >= 'A' && nextInt <= 'F') nextInt = nextInt - 'A' + 10;
        decValue = (decValue << 4) + nextInt;
    }
  }
  return decValue;
}

void callback(char* topic, byte* message, unsigned int length) {             //MQTT Callback
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
   Serial.println(line);
     //Rotorhazard send \n first 
     //   ->                    \n29UML1:Callsign 2 L1: 0:06.451%
   
     if(line.charAt(0) == '\n') {
     Serial.println("Send text mqtt");
     String stringOne;
     stringOne="T="+line.substring(8,31);
     Serial.println(stringOne);
     text(stringOne);            //not working
     }
     
     if(line.charAt(0) == 'T') {  //Normal mqtt osd text working!!
     text(line);
     }
     
     //LED Light
     if(line.charAt(0) == 'L') {
     if(line.charAt(2)=='0'){
     pinMode(4,OUTPUT);
     digitalWrite(4,LOW);
     }
     if(line.charAt(2)=='1'){
     pinMode(4,OUTPUT);
     digitalWrite(4,HIGH);
     }
     }

     //Rapidfire CMD
       
  if(line.charAt(0) == 'S') {
  buzzer();
  Serial.println("BUZZER");
  global="MQTTBUZZER";
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
     


}

void setup(void) {
  pinMode(33,OUTPUT);
  Serial.begin(115200);

  //WiFi.begin(WIFI_AP_NAME);
  
    wifiMulti.addAP("Chorus32 LapTimer", "");
    wifiMulti.addAP("Laptimer", "laptimer");
    wifiMulti.addAP("A1-7FB051", "hainz2015");

    Serial.println("Connecting Wifi...");
   /*
    if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }

  */
  Serial.println("WiFi not connected!");
     while(wifiMulti.run() != WL_CONNECTED) {
       Serial.print(".");

    
       delay(1000);
    }




      Serial.print("X");

    
       delay(1000);
      Serial.print("Y");

    
       delay(1000);
  
      Serial.print("Z");

    
       delay(1000);
  //memset(buf, 0, 1500 * sizeof(char));






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
  //memset(buf, 0, 1500 * sizeof(char));
  
  /*
  
  delay(1000); //Wait again 
 
  MDNS.begin("radio0");
  MDNS.addService("http", "tcp", 80);
// client.setServer(mqtt_server, 1883);
//  client.setCallback(callback);

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

Serial.println("OTA started");
 delay(1000);
 

  ArduinoOTA.begin();

*/


  delay(1000); //Wait again 
  server.on("/", handleRoot);      //This is display page
  server.on("/readADC", handleADC);//To get update of ADC Value only
  server.on("/setLED", handleLED);
  server.on("/setTime", handleTime);
  server.on("/reconnect", reconnect);
  server.on("/led", led);

  server.begin();                  //Start server
  Serial.println("HTTP server started");
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


void text_example(String text){
digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);  
bitBangData(84); // data transmission   T
bitBangData(61); // data transmission   =
bitBangData(5); // data transmission    (5 buchstaben also länge)
bitBangData(138); // data transmission  (checksum)
bitBangData(72); // data transmission   H
bitBangData(101); // data transmission  E
bitBangData(108); // data transmission  L
bitBangData(108); // data transmission  L
bitBangData(111); // data transmission  O
delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);
}

void text(String text){                               //Rapidfire OSD TEXT

//Checksum
int len = strlen(text.c_str());
int sum;
int buf;
   
for(int i=0; i<len; i++){
buf=text.charAt(i);
sum=sum+(int(buf));
}
sum=sum+(len-2);
String stringOne =  String(sum, HEX);  
String newString = stringOne.substring(stringOne.length() - 2, stringOne.length());
int chksum=hexToDec(newString);
//checksum

digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);  
bitBangData(84); // data transmission   OSD MODE
bitBangData(61); // data transmission
bitBangData(len-2); // data transmission
bitBangData(chksum); // data transmission 138
   for(int i=2; i<len; i++){
   //Serial.println(int(text.charAt(i)));
   bitBangData(int(text.charAt(i)));


 }

delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);

Serial.println(chksum);
global=chksum;
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


void rfmodes(int i){
  digitalWrite(SPI_SS_PIN, LOW);
delayMicroseconds(delaytime);
bitBangData(68); // data transmission
bitBangData(61); // data transmission
bitBangData(1); // data transmission
bitBangData(130+(i-48)); // data transmission  145 oder so pal // 147  146      141
bitBangData(i-48); // data transmission  3               //     6    5    0
delayMicroseconds(delaytime);
digitalWrite(SPI_SS_PIN, HIGH);
Serial.println(i-48);
//0=rapidfire1
//1=rapidfire2
//2=legacy
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
boolean reconnect() {                                       //MQTT Connect and Reconnect
  // Loop until we're reconnected
 mqtt=1;
   String ipa=server.arg("ip");

   if(ipa.length()>=5){
    Serial.print("New MQTT connection...");
    mqttserver=ipa;
   }else{
    Serial.print("MQTT reconnect...");
   }
   
   
  client.setServer(mqttserver.c_str(), 1883);
  client.setCallback(callback);
  
    Serial.print("Attempting MQTT connection...");
    Serial.println(mqttserver.c_str());
    // Attempt to connect
    if (client.connect("rapidfire_1")) {
      
      Serial.println("connected");
      // Subscribe
      //client.subscribe("esp32/output");
      //client.subscribe("rx/cv1/cmd_esp_all");
      
      client.subscribe("rx/cv1/cmd_node/1");
      global="MQTT Connected";
      client.publish("rxcn/CV_00000001", "1");


StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
 
  JSONencoder["dev"] = "rx";
  JSONencoder["ver"] = "todover";
  JSONencoder["fw"] = "todover";
  JSONencoder["nn"] = "cvtest1";
  
  //JsonArray& values = JSONencoder.createNestedArray("values");
 
 
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
 
 

       JSONencoder["node_number"] = "1";
     client.publish("rxcn/CV_00000001", "1");
     delay(500);
     client.publish("status_static/CV_00000001", JSONmessageBuffer);
  char JSONmessageBuffer2[100];
  JSONencoder.printTo(JSONmessageBuffer2, sizeof(JSONmessageBuffer2));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer2);
    delay(500);
     client.publish("status_variable/CV_00000001", JSONmessageBuffer2);
//rx/cv1/cmd_node/1
 
    } else {
      mqtt=1;
      Serial.print("failed, rc=");
      Serial.print(client.state());
      global="MQTT not Connected";
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(500);
    }
  return client.connected();  
}
void loop(void) {
  server.handleClient();
 if (!client.connected() && mqtt==1) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) { // Try to reconnect.
      lastReconnectAttempt = now;
      if (reconnect()) { // Attempt to reconnect.
        lastReconnectAttempt = 0;
      }
    }
  } 
if(mqtt==1){ // Connected.
    client.loop();
    //client.publish(channelName,"Hello world!"); // Publish message.
    //Serial.println(mqtt);
 
  }



}
