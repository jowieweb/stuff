#LED Control adopted from : https://github.com/richardghirst/rpi_ws281x/blob/master/python/examples/strandtest.py
#https://learn.adafruit.com/neopixels-on-raspberry-pi/software
#needs PIP to install python lib!

import paho.mqtt.client as mqtt
from neopixel import *

BROKERIP       = "192.168.11.10"                 # ip address of the broker
RECV_TOPIC 	   = "/jonas/printer/light"			 # control topic for the LEDs
RETURN_TOPIC   = "/jonas/printer/light/return"   # Topic indicating the actual state of the LEDs 

LED_COUNT      = 64                   # Number of LED pixels.
LED_PIN        = 18                   # GPIO pin connected to the pixels (18 uses PWM!).
LED_FREQ_HZ    = 800000               # LED signal frequency in hertz (usually 800khz)
LED_DMA        = 10                   # DMA channel to use for generating signal (try 10)
LED_BRIGHTNESS = 255                  # Set to 0 for darkest and 255 for brightest
LED_INVERT     = False                # True to invert the signal (when using NPN transistor level shift)
LED_CHANNEL    = 0                    # set to '1' for GPIOs 13, 19, 41, 45 or 53
LED_ONCOLOR    = Color(200, 128, 128) # RGB color for on state


def ledOn():
	print("LIGHT ON")
	for i in range(strip.numPixels()):
		strip.setPixelColor(i,  LED_ONCOLOR)
	strip.show()
	client.publish(RETURN_TOPIC, payload="1", qos=2, retain=False)
	
def ledOff():
	print("LIGHT OFF")
	for i in range(strip.numPixels()):
		strip.setPixelColor(i,  Color(0, 0, 0))
	strip.show()
	client.publish(RETURN_TOPIC, payload="0", qos=2, retain=False)

def on_connect(client, userdata, flags, rc):
	print("Connected with result code "+str(rc))
	
	
def on_message(client, userdata, msg):
	print(msg.topic+" "+str(msg.payload))
	if str(msg.payload) == "1":
		ledOn();
	elif str(msg.payload) == "0":
		ledOff();
	
strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN, LED_FREQ_HZ, LED_DMA, LED_INVERT, LED_BRIGHTNESS, LED_CHANNEL)
try:
	client = mqtt.Client()
	client.will_set(RETURN_TOPIC, '0', 2, False)
	client.on_connect = on_connect
	client.on_message = on_message
	client.connect(BROKERIP, 1883, 60)
	client.subscribe(RECV_TOPIC, 2)
	strip.begin()
	client.loop_forever()
except KeyboardInterrupt:
	ledOff();
