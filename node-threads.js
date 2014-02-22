// export the constructor function
var NodeThreads = null;

try {
    NodeThreads = require('./build/Release/node-threads');
}
catch (error) {
    NodeThreads = require('./build/Debug/node-threads');
}

module.exports = NodeThreads;