Description
===========

node-lirc_client provides a lirc_client object so your nodejs application
can receive lirc command.

Requirements
============

* [node.js](http://nodejs.org) -- v0.10.0+
* [lirc](http://www.lirc.org/) -- v0.9.0
  * lirc-devel(Fedora based), liblircclient-dev(raspbian), liblircclient-dev(Ubuntu)  package is required for lirc_client.h header file.

Example
=======

```javascript
var lirc_client = require('lirc_client');

try {
	var client = new lirc_client.client("clienttest", true);
	client.on("data", function(data) {
		console.log("Received data from lirc:",data);
		client.close();
	});
	client.on("closed", function() {
		console.log("Lirc daemon closed our connection. We need to reconnect.");
	});
	console.log("client.isConnected:", client.isConnected);
}
catch (err) {
	console.log("Error on new client:",err);
}
```
See examples folder for more detailed example.

API Documentation
=================

Module Functions
---------------- 

* **client**([< _String_ >programName], [< _Boolean_ >verbose], [< _Array_ >configFiles]) - Should be called with new to create new lirc_client object.
  * programName is required when no other lirc_client object is connected.
  * programName and verbose will be ignored when another lirc_client object is allready connected.
  * Will throw errors when something fails.
  * **!! Currently the configFiles option is ignored so only the global lircrc (/etc/lirc/lircrc) and the personal lircrc (~/.lircrc) are used !!**

* **close**() - Closes connection to lircd for this object.

* **connect**() - Reconnects a closed connection to the lircd for this object. It will reuse the active programName and verbose values.
** Will throw errors when something fails.

Module Functions
---------------- 

* < _Boolean_ >**isConnected** - When true there is a connection to the lircd and the object will receive events. When false object is not connected to lircd and it will not receive events.

Module Events
-------------

* **data**(< _String_ > data) - Event is emited when there is data available on the lircd connection. Data will contain config string from lircrc file for button pressed.

* **closed**() - Event is emited when lircd has closed our connection. 



