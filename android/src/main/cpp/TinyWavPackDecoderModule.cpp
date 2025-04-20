#include <jni.h>
#include <string>
#include <android/log.h>
#include "TinyWavPackDecoderInterface.h"

#define TAG "TinyWavPackDecoder"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

// Cache for the Java VM and class references
static JavaVM *javaVM = nullptr;
static jclass decoderClass = nullptr;
static jmethodID onProgressMethod = nullptr;

// Callback function to report progress back to Java
static void progressCallbackBridge(float progress, void* context) {
    JNIEnv *env;
    // Get the JNI environment
    jint result = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6);
    
    if (result == JNI_OK) {
        // Call the onProgress method in Java
        env->CallStaticVoidMethod(decoderClass, onProgressMethod, progress);
    } else if (result == JNI_EDETACHED) {
        // Thread is detached, attach it
        if (javaVM->AttachCurrentThread(&env, nullptr) == JNI_OK) {
            env->CallStaticVoidMethod(decoderClass, onProgressMethod, progress);
            javaVM->DetachCurrentThread();
        } else {
            LOGE("Failed to attach thread for progress callback");
        }
    } else {
        LOGE("Failed to get JNI environment for progress callback");
    }
}

// The JNI implementation of the decode method
extern "C" JNIEXPORT jstring JNICALL
Java_com_tinywavpackdecoder_TinyWavPackDecoderModule_nativeDecodeWavPack(
        JNIEnv *env,
        jclass clazz,
        jstring j_input_path,
        jstring j_output_path,
        jint j_max_samples,
        jint j_bits_per_sample) {
    
    // Convert Java strings to C strings
    const char *inputPath = env->GetStringUTFChars(j_input_path, nullptr);
    const char *outputPath = env->GetStringUTFChars(j_output_path, nullptr);
    
    // Log the parameters
    LOGI("Decoding WavPack: input=%s, output=%s, maxSamples=%d, bitsPerSample=%d",
         inputPath, outputPath, j_max_samples, j_bits_per_sample);
    
    // Call the decoder
    DecoderResult result = decode_wavpack_to_wav(
            inputPath,
            outputPath,
            j_max_samples,
            j_bits_per_sample,
            progressCallbackBridge,
            nullptr);
    
    // Release the C strings
    env->ReleaseStringUTFChars(j_input_path, inputPath);
    env->ReleaseStringUTFChars(j_output_path, outputPath);
    
    // Return the result
    if (result.success) {
        return env->NewStringUTF("Success");
    } else {
        return env->NewStringUTF(result.error);
    }
}

// Called when the library is loaded
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    javaVM = vm;
    JNIEnv *env;
    
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }
    
    // Cache the Java class and method IDs
    jclass localClass = env->FindClass("com/tinywavpackdecoder/TinyWavPackDecoderModule");
    if (localClass == nullptr) {
        LOGE("Failed to find TinyWavPackDecoderModule class");
        return JNI_ERR;
    }
    
    decoderClass = (jclass)env->NewGlobalRef(localClass);
    onProgressMethod = env->GetStaticMethodID(decoderClass, "onProgress", "(F)V");
    
    if (onProgressMethod == nullptr) {
        LOGE("Failed to find onProgress method");
        return JNI_ERR;
    }
    
    return JNI_VERSION_1_6;
}