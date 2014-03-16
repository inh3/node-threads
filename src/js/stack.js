// https://code.google.com/p/v8/wiki/JavaScriptStackTraceApi
(function() {
    return function (path) {
        var origPrepareStackTrace = Error.prepareStackTrace;
        Error.prepareStackTrace = function (_, stack) {
            return stack;
        };
        var stack = (new Error()).stack;
        Error.prepareStackTrace = origPrepareStackTrace;
        
        // return previous frame (which is the callee)
        var filename = stack[1].getFileName();
        return {
            __dirname: path.dirname(filename),
            __filename: filename
        };
    };
})();