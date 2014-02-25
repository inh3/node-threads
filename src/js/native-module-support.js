/**
 * Determine if a native module is supported within the thread pool.
 *
 * @param {String} moduleName
 */
(function nativeModuleSupport(moduleName) {

    var nativeModules = [
        'path',
        'util'
    ];

    var isSupported = nativeModules.some(function(nativeModule) {
        if(moduleName === nativeModule) {
            return true;
        }

        return false;
    });

    return isSupported;
});