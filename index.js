var EventEmitter = require('events').EventEmitter,
    addon = require('./build/Release/lirc_client');

addon.client.prototype.__proto__ = EventEmitter.prototype;

module.exports = addon;
