var lirc_client = require('..');

try {
	var client1 = new lirc_client.client("testone", true);
	client1.on("data", function(data) {
		console.log("Received data from lirc client1:",data);
	});
	client1.on("closed", function() {
		console.log("Lirc daemon closed our connection.");
	});

	console.log(" Before close. client1.isClosed:",client1.isClosed);

	client1.close();

	console.log(" After close. client1.isClosed:",client1.isClosed);

	client1.connect();

	console.log(" After connect. client1.isClosed:",client1.isClosed);

	client1.close();

	// Next line will trigger an error because client1 is closed and we did not specify a programname
	// When no other lirc_client object is open and we instantiate a new object we need to specify a programname
	var client2 = new lirc_client.client(); 
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


