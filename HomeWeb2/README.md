# HomeWeb2
Simple WebApp based on Node.js allowing to control and monitor MQTT based SmartHome instances.
Uses WebSockets to keep the states in sync across multiple users.
Web design inspired by Homeassistant

Configuration done in config.json file:
```javascript
{
	"mqttIP" : "mqtt://127.0.0.1",
	"mysqlIP" : "127.0.0.1",
	"mysqluser" : " ",
	"mysqlpw"	: " ",
	"mysqldb"	: " "
}
```

