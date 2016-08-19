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
* Compile tools like make and gcc.

Example
=======

```javascript
var lirc_client = require('lirc_client');

try {
	lirc_client.connect("testone", true, "test1.lircrc",function(type, data, configFile){
		console.log("Type:", type);
		console.log("Data:", data);
		switch (type) {
			case "rawdata":
				console.log("Rawdata received:",data);
				break;
			case "data":
				console.log("Data received '%s' from configFile '%s'",data, configFile);
				break;
			case "closed":
				console.log("Lircd closed connection to us.");
				break;
		}
	});

	console.log("lirc_client.isConnected:", lirc_client.isConnected);
	console.log("lirc_client.mode:", lirc_client.mode);
	console.log("lirc_client.configFiles:", lirc_client.configFiles);

	console.close();
	console.log("lirc_client.isConnected:", lirc_client.isConnected);
}
catch (err) {
	console.log("Error on connect:",err);
}
```
See examples folder for more detailed example.

API Documentation
=================

Module Functions
---------------- 

* **connect**(< _String_ >programName, [< _Boolean_ >verbose], [< _String_ >configFiles], < _Function_ >callback(< _String_ >type[, < _String_ >data[, < _String_ >configFile]])) - Should be called to connect to lircd.
* **connect**(< _String_ >programName, [< _Boolean_ >verbose], [< _Array_ >configFiles], < _Function_ >callback(< _String_ >type[, < _String_ >data[, < _String_ >configFile]])) - Should be called to connect to lircd.
  * < _String_ >**programName** - Is the program name used to select right button in lircrc config files. Will be matched by lirc against the "prog" attribute. ([.lircrc file format](http://www.lirc.org/html/configure.html#lircrc_format))
  * < _Boolean_ >**verbose** - Will put the lirc library in verbose mode or not.
  * < _String_ >**configFiles** - Specify full or relative path to an existing lircrc file. ([.lircrc file format](http://www.lirc.org/html/configure.html#lircrc_format)). When undefined then the lirc default config files _/etc/lirc/lircrc_ and _~/.lircrc_ will be loaded.
  * < _Array_ >**configFiles** - Array of < _String_ > values. Each strings is a full or relative path to an existing lircrc file. ([.lircrc file format](http://www.lirc.org/html/configure.html#lircrc_format)). When undefined then the lirc default config files _/etc/lirc/lircrc_ and _~/.lircrc_ will be loaded.
  * < _Function_ >**callback(< _String_ >type[, < _String_ >data[, < _String_ >configFile]])** - Callback function which gets called when data is available or connection to lircd was closed outside of our control.
    * < _String_ >**type** - Specifies why the callback function was called. Following values are possible:
      * "rawdata" - Means data argument contains raw data from lircd.conf file. 
      * "data" - Means data argument contains config attribute from lircrc button which matches received ir button and prog attribute from lirrc file(s) and configFile will contain name of configFile as specified in connect or addConfig.
      * "closed" - Means connection to lircd was closed outside of our control. 
    * Will throw errors when something fails.

* **close**() - Closes the connection to lircd for this object.

* **reConnect**() - Reconnects a closed connection to the lircd for this object. It will reuse arguments from connect call.
  * Will throw errors when something fails.

* **addConfig**(< _String_ >configFile | < _Array_ >configFiles) - Add one or more lircrc config files.
  * < _String_ >**configFile** - Must contain the full or relative path to an existing lircrc file.
  * < _Array_ >**configFiles** - Is an array of _String_ values. Each string must contain the full or relative path to an existing lircrc file.

* **clearConfig**() - Will remove all lircrc config files from object. No more "data" events will be emited until at least one config has been added again.

Module properties
---------------- 

* < _Boolean_ >**isConnected** (read only) - When true is returned there is a connection to the lircd and the object will receive events. When false is returned the object is not connected to lircd and it will not receive events.

* < _String_ >**mode** (read/write) - Will perform lirc_getmode and lirc_setmode. Are not described on lirc website but were found in lirc_client.h file and on following [page](http://lirc.10951.n7.nabble.com/Patch-control-lirc-mode-from-external-program-td1611.html)

* < _Array_ >**configFiles** (read only) - Will contain a list with config filenames currently active. When array is empty the lirc default lircrc config files are used.


