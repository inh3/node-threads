// export the constructor function
var NodeThreads = null;

try {
    NodeThreads = require('./build/Release/node-threads')(process.cwd());
}
catch (error) {
    NodeThreads = require('./build/Debug/node-threads')(process.cwd());
}

module.exports = NodeThreads;