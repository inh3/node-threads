// inheriting event notification
var EventEmitter = require('events').EventEmitter;
var Util = require('util');

// inherit from the event emitter
var NodeThreads = require('./build/Release/node-threads');
Util.inherits(NodeThreads, EventEmitter);

// export the constructor function
module.exports = NodeThreads;