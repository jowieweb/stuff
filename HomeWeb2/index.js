const config = require('./config.json');
const express = require("express");
const bodyParser = require("body-parser");
const path = require("path");
const mqtt = require("mqtt");
const app = express();
const WebSocket = require("ws");
const client = mqtt.connect(config.mqttIP);

const db = require("./db");
const wss = new WebSocket.Server({port:8081});

var ids;
var topicToId = new Map();
var idToValue = new Map();
var idToObject = new Map();
var wsCount = 0;

client.on("connect", function(){
	client.subscribe("/test");
	console.log("MQTT CONNECTED");
	
}); 

client.on("message", function (topic, msg){
	//console.log(topic + " " + msg.toString() );
	var mqttText = parseFloat(msg.toString());
	var id = topicToId.get(topic);
	var oldMsg = parseFloat(idToValue.get(id));

	
	obj =idToObject.get(id);
	if(obj == undefined || obj.type == undefined)
		return;
	
	if(isAnswer.get(topic)){
		idToValue.set(id,msg);
		
		if(obj.writeDB[0] == "1" && oldMsg != mqttText ){
			db.insertDB(obj.id, msg);
		}
	}
	var toSend={
		id: id, 
		message: msg.toString(),
		type: obj.type,
		isAnswer: isAnswer.get(topic)
		};
	sendToWS(toSend);
});

function sendToWS(obj){
	var text =JSON.stringify(obj);
	
	sockets.forEach(function(value, key) {
		try{
			value.send(text);
		}catch(err){
			sockets.delete(key);
		}
		
	}, sockets);
	
	
} 

var trons = db.getTrons();
var isAnswer = new Map();
var sockets = new Map();

var sub = function(list){
	console.log("CALLBACK");
	ids = list;
	for (var key in ids) {
		var topic = "";
		idToObject.set(ids[key].id, ids[key]);
		if(ids[key].type == "MJPG"){
			console.log("FOUND!");
			continue;
		} 
		if(ids[key].topic != undefined && ids[key].topic != ""){	
			topic = ids[key].topic;			
			topicToId.set(topic, ids[key].id);
			isAnswer.set(topic, false);
			idToValue.set(ids[key].id, "0");
			//client.subscribe(topic);
		}
		if(ids[key].topicAnswer != undefined && ids[key].topicAnswer != ""){	
			topic = ids[key].topicAnswer;			
			topicToId.set(topic, ids[key].id);			
			isAnswer.set(topic, true);
			idToValue.set(ids[key].id, "0");
			//client.subscribe(topic);
		}
		//console.log("SUB: " + topic);
		client.subscribe(topic);
		
		 
	}
	//console.log(idToValue);
	//console.log("");
	//console.log(idToObject);
};

db.getIDs(sub);

function chartDataRdy(data, wsID){
	
	
	var toSend={
		type: "Chart",
		data: data
	};
	sockets.get(wsID).send(JSON.stringify(toSend));
}
 

wss.on("connection", function connection(ws){
	
	ws.on("message", function incomming(message){
		console.log(message);
		var obj = JSON.parse(message);
		if(obj.type == 'chart'){
			console.log("CHART");
			db.getDataById(obj.id, chartDataRdy, obj.wsID);
			
		}else {
			client.publish(idToObject.get(Number(obj.id)).topic,""+ obj.state);
		}
	});
	var socketID = wsCount++;
	
	sockets.set(socketID, ws);
	
	ws.send(JSON.stringify({type:'id',id:socketID}));
	
	/* send current state to new WS */
	for (var i = 0; i < ids.length; i++) {
		if(ids[i].type == "MJPG"){
			continue;
		} 
		var toSend={
			id: idToObject.get(ids[i].id).id, 
			message: idToValue.get(ids[i].id).toString('utf8'),
			type: ids[i].type,
			isAnswer: true
		};
			
		
		ws.send(JSON.stringify(toSend));
		
		
	}
	
	 
	console.log("WS OK");
	//ws.send("from Server");
});

var newConnection = function(req, res, next){
	console.log("new conn.");
	next();
}



app.use(newConnection);

app.set("view engine", "ejs");
app.set("views", path.join(__dirname, "views"));

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({extended:false}));

app.use(express.static(path.join(__dirname, "public")));
app.use(express.static(path.join(__dirname, "public/bs")));
app.use(express.static(path.join(__dirname, "public/bs/css")));
app.use(express.static(path.join(__dirname, "public/bs/js")));

app.get("/",function(req, res){
	res.render("index",{
		titel: "my cool titel",
		msg: "",
		trons: trons,
		idToValue: idToValue,
		idToObjecta: idToObject
	});
	
	
}); 

app.get("/chart",function(req, res){
	res.render("chart",{
		trons: trons,
		idToValue: idToValue,
		idToObjecta: idToObject
	});
	
}); 

app.get("/reload",function(req, res){
	db.reload();
	res.write("<meta http-equiv=\"refresh\" content=\"3;url=/\"> ok");	
	res.end();
	topicToId = new Map();
	idToValue = new Map();
	idToObject = new Map();
	trons = db.getTrons();
    isAnswer = new Map();
	db.getIDs(sub);
});


app.get('/api', function(req, res){
	console.log(req.query.id);
	res.setHeader('Content-Type', 'application/json');
	db.getDataById(req.query.id,
	function(data,res){
		res.send((data));
	},res);
	
});

	
app.listen(3000, function(){
	console.log("started on 3000...");	
	
});

