var lirc_client = require('..');

try {
	var client1 = new lirc_client.client("testone", true, ["test1.lircrc","test2.lircrc"]);
	client1.on("data", function(data) {
		console.log("Received data from lirc client1:",data);
	});
	client1.on("rawdata", function(data) {
		console.log("Received rawdata from lirc client1:",data);
	});
	client1.on("closed", function() {
		console.log("Lirc daemon closed our connection.");
	});

	console.log(" client1.mode:", client1.mode);
	console.log(" client1.configFiles:", client1.configFiles);
	client1.addConfig("test2a.lircrc");
	console.log(" client1.configFiles:", client1.configFiles);

	client1.clearConfig();
	console.log(" client1.configFiles:", client1.configFiles);

	client1.addConfig(["test2b.lircrc", "test2c.lircrc"]);
	console.log(" client1.configFiles:", client1.configFiles);

	console.log(" Before close. client1.isConnected:",client1.isConnected);

	client1.close();

	console.log(" After close. client1.isConnected:",client1.isConnected);

	client1.connect();

	console.log(" After connect. client1.isConnected:",client1.isConnected);

	client1.close();  

	var client2 = new lirc_client.client("testtwo", ["test3.lircrc"]); 
	client2.on("data", function(data) {
		console.log("Received data from lirc client2:",data);
	});
	client2.on("closed", function() {
		console.log("Lirc daemon closed our connection.");
	});

	console.log("Second created.");

}
catch (err) {
	console.log("Error on creating new lirc_client:",err);
}


