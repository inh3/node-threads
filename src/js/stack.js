(function() {
    return function (path) {
        var origPrepareStackTrace = Error.prepareStackTrace;
        Error.prepareStackTrace = function (_, stack) {
            return stack;
        };
        Error.stackTraceLimit = 20;
        var stack = (new Error()).stack;
        Error.prepareStackTrace = origPrepareStackTrace;
        
        // return previous frame (which is the callee)
        var filename = stack[1].getFileName();
        return {
            dirname: path.dirname(filename),
            filename: filename
        };
    };
})();