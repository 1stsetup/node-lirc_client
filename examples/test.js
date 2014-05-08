var lirc_client = require('..');

try {
	lirc_client.connect("testone", true, "test1.lircrc",function(type, data){
		console.log("Type:", type);
		console.log("Data:", data);
	});

	console.log("isConnected:",lirc_client.isConnected);
	console.log("mode:",lirc_client.mode);
	console.log("configFiles:",lirc_client.configFiles);

	lirc_client.addConfig(["test2.lircrc", "test2a.lircrc"]);

	console.log("configFiles:",lirc_client.configFiles);

	lirc_client.close();
	console.log("isConnected:",lirc_client.isConnected);

	lirc_client.reConnect();
	console.log("configFiles:",lirc_client.configFiles);

/*	lirc_client.connect("testone", true, "test1.lircrc",function(type, data){
		console.log("Type:", type);
		console.log("Data:", data);
	});
*/
	console.log("isConnected:",lirc_client.isConnected);

}
catch (err) {
	console.log("Error on creating new lirc_client:",err);
}


