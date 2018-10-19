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
  /* input button of sonoff is on 0 */
  pinMode(0, INPUT);
  /* LED is on 12 */
  pinMode(12, OUTPUT);
  attachInterrupt(0, interrupt, HIGH);
  Serial.begin(115200);
  Serial.println(TOPIC);

  myESP = ESPMy(SSID, PASSWORD);
  myESP.setConnectionLED(13);

  myESP.enableOTA(OTANAME, OTAPW);
  myESP.subscribe(TOPIC);
  myESP.connect(cb, BROKER);
  Serial.println("test");
}

/*
   loop function
*/
void loop() {
  myESP.loop();
  int timeNow = millis();
  /* check if the relay state has changed, or 1 minute has passed */
  if (state != oldstate || timeNow > lastSend + 600000) {
    lastSend = timeNow;
    /* publish the relay state via mqtt */
    if (state == 1) {
      myESP.publish(RETURN, "1");
    }
    else {
      myESP.publish(RETURN, "0");
    }
    oldstate = state;
  }

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

/*
   turn the relay off
*/
void off() {
  digitalWrite(12, LOW);
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
  digitalWrite(12, HIGH);
  state = 1;
  /* force update */
  oldstate = -1;
  Serial.println("on");
  lastInterrupt = millis();
}