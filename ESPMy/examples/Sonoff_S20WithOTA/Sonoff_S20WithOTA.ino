/*
 * Sketch for SONOFF S20 / S21 
 * Controll via MQTT / HTTP / Alexa
 * Supports OTA
 * Needs ESPMy library 
 * FAUXMO SEEMS TO WORK ONLY FOR WITH LIB VERSION 2.3.0 ON ESP BOARD VERSION 2.3.0
 */
#include <ESP8266mDNS.h>
#include "ESPMy.h"

#define SSID "SSID"
#define PASSWORD "PW"
#define RELAYPIN 12
#define BUTTONPIN 0
#define LEDPIN 13

#define OTANAME "SONOFF_S20"
#define OTAPW "MYCOOLPW"

#define MQTT
#ifdef MQTT
  #define TOPIC "/switchTopic"
  #define RETURN "/switchTopic/actualStatus"
  #define BROKER "192.168.11.10"
#endif

#define WEBINTERFACE
#define ALEXA

#ifdef ALEXA
  #define ALEXADEVNAME "Licht"
  #include "fauxmoESP.h"
  fauxmoESP fauxmo;
#endif

#ifdef WEBINTERFACE
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
String webPageON = "IS NOW ON!";
String webPageOFF = "IS NOW OFF!";
#endif


ESPMy myESP;
/* relay state. 0= off, 1=on */
int state = 0;
/* last relay state */
int oldstate = 0;
/* last update to mqtt */
int lastSend = 0;
/* indicate if interrupt was triggerd */
bool interrupted = false;
/* millis when last interrupt happend. used for debunce */
long lastInterrupt = 0;

/*
   setup function. called once
*/
void setup() {

pinMode(BUTTONPIN, INPUT);
  pinMode(RELAYPIN, OUTPUT);
  attachInterrupt(BUTTONPIN, interrupt, HIGH);
  Serial.begin(115200);
  Serial.println(TOPIC);

  myESP = ESPMy(SSID, PASSWORD);
  
  myESP.setDeviceName(OTANAME);
  myESP.setConnectionLED(LEDPIN);

  myESP.enableOTA(OTANAME, OTAPW);
#ifdef MQTT
  myESP.subscribe(TOPIC);
  myESP.connect(cb, BROKER);
#else
  myESP.connect();
#endif

#ifdef WEBINTERFACE
  setupWebinterface();
#endif

#ifdef ALEXA
  fauxmo.enable(true);
  fauxmo.addDevice(ALEXADEVNAME);
  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state) {
        if(state){
          on();
        } else {
          off();
        }
  });
  fauxmo.onGetState([](unsigned char device_id, const char * device_name) {
        return state ==1;
  });
#endif

  Serial.println("test");
}
/*
   loop function
*/
void loop() {
  #ifdef ALEXA
  fauxmo.handle();
#endif 
  myESP.loop();
  int timeNow = millis();
  /* check if the relay state has changed, or 1 minute has passed */
  if (state != oldstate || timeNow > lastSend + 600000) {
    lastSend = timeNow;
    /* publish the relay state via mqtt */
#ifdef MQTT
    if (state == 1) {
      myESP.publish(RETURN, "1");
    }
    else {
      myESP.publish(RETURN, "0");
    }
#endif
    oldstate = state;
  }

#ifdef WEBINTERFACE
  server.handleClient();
#endif


}

/*
  interrupt handler for button
*/
void interrupt() {
  /* check if bounced */
  if (lastInterrupt < millis() - 100) {
    lastInterrupt = millis();
    interrupted = true;
    toggle();
  }

}

/*
 * toggle the relay
 * from off to on
 * from on to off
 */
void toggle() {
  if (state == 1) {
    off();
  } else {
    on();
  }
}
#ifdef MQTT
/*
   callback for mqtt
*/
void cb(char* topic, byte* payload, unsigned int length) {
  if (payload[0] == '1') {
    on();
  } else if (payload[0] == '0') {
    off();
  } else if(payload[0] == 't'){
    toggle();
  }
}
#endif
/*
   turn the relay off
*/
void off() {
  digitalWrite(RELAYPIN, LOW);
  state = 0;
  /* force update */
  oldstate = -1;
  Serial.println("off");
  lastInterrupt = millis();
}

/*
   turn the relay on
*/
void on()
{
  digitalWrite(RELAYPIN, HIGH);
  state = 1;
  /* force update */
  oldstate = -1;
  Serial.println("on");
  lastInterrupt = millis();
}


#ifdef WEBINTERFACE
void setupWebinterface(){
    server.on("/", []() {
    if (state == 1) {
      server.send(200, "text/html", "IS CURRENTLY ON! '/off' to turn it off");
    } else if (state == 0) {
      server.send(200, "text/html", "IS CURRENTLY OFF! '/on' to turn it on");
    }
  });
  server.on("/on", []() {
    server.send(200, "text/html", webPageON);
    on();
  });
  server.on("/off", []() {
    server.send(200, "text/html", webPageOFF);
    off();
  });
  server.begin();
}

#endif