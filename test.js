var addon = require('./build/Release/lirc_client');

try {
var lc = new addon.client("testje");
}
catch (err) {
	console.log("Error on new client:",err);
}


