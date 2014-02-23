// export the constructor function
var NodeThreads = null;

try {
    NodeThreads = require('./build/Release/node-threads')(__dirname);
}
catch (error) {
    NodeThreads = require('./build/Debug/node-threads')(__dirname);
}

module.exports = NodeThreads;