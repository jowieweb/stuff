

#ifndef ESP_MY_H
#define ESP_MY_H

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <PubSubClient.h>
#include <ESP8266WiFi.h>

#include <WiFiClient.h>
#define MAX_SUBS 16
#define NOLED 1024
#define MAXWLAN 5

struct Wlan {
	String ssid;
	String pass;
	String mqtt;
};
typedef struct Wlan Wlan;

class ESPMy{

public:

	PubSubClient client;
	ESPMy();
	ESPMy(String ssid, String pass);
	
	void addWlan(String ssid, String pass);

	int loop();

	bool subscribe(const char* topic);

	void publish(const char* topic, const char* payload);

	bool connect(MQTT_CALLBACK_SIGNATURE, const char* mqttIP);	
	void connect();
	String getIP();
	void setConnectionLED(int16_t pin);
	void addToClientName(uint16_t add);


	void enableOTA(const char* hostname, const char* pass);
	void disconnect();
	
	void setLimitTrys(bool limit);
	
private:
	String ssid;
	String pass;
	WiFiClient wifiClient;
	String _clientName;
	uint8_t subCount = 0;
	int16_t ledPin = NOLED;
	bool mqttUsed = false;
	bool _useOTA = false;
	bool _OTArunning = false;
	char* _subscriptions[MAX_SUBS];
	void OTA_begin();
	void reconnect();	
	bool reconnectMQTT();
	String macToStr(const uint8_t* mac);
	void resubscribe();
	void setLED(bool on);
	uint16_t clientSuffix =0;
	bool limitTrys = false;
	Wlan wlans[MAXWLAN];
	int wlanCount = 0;
	int wlanUsed =0;
	
};

#endif


