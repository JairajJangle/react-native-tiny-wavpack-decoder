package com.tinywavpackdecoder

import android.util.Log
import com.facebook.react.bridge.*
import com.facebook.react.module.annotations.ReactModule
import com.facebook.react.modules.core.DeviceEventManagerModule

@ReactModule(name = TinyWavPackDecoderModule.NAME)
class TinyWavPackDecoderModule(reactContext: ReactApplicationContext) :
    ReactContextBaseJavaModule(reactContext) {

    companion object {
        const val NAME = "TinyWavPackDecoderModule"
        private const val TAG = "TinyWavPackDecoder"
        private const val EVENT_PROGRESS = "onProgressUpdate"

        init {
            System.loadLibrary("tiny-wavpack-decoder")
        }

        // Called from JNI
        @JvmStatic
        fun onProgress(progress: Float) {
            sInstance?.sendProgressEvent(progress)
        }

        // Keep a static reference to the module instance
        private var sInstance: TinyWavPackDecoderModule? = null

        // Native method declarations
        @JvmStatic
        external fun nativeDecodeWavPack(
            inputPath: String,
            outputPath: String,
            maxSamples: Int,
            bitsPerSample: Int
        ): String
    }

    init {
        sInstance = this
    }

    override fun getName(): String {
        return NAME
    }

    @ReactMethod
    fun decodeWavPack(
        inputPath: String,
        outputPath: String,
        maxSamples: Double?,
        bitsPerSample: Double?,
        promise: Promise
    ) {
        try {
            // Convert parameters to the correct types
            val maxSamplesInt = maxSamples?.toInt() ?: -1
            val bitsPerSampleInt = bitsPerSample?.toInt() ?: 16

            Log.d(TAG, "Decoding WavPack: $inputPath -> $outputPath")

            // Run in a background thread to avoid blocking the JS thread
            Thread {
                try {
                    val result = nativeDecodeWavPack(
                        inputPath,
                        outputPath,
                        maxSamplesInt,
                        bitsPerSampleInt
                    )

                    if (result == "Success") {
                        reactApplicationContext.runOnJSQueueThread {
                            promise.resolve(result)
                        }
                    } else {
                        reactApplicationContext.runOnJSQueueThread {
                            promise.reject("DECODE_ERROR", result)
                        }
                    }
                } catch (e: Exception) {
                    Log.e(TAG, "Error decoding WavPack", e)
                    reactApplicationContext.runOnJSQueueThread {
                        promise.reject("DECODE_ERROR", e.message, e)
                    }
                }
            }.start()
        } catch (e: Exception) {
            Log.e(TAG, "Error starting decode thread", e)
            promise.reject("DECODE_ERROR", e.message, e)
        }
    }

    // Send progress events to JavaScript
    private fun sendProgressEvent(progress: Float) {
        reactApplicationContext.runOnJSQueueThread {
            val params = Arguments.createMap().apply {
                putDouble("progress", progress.toDouble())
            }
            reactApplicationContext
                .getJSModule(DeviceEventManagerModule.RCTDeviceEventEmitter::class.java)
                .emit(EVENT_PROGRESS, params)
        }
    }

    @ReactMethod
    fun addListener(eventType: String) {
        // Required for RN built in Event Emitter Support
    }

    @ReactMethod
    fun removeListeners(count: Int) {
        // Required for RN built in Event Emitter Support
    }
}