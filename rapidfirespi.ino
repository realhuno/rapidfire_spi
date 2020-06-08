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

String mqttid="2"; //Change THIS .... MQTTID!!

#define WIFI_AP_NAME "Chorus32 LapTimer"



int heartbeat=0;
String ipa="10.0.0.50";
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


     t_state.trim();
     int len = strlen(t_state.c_str());
     String fill;
     int i;
     for (int i = len; i < 26; i++){
     fill=fill+" ";
     }
     t_state=t_state+fill;
     t_state.replace("%", " "); //replace % with space
     text(t_state);            //not working
  text(t_state);
  }  
  if(t_state.charAt(0) == 'D') {
   rfmodes(t_state.charAt(2));
  }  
 Serial1.println(t_state);
 Serial.println(t_state);
 global=global+t_state+"<br>";
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
 String rxvid=WiFi.macAddress();
 String topicmsg;
 int bandn;
 rxvid.replace(":", ""); //remove : from mac

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    line += (char)message[i];   
  }
  global=global+line+"<br>";
  Serial.println("Topic:");

   Serial.println(String(topic));
   Serial.println(line);
  
     //Rotorhazard send \n first 
     //   ->                    29UML1:Callsign 2 L1: 0:06.451%
     line.trim();
     //SWITCH CHANNEL
     if(line.charAt(0) == '0' && line.charAt(1) == '9' && line.charAt(2) == 'B' && line.charAt(3) == 'C') {   
     channel(line.charAt(4)+1);
     global=global+"CHANNEL: "+line.charAt(4)+"<br>";
     }
     //SWITCH BAND
     if(line.charAt(0) == '0' && line.charAt(1) == '9' && line.charAt(2) == 'B' && line.charAt(3) == 'G') { 
     bandn=line.charAt(4);   
     if(bandn==2){bandn=2;} //raceband
     if(bandn==4){bandn=1;} //fatshark
     if(bandn==3){bandn=3;} //e
     if(bandn==5){bandn=6;} //low
     if(bandn==1){bandn=4;} //b
     if(bandn==0){bandn=5;} //a
    
     band(bandn);
     global=global+"BAND: "+line.charAt(4)+"<br>";
     }
     //09BC7%    09B=command change freq..  c=raceband channel 7   
     //Change RotorHazard vrx id rx/cv1/cmd_esp_target/CV_246F28166140
     
     if(line.indexOf('node_number')){                      //<<command 
     int current_nr=line.substring(12,13).toInt();
     if(current_nr>0){
     //String stringnr="5";
      client.unsubscribe("rx/cv1/cmd_node/0");
      client.unsubscribe("rx/cv1/cmd_node/1");
      client.unsubscribe("rx/cv1/cmd_node/2");
      client.unsubscribe("rx/cv1/cmd_node/3");
      client.unsubscribe("rx/cv1/cmd_node/4");
      client.unsubscribe("rx/cv1/cmd_node/5");
      client.unsubscribe("rx/cv1/cmd_node/6");
      client.unsubscribe("rx/cv1/cmd_node/7");
      client.unsubscribe("rx/cv1/cmd_node/8");
      topicmsg = "rx/cv1/cmd_node/"+line.substring(12,13);                                            
                             
      client.subscribe(topicmsg.c_str());
     global=global+topicmsg+"<br>"; 
     char JSONmessageBuffer[100];
     StaticJsonBuffer<300> JSONbuffer;
     JsonObject& JSONencoder = JSONbuffer.createObject();
     //JSONencoder["node_number"] = String(6);
     JSONencoder["node_number"] = String(line.substring(12,13));

     JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
     
     
     topicmsg="status_variable/CV_"+rxvid;
     client.publish(topicmsg.c_str(), JSONmessageBuffer);
     
     
     global=global+"CHANGE NODE_NUMBER"+"<br>";
     //mqttid=current_nr;
     //reconnect();
     }
     }
      
   
                           //rx/cv1/cmd_esp_target/CV_00000001  

     if(line.charAt(0) == '0') {             //09UMFinish%
     Serial.println("Send text mqtt");
     String stringOne;
     stringOne="T="+line.substring(4,line.lastIndexOf('%')+3); //8
     //stringOne="T="+line; //8
     int len = strlen(stringOne.c_str());
     String fill;
     int i;
     for (int i = len; i < 24; i++){
     fill=fill+" ";
     }
     stringOne=stringOne+fill;
     stringOne.replace("%", " "); //replace % with space
     text(stringOne);            //not working
     global=global+stringOne+"<br>";
     }
     
     if(line.charAt(0) == '2') {          //Push in laptimes
     Serial.println("Send text mqtt");
     String stringOne;

     stringOne="T="+line.substring(5,line.lastIndexOf('%')+3); //8
     int len = strlen(stringOne.c_str());
     //fill with spaces
     String fill;
     int i;
     for (int i = len; i < 24; i++){
     fill=fill+" ";
     }
     stringOne=stringOne+fill;
     stringOne.replace("%", " "); //replace % with space

     
     text(stringOne);            //not working
     global=global+"ooooo";
     global=global+line.lastIndexOf('%');
     global=global+"ooooo"+"<br>";
     }
     
     if(line.charAt(0) == 'T') {  //Normal mqtt osd text working!!
     String t_state=line;
     t_state.trim();
     int len = strlen(t_state.c_str());
     String fill;
     int i;
     for (int i = len; i < 26; i++){
     fill=fill+" ";
     }
     t_state=t_state+fill;
     t_state.replace("%", " "); //replace % with space
     text(t_state);            //not working 


      
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
  global=global+"MQTTBUZZER"+"<br>";
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
    //INIT SPI 
  pinMode(SPI_SS_PIN, OUTPUT);
  pinMode(SPI_DATA_PIN, OUTPUT);
  pinMode(SPI_CLOCK_PIN, OUTPUT);
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

    
       delay(500);
    }



  digitalWrite(SPI_SS_PIN, HIGH);
  digitalWrite(SPI_CLOCK_PIN, HIGH);
  digitalWrite(SPI_DATA_PIN, HIGH);
  delay(200); //Let Rapidfire init SPI       
  
  digitalWrite(SPI_SS_PIN, LOW);
  digitalWrite(SPI_CLOCK_PIN, LOW);
  digitalWrite(SPI_DATA_PIN, LOW);
  delay(1000);
  digitalWrite(SPI_SS_PIN, HIGH);

  Serial.println("SPI Start");
  //memset(buf, 0, 1500 * sizeof(char));
  
  

 
 // MDNS.begin("radio0");
 // MDNS.addService("http", "tcp", 80);


Serial.println("OTA started");
 delay(250);
 

  ArduinoOTA.begin();




  delay(250); //Wait again 
  server.on("/", handleRoot);      //This is display page
  server.on("/readADC", handleADC);//To get update of ADC Value only
  server.on("/setLED", handleLED);
  server.on("/setTime", handleTime);
  server.on("/reconnect", reconnect);
  server.on("/led", led);

  server.begin();                  //Start server
  Serial.println("HTTP server started");

  //setup done... blink led
 pinMode(4,OUTPUT);
 digitalWrite(4,HIGH);
 delay(500);
 digitalWrite(4,LOW);
 delay(500);

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
Serial.println(len,DEC);
global=global+chksum+"<br>";
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
 ipa=server.arg("ip");
 String rxvid=WiFi.macAddress();
 String topic;
 rxvid.replace(":", ""); //remove : from mac
 global=global+rxvid+"<br>";
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
    topic = "rapidfire_"+rxvid;
    if (client.connect(topic.c_str())) {
      
      Serial.println("connected");
      // Subscribe
      //client.subscribe("esp32/output");
      //client.subscribe("rx/cv1/cmd_esp_all");
      //topic = "rx/cv1/cmd_node/"+mqttid;                    <<switch this to callback                                 
      //global=global+topic+"<br>";                          <<switch this to callback 
      
      //client.subscribe(topic.c_str());//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<rx/cv1/cmd_node/1
      client.subscribe("rx/cv1/cmd_all");
      topic = "rx/cv1/cmd_esp_target/CV_" + rxvid;
      client.subscribe(topic.c_str()); //change id new via mac
      //client.subscribe("rx/cv1/cmd_esp_target/CV_00000001"); //change id old
      
      global=global+"MQTT Connected"+"<br>";
      //String topic = "rxcn/CV_" + rxvid;     why two times? bottom....
      //client.publish(topic.c_str(), "1");


StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
 
  JSONencoder["dev"] = "rx";
  JSONencoder["ver"] = "todover";
  JSONencoder["fw"] = "todover";
  JSONencoder["nn"] = "rapidfire";
  
  //JsonArray& values = JSONencoder.createNestedArray("values");
 
 
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
 
 

     JSONencoder["node_number"] = "1";
     topic = "rxcn/CV_" + rxvid;
     client.publish(topic.c_str(), "1");
     delay(500);
     topic = "status_static/CV_" + rxvid;
     client.publish(topic.c_str(), JSONmessageBuffer);
  char JSONmessageBuffer2[100];
  JSONencoder.printTo(JSONmessageBuffer2, sizeof(JSONmessageBuffer2));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer2);
    delay(500);
     topic = "status_variable/CV_" + rxvid;
     global=global+topic+"<br>";
     client.publish(topic.c_str(), JSONmessageBuffer2);
//rx/cv1/cmd_node/1
 
    } else {
      mqtt=1;
      Serial.print("failed, rc=");
      Serial.print(client.state());
      global=global+"MQTT not Connected"+"<br>";
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
  ArduinoOTA.handle();


}
