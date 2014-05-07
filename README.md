[![NPM](https://nodei.co/npm/lirc_client.png)](https://nodei.co/npm/lirc_client/)

Description
===========

node-lirc_client provides a lirc_client object so your nodejs application
can receive lirc command.

It is based on the lirc_client [example](http://www.lirc.org/html/technical.html#library) 

Installation
============
```text
npm install lirc_client
````

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
	client.on("rawdata", function(data) {
		console.log("Received rawdata from lirc:",data);
		client.close();
	});
	client.on("closed", function() {
		console.log("Lirc daemon closed our connection. We need to reconnect.");
	});
	console.log("client.isConnected:", client.isConnected);
	console.log("client.mode:", client.mode);
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
  * < _String_ >**programName** - Is the program name used to select right button in lircrc config files. Will be matched by lirc against the "prog" attribute. ([.lircrc file format](http://www.lirc.org/html/configure.html#lircrc_format))
  * < _Boolean_ >**verbose** - Will put the lirc library in verbose mode or not.
  * < _Array_ >**configFiles** - Specify zero or multiple lircrc files. Is an array of _String_ values. Each string must contain the full or relative path to an existing lircrc file. ([.lircrc file format](http://www.lirc.org/html/configure.html#lircrc_format)). When undefined or array length is 0 (zero) then the lirc default config files _/etc/lirc/lircrc_ and _~/.lircrc_ will be loaded.
    * programName is required when no other lirc_client object is connected.
    * programName and verbose will be ignored when another lirc_client object is allready connected.
    * Will throw errors when something fails.

* **close**() - Closes the connection to lircd for this object.

* **connect**() - Reconnects a closed connection to the lircd for this object. It will reuse the active programName and verbose values.
  * Will throw errors when something fails.

Module properties
---------------- 

* < _Boolean_ >**isConnected** (read only) - When true is returned there is a connection to the lircd and the object will receive events. When false is returned the object is not connected to lircd and it will not receive events.

* < _String_ >**mode** (read/write) - Will perform lirc_getmode and lirc_setmode. Are not described on lirc website but were found in lirc_client.h file and on following [page](http://lirc.10951.n7.nabble.com/Patch-control-lirc-mode-from-external-program-td1611.html)

Module Events
-------------

* **rawdata**(< _String_ > data) - Event is emited when there is data available on the lircd connection. The first argument to the callback function will contain the raw lirc data for the button pressed. (code from lirc_client function "int lirc_nextcode(char **code);")

* **data**(< _String_ > data) - Event is emited when there is data available on the lircd connection. The first argument to the callback function will contain the config string from lircrc file for the button pressed and the specified programname when client was created. (string from lirc_client function "int lirc_code2char(struct lirc_config *config,char *code,char **string);")

* **closed**() - Event is emited when lircd has closed our connection. 



