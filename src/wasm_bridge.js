/**
 * JavaScript bridge for Doom WASM module
 * These functions are called from C code via Emscripten
 */

mergeInto(LibraryManager.library, {
    /**
     * Render ASCII frame - called from DG_DrawFrame()
     * @param {number} framePtr - Pointer to ASCII frame string in WASM memory
     * @param {number} width - Frame width in characters
     * @param {number} height - Frame height in characters
     */
    JS_RenderASCIIFrame: function(framePtr, width, height) {
        const frame = UTF8ToString(framePtr);
        
        // Call global callback if registered
        if (typeof globalThis.JS_RenderASCIIFrame_Callback === 'function') {
            globalThis.JS_RenderASCIIFrame_Callback(frame, width, height);
        }
    },
    
    /**
     * Get next key from input queue
     * @returns {number} - Key code (0 if queue is empty)
     */
    JS_GetNextKey: function() {
        if (typeof globalThis.JS_GetNextKey_Callback === 'function') {
            return globalThis.JS_GetNextKey_Callback();
        }
        return 0;
    },
    
    /**
     * Log message from WASM
     * @param {number} messagePtr - Pointer to message string in WASM memory
     */
    JS_Log: function(messagePtr) {
        const message = UTF8ToString(messagePtr);
        
        // Call global callback if registered
        if (typeof globalThis.JS_Log_Callback === 'function') {
            globalThis.JS_Log_Callback(message);
        } else {
            console.log('[DOOM WASM]', message);
        }
    }
});
