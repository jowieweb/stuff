(function() {
    const config = require('./config.json');
	const mysql = require('mysql');
	
	var con = mysql.createConnection({
	  host: config.mysqlIP,
	  user: config.mysqluser,
	  password: config.mysqlpw,
	  database: config.mysqldb
	});
	
	var sql = "SELECT * FROM action";
	function connectDB(){
		con.connect(function(err) {
		  if (err) throw err;
		  console.log("Connected!");
		  con.query(sql, function (err, result, fields) {
			if (err) throw err;
			console.log("DB OKAY!");
		  });
		});
	} 
	var mqttMap = [];
	var callback;
	function getMQTT(){
		mqttMap = [];
		con.query("select id, mqttTopic, mqttTopicAnswer,writeDB,  type  from Element where mqttTopic is not null and mqttTopicAnswer is not null", function(err, result, fields){
			if (err) throw err;	
			for (var key in result) {
				mqttMap.push(
				{
					id: result[key].id, 
					topic: result[key].mqttTopic, 
					topicAnswer: result[key].mqttTopicAnswer,
					type: result[key].type,
					writeDB: result[key].writeDB
				});
			}
			callback(mqttMap);		 
		});
	}
	
	var myMap = new Map();
	function createTrons(){
		myMap = new Map();
		con.query("SELECT a.titel as mainTron, b.titel as subTron, c.* FROM Element AS a JOIN Element AS b ON a.id = b.TRON_ID JOIN Element as c on b.id = c.TRON_ID WHERE a.type = 'Tron' AND a.TRON_ID IS NULL", function (err, result, fields) {
			if (err) throw err;		
			
			for (var key in result) {
				var subMap = new Map();
				var guielement = [];
				if(myMap.get(result[key].mainTron) == undefined){
					guielement.push(result[key]);				
					subMap.set(result[key].subTron, guielement);
					myMap.set(result[key].mainTron, subMap);
				} else {
					subMap =  myMap.get(result[key].mainTron)
					if(subMap.get(result[key].subTron) == undefined){
						guielement.push(result[key]);
						subMap.set(result[key].subTron, guielement);					
						myMap.set(result[key].mainTron,subMap);
					} else {
						guielement = subMap.get(result[key].subTron);
						guielement.push(result[key]);
						subMap.set(result[key].subTron, guielement);					
						myMap.set(result[key].mainTron,subMap);
					}			
				}
				
			}
			
		});
	}

	createTrons();
	getMQTT();
 
	
	module.exports.getTrons = function(){		
	  return myMap; 
	};
	
	module.exports.getIDs = function(cb){	
	  mqttMap = [];
	  console.log("MAP " + mqttMap);
	  callback = cb;
	  return mqttMap;
	};
	
	module.exports.insertDB = function(id, value){
		var data =[id, value, '1'];
		 var sql = "INSERT INTO Data (guiID, value, time) VALUES (" + id + ",'" + value + "', UNIX_TIMESTAMP());";
		  con.query(sql, data, function (err, result, fields) {
			if (err) throw err;
			
		  });
		
	}

	module.exports.getDataById = function(id, callback, wsID){
		//var sql = "Select value, time from Data where guiID = '" + id + "' and time > DATE(NOW()-INTERVAL 7 DAY)";
		//var sql = "Select Data.value, time, Data.id ,titel, type from Data join Element on Element.id = Data.guiID where guiID = '" + id + "' and `Data`.id > (Select id from Data where guiID ='" + id + "' and time > UNIX_TIMESTAMP(DATE(NOW()-INTERVAL 7 DAY)) limit 1) "
		
		var sql = "Select Data.value, time, Data.id ,titel, type from Data join Element on Element.id = Data.guiID where guiID = '" + id + "' and time > UNIX_TIMESTAMP(DATE(NOW()-INTERVAL 7 DAY))";
		con.query(sql, function(err, result, fileds){
			if(err) throw err;
				console.log("got chart");
				callback(result, wsID);				
			
		});
		
	}
	module.exports.reload = function(){
		createTrons();
		getMQTT();
	}
	


}());


/* result[key].mainTron,result[key].subTron
SELECT a.titel, b.titel, c.* FROM GuiElement AS a JOIN GuiElement AS b ON a.id = b.TRON_ID JOIN GuiElement as c on b.id = c.TRON_ID WHERE a.DTYPE = 'Tron' AND a.TRON_ID IS NULL
*/