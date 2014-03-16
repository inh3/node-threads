// export the constructor function
var NodeThreads = null;

try {
    NodeThreads = require('./build/Release/node-threads')(__dirname, process.cwd());
}
catch (error) {
    NodeThreads = require('./build/Debug/node-threads')(__dirname, process.cwd());
}

module.exports = NodeThreads;