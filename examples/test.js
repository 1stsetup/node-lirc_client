var lirc_client = require('..');

try {
	var client = new lirc_client.client("testje", true);
	client.on("data", function(data) {
		console.log("Received data from lirc:",data);
	});
	client.on("closed", function() {
		console.log("Lirc daemon closed our connection.");
	});
}
catch (err) {
	console.log("Error on new client:",err);
}


