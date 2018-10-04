
#include "ESPMy.h"

ESPMy::ESPMy(){	}


ESPMy::ESPMy(String ssid, String pass){
	wlans[0].ssid = ssid;
	wlans[0].pass = pass;
	wlanCount=1;
	wlanUsed = 0;
}

void ESPMy::addWlan(String ssid, String pass){
	if(wlanCount >= MAXWLAN){
		Serial.println("MAX WLAN COUNT REACHED!");
		return;
	}
	wlans[wlanCount].ssid = ssid;
	wlans[wlanCount].pass = pass;
	wlanCount++;	
	
}


void ESPMy::connect(){
	Serial.println("CONNECT!");
	
	_clientName += "esp8266-";
	uint8_t mac[6];
	WiFi.macAddress(mac);
	_clientName += macToStr(mac);
	WiFi.mode(WIFI_STA);		
	reconnect();	
}

bool ESPMy::connect(MQTT_CALLBACK_SIGNATURE, const char* mqttIP){
	
	connect();
	
	client = PubSubClient(mqttIP, 1883, wifiClient);	
	mqttUsed = true;
	client.setCallback(callback);
	return reconnectMQTT();	

}


void ESPMy::reconnect(){
	Serial.println("---Reconnecting WIFI---");
	
	WiFi.disconnect();

	//attempt to connect to the wifi if connection is lost
	WiFi.begin(wlans[wlanUsed].ssid.c_str(), wlans[wlanUsed].pass.c_str());

	int count = 0;
	while(WiFi.status() != WL_CONNECTED && count < 10){
		setLED(true);
		count++;
		delay(500);
		Serial.println(".");		
		setLED(false);
		delay(500);
	}

	
	Serial.print("connecting to wlan took ");
	Serial.print(count);
	Serial.print(" sec. IP is: \n");	
	
	Serial.println(WiFi.localIP().toString());
	if(WiFi.status() == WL_CONNECTED){
		Serial.println("---WIFI Connected---\n");
		setLED(false);
	}else if (wlanCount>1){
		Serial.println("trying next wlan");
		if(wlanUsed+1 < wlanCount){
			wlanUsed++;
		} else {
			wlanUsed = 0;
		}
		Serial.println(wlans[wlanUsed].ssid);
	}
}

bool ESPMy::reconnectMQTT(){
	Serial.println("---Reconnecting MQTT---");
	if(!mqttUsed){
		return false;
	}
	if(client.connected()){
		return true;
	}
		
	if(!client.connect((char*) _clientName.c_str())){
		Serial.println("---!!MQTT Connect Failed!!---");
		return false;
	} else {
		
		client.publish("/debug","1");
		resubscribe();
		
		if(client.connected()){
			Serial.println("---MQTT Connected---\n");
		}
		return true;
	}
	
	return false;
}

void ESPMy::disconnect(){
	client.disconnect ();
	WiFi.disconnect();
}


void ESPMy::resubscribe(){
	for(int i = 0; i < subCount; i++){
		if(_subscriptions[i]){
			client.subscribe(_subscriptions[i],1);
			yield();		
		}
	}
}

bool ESPMy::subscribe(const char* topic){

	if(subCount < MAX_SUBS){
		char* newTopic = (char*) malloc(sizeof(char)* (strlen(topic) +1));
		
		memcpy ( newTopic, topic, strlen(topic)+1 );
		_subscriptions[subCount] = newTopic;
		subCount++;		
		if(mqttUsed){
			client.subscribe(newTopic,1);
		}
	}
	
	return true;
}


void ESPMy::publish(const char* topic, const char* payload){
	if(!mqttUsed)
		return;
	
	if(!client.connected()){
		reconnectMQTT();
	}
	if(!client.connected()){
		return;
	}
	
	client.publish(topic,payload);
}



int ESPMy::loop(){

	if(WiFi.status() != WL_CONNECTED){
		Serial.println("NOT CONNECTED!");
		if(!limitTrys){
			reconnect();
		}else {
			return 0;
		}
	}
	if(mqttUsed){
		if(!client.connected()){
			reconnectMQTT();
		}
		client.loop();
	}
	if(_useOTA && _OTArunning) {ArduinoOTA.handle();}
	else if(_useOTA && !_OTArunning){
		OTA_begin();
		ArduinoOTA.handle();
	}
	
	
	delay(1);
	yield();
	return 0;
}

void ESPMy::enableOTA(const char* hostname, const char* pass){
	_useOTA = true;
	ArduinoOTA.setPassword(pass);
	ArduinoOTA.setHostname(hostname);	
	OTA_begin();
	
	ArduinoOTA.onStart([]() {/* ota start code */ Serial.println("OTA START");});
	ArduinoOTA.onEnd([]() {
		WiFi.disconnect();
		int timeout = 0;
		while(WiFi.status() != WL_DISCONNECTED && timeout < 200){
			delay(10);
			timeout++;
		}
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {/* ota progress code */});
	ArduinoOTA.onError([](ota_error_t error) {
		Serial.println("OTA ERR"); 
		Serial.println(error);
		
		     if      (error == OTA_AUTH_ERROR   ) Serial.println("Auth Failed");
			else if (error == OTA_BEGIN_ERROR  ) Serial.println("Begin Failed");
			else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
			else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
			else if (error == OTA_END_ERROR    ) Serial.println("End Failed");
		
		});
	
}

void ESPMy::OTA_begin(){	
	ArduinoOTA.begin();
	_OTArunning = true;	
}


String ESPMy::getIP(){
	return WiFi.localIP().toString();
}

void ESPMy::setLimitTrys(bool limit){
	limitTrys= limit;
}

void ESPMy::setConnectionLED(int16_t pin){
	pinMode(pin,OUTPUT);
	digitalWrite(pin, LOW);
	ledPin = pin;
}

String ESPMy::macToStr(const uint8_t* mac){ 
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5){
      result += ':';
    }
  }
  result += clientSuffix;
  return result;
}
void ESPMy::addToClientName(uint16_t add){
	clientSuffix = add;
}

void ESPMy::setLED(bool on){
	if(ledPin != NOLED){
		digitalWrite(ledPin, !on);
	}
}

