#include "TinyWavPackDecoderInterface.h"
#include "wavpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Static file pointer for wavpack read_bytes callback
static FILE *currentWvFile = NULL;

// WavPack callback for reading bytes from file
static int read_bytes(void *data, int bcount)
{
    return (int)fread(data, 1, bcount, currentWvFile);
}

// Define error codes
#define ERR_FILE_OPEN_FAILED 1
#define ERR_WAVPACK_OPEN_FAILED 2
#define ERR_MEMORY_ALLOCATION_FAILED 3
#define ERR_INVALID_CHANNELS 4
#define ERR_INVALID_SAMPLES 5
#define ERR_DATA_OVERFLOW 6
#define ERR_WRITE_FAILED 7
#define ERR_DECODE_FAILED 8
#define ERR_INVALID_BPS 9

// Helper function to create and return an error result
static DecoderResult make_error(int code, const char *message)
{
    DecoderResult result;
    result.success = 0;
    strncpy(result.error, message, sizeof(result.error) - 1);
    result.error[sizeof(result.error) - 1] = '\0';
    return result;
}

// Helper function to create and return a success result
static DecoderResult make_success()
{
    DecoderResult result;
    result.success = 1;
    result.error[0] = '\0';
    return result;
}

// Helper function to format samples to the desired bits per sample
static void format_samples(int bps, void *dst, int32_t *src, uint32_t samcnt, int is_float, int bitsPerSample)
{
    switch (bps)
    {
    case 1:
    { // 8-bit
        uint8_t *d = (uint8_t *)dst;
        for (uint32_t i = 0; i < samcnt; i++)
        {
            if (is_float)
            {
                float val = *(float *)(&src[i]);
                val = val > 1.0f ? 1.0f : (val < -1.0f ? -1.0f : val);
                d[i] = (uint8_t)((val * 127.0f) + 128);
            }
            else
            {
                int32_t val = src[i];
                val = val > 32767 ? 32767 : (val < -32768 ? -32768 : val);
                d[i] = (uint8_t)((val >> (bitsPerSample <= 16 ? 0 : (bitsPerSample - 16))) + 128);
            }
        }
        break;
    }
    case 2:
    { // 16-bit
        int16_t *d = (int16_t *)dst;
        for (uint32_t i = 0; i < samcnt; i++)
        {
            if (is_float)
            {
                float val = *(float *)(&src[i]);
                val = val > 1.0f ? 1.0f : (val < -1.0f ? -1.0f : val);
                d[i] = (int16_t)(val * 32767.0f);
            }
            else
            {
                int32_t val = src[i];
                val = val > 32767 ? 32767 : (val < -32768 ? -32768 : val);
                d[i] = (int16_t)(bitsPerSample <= 16 ? val : (val >> (bitsPerSample - 16)));
            }
        }
        break;
    }
    case 3:
    { // 24-bit
        uint8_t *d = (uint8_t *)dst;
        for (uint32_t i = 0; i < samcnt; i++)
        {
            int32_t val;
            if (is_float)
            {
                float fval = *(float *)(&src[i]);
                fval = fval > 1.0f ? 1.0f : (fval < -1.0f ? -1.0f : fval);
                val = (int32_t)(fval * 8388607.0f);
            }
            else
            {
                val = src[i];
                val = val > 8388607 ? 8388607 : (val < -8388608 ? -8388608 : val);
            }
            d[i * 3] = (uint8_t)(val);
            d[i * 3 + 1] = (uint8_t)(val >> 8);
            d[i * 3 + 2] = (uint8_t)(val >> 16);
        }
        break;
    }
    case 4:
    { // 32-bit
        int32_t *d = (int32_t *)dst;
        for (uint32_t i = 0; i < samcnt; i++)
        {
            if (is_float)
            {
                float val = *(float *)(&src[i]);
                val = val > 1.0f ? 1.0f : (val < -1.0f ? -1.0f : val);
                d[i] = (int32_t)(val * 2147483647.0f);
            }
            else
            {
                d[i] = src[i];
            }
        }
        break;
    }
    }
}

DecoderResult decode_wavpack_to_wav(
    const char *inputPath,
    const char *outputPath,
    int max_samples,
    int force_bps,
    ProgressCallback progress_callback,
    void *context)
{
    // Validate inputs
    if (!inputPath || !outputPath)
    {
        return make_error(ERR_FILE_OPEN_FAILED, "Invalid input or output path");
    }
    if (max_samples < -1)
    {
        return make_error(ERR_INVALID_SAMPLES, "Invalid max samples");
    }
    if (force_bps && force_bps != 8 && force_bps != 16 && force_bps != 24 && force_bps != 32)
    {
        return make_error(ERR_INVALID_BPS, "Invalid bits per sample (must be 8, 16, 24, or 32)");
    }

    // Buffer for WavPack error messages
    char error[80];

    // Local variable to ensure each method call has its own isolated file handle.
    FILE *wvFile = NULL;
    FILE *wavFile = NULL;

    // Opens the .wv file in binary read mode ("rb").
    wvFile = fopen(inputPath, "rb");

    /* If it fails (e.g., file doesn’t exist), rejects the promise with an error
    message and exits. */
    if (!wvFile)
    {
        return make_error(ERR_FILE_OPEN_FAILED, "Cannot open input file");
    }

    // Set static file pointer for read_bytes callback
    currentWvFile = wvFile;

    /* Creates a WavPack context using the read_bytes callback. If it fails (e.g.,
    invalid WavPack file), rejects with the error message from error and closes
    the file. */
    WavpackContext *wpc = WavpackOpenFileInput(read_bytes, error);
    if (!wpc)
    {
        fclose(wvFile);
        currentWvFile = NULL;
        return make_error(ERR_WAVPACK_OPEN_FAILED, error);
    }

    //////////////// Extracts audio properties from the WavPack file ///////////////
    int numChannels = WavpackGetNumChannels(wpc);     // Number of channels (e.g., 1 or 2).
    int sampleRate = WavpackGetSampleRate(wpc);       // Samples per second (e.g., 44100 Hz).
    int bitsPerSample = WavpackGetBitsPerSample(wpc); // Original bit depth (e.g., 16, 24).
    int numSamples = WavpackGetNumSamples(wpc);       // Total number of samples.
    int flags = WavpackGetMode(wpc);                  // Check for float format
    int isFloatFormat = (flags & MODE_FLOAT) != 0;
    ////////////////////////////////////////////////////////////////////////////////

    //////////// Sanity check for buffer size before memory allocation /////////////
    // Validate channel count
    if (numChannels <= 0 || numChannels > 100)
    {
        fclose(wvFile);
        currentWvFile = NULL;
        return make_error(ERR_INVALID_CHANNELS, "Invalid number of channels");
    }

    // Validate sample count
    if (numSamples <= 0)
    {
        fclose(wvFile);
        currentWvFile = NULL;
        return make_error(ERR_INVALID_SAMPLES, "Invalid number of samples");
    }

    // Calculate samples to decode
    uint32_t samples_to_decode = (max_samples == -1) ? numSamples : (max_samples > numSamples ? numSamples : max_samples);

    // Opens the .wav file in binary write mode ("wb")
    wavFile = fopen(outputPath, "wb");
    if (!wavFile)
    {
        // If it fails (e.g., permissions issue), make error and close the input file.
        fclose(wvFile);
        currentWvFile = NULL;
        return make_error(ERR_FILE_OPEN_FAILED, "Cannot open output file");
    }
    ////////////////////////////////////////////////////////////////////////////////

    /////////////////////////////// Write WAV Header ///////////////////////////////
    uint32_t bytesPerSample = force_bps ? (force_bps / 8) : 2; // Default to 16-bit if not forced

    // Avoid overflow: Checks if the total size of audio data exceeds 32-bit limit.
    if ((uint64_t)samples_to_decode * numChannels * bytesPerSample > UINT32_MAX)
    {
        fclose(wavFile);
        fclose(wvFile);
        currentWvFile = NULL;
        return make_error(ERR_DATA_OVERFLOW, "Audio data size exceeds 32-bit limit");
    }

    uint32_t dataSize = samples_to_decode * numChannels * bytesPerSample; // Total audio data size in bytes.

    // Create and write WAV header
    WavHeader header = {
        /* "RIFF" identifier: Marks the start of a RIFF file. Every WAV file begins
        with this */
        .riff = {'R', 'I', 'F', 'F'}, // "RIFF"

        /* A 32-bit unsigned integer representing the total file size minus 8 bytes
        (the size of riff and fileSize fields). */
        .fileSize = dataSize + 36,

        /* The RIFF format identifier, a 4-byte string: Specifies that this RIFF
        file is a WAV audio file. */
        .wave = {'W', 'A', 'V', 'E'}, // "WAVE"

        /* The format chunk identifier, a 4-byte string: Marks the start of the format
        chunk, which describes the audio properties. The space is part of the standard. */
        .fmt = {'f', 'm', 't', ' '}, // "fmt "

        /* A 32-bit unsigned integer indicating the size of the format chunk data
        (excluding fmt and fmtSize): Specifies how many bytes follow in the fmt chunk.
        16 bytes (for PCM, this is fixed: audioFormat [2] + numChannels [2] + sampleRate [4] + byteRate [4] + blockAlign [2] + bitsPerSample [2]). */
        .fmtSize = 16, // 16 for PCM

        /* A 16-bit unsigned integer specifying the audio format: 1 (means uncompressed
        PCM). This tells the player the data is raw PCM, not compressed (e.g., MP3
        would be a different value). */
        .audioFormat = 1,

        /* A 16-bit unsigned integer for the number of audio channels. Cast from int
        numChannels (from WavpackGetNumChannels), typically 1 (mono) or 2 (stereo). */
        .numChannels = (uint16_t)numChannels,

        /* A 32-bit unsigned integer for the sample rate in Hz. Cast from int sampleRate
        (from WavpackGetSampleRate), e.g., 44100 Hz. Defines how many samples per
        second the audio contains.*/
        .sampleRate = (uint32_t)sampleRate,

        /* A 32-bit unsigned integer for the average bytes per second.
        sampleRate * numChannels * bytesPerSample (e.g., 44100 * 2 * 2 = 176400 bytes/sec for stereo 16-bit at 44.1 kHz).
        bytesPerSample is fixed at 2 (16 bits / 8 = 2 bytes). Helps players determine
        playback speed and buffer requirements.*/
        .byteRate = (uint32_t)(sampleRate * numChannels * bytesPerSample),

        /* A 16-bit unsigned integer for the byte size of one sample frame.
        numChannels * bytesPerSample (e.g., 2 * 2 = 4 bytes for stereo 16-bit).
        Specifies the alignment of sample data (how many bytes per multi-channel sample).*/
        .blockAlign = (uint16_t)(numChannels * bytesPerSample),

        /* A 16-bit unsigned integer for the bits per sample. Defines the resolution
        of each sample (8-bit, 16-bit, 24-bit, or 32-bit).*/
        .bitsPerSample = (uint16_t)(bytesPerSample * 8),

        /* The data chunk identifier, a 4-byte string. Marks the start of the audio
        data chunk.*/
        .data = {'d', 'a', 't', 'a'},

        /* A 32-bit unsigned integer for the size of the audio data in bytes.
        dataSize (from numSamples * numChannels * bytesPerSample).
        Tells the player how many bytes of raw audio data follow. */
        .dataSize = dataSize};

    // This ensures the header is written successfully before proceeding to decode and write audio data.
    if (fwrite(&header, sizeof(WavHeader), 1, wavFile) != 1)
    {
        fclose(wavFile);
        fclose(wvFile);
        currentWvFile = NULL;
        return make_error(ERR_WRITE_FAILED, "Failed to write WAV header");
    }
    ////////////////////////////////////////////////////////////////////////////////

    // Allocate buffers for decoding
    const int BUFFER_SIZE = 4096;

    // For 32-bit samples from WavPack.
    int32_t *buffer = (int32_t *)malloc(BUFFER_SIZE * numChannels * sizeof(int32_t));

    // For output samples (size depends on bits per sample).
    void *samples = malloc(BUFFER_SIZE * numChannels * bytesPerSample);

    // If allocation fails, rejects and cleans up.
    if (!buffer || !samples)
    {
        if (buffer)
            free(buffer);
        if (samples)
            free(samples);
        fclose(wavFile);
        fclose(wvFile);
        currentWvFile = NULL;
        return make_error(ERR_MEMORY_ALLOCATION_FAILED, "Memory allocation failed");
    }

    ///////////////////////////////// Decoding Loop ////////////////////////////////
    int totalSamplesRead = 0;

    /* Loop:
    - Calculates how many samples to read per iteration (up to BUFFER_SIZE).
    - WavpackUnpackSamples: Fills buffer with decoded samples.
    - Converts samples to the desired bit depth:
        - Float: Scales [-1.0, 1.0] to appropriate range for output bits per sample.
        - Integer: Direct cast or shifts as needed.
    - Writes converted samples to the .wav file.
    - Exits if no more samples are read (samplesRead <= 0). */
    while (totalSamplesRead < samples_to_decode)
    {
        int samplesToRead = (samples_to_decode - totalSamplesRead < BUFFER_SIZE) ? samples_to_decode - totalSamplesRead : BUFFER_SIZE;

        int samplesRead = WavpackUnpackSamples(wpc, buffer, samplesToRead);

        if (samplesRead < 0)
        {
            free(buffer);
            free(samples);
            fclose(wavFile);
            fclose(wvFile);
            currentWvFile = NULL;
            return make_error(ERR_DECODE_FAILED, "Failed to unpack samples");
        }

        /* Handle Incomplete Decoding: Rejects on negative return (error), logs a
        warning for partial reads, and continues cleanly on zero. */
        if (samplesRead == 0)
            break;

        // Convert samples to desired bit depth
        format_samples(bytesPerSample, samples, buffer, samplesRead * numChannels, isFloatFormat, bitsPerSample);

        // This catches write failures (e.g., disk full) during the sample writing process, allowing cleanup and error reporting.
        if (fwrite(samples, bytesPerSample, samplesRead * numChannels, wavFile) != samplesRead * numChannels)
        {
            free(buffer);
            free(samples);
            fclose(wavFile);
            fclose(wvFile);
            currentWvFile = NULL;
            return make_error(ERR_WRITE_FAILED, "Failed to write audio samples");
        }

        totalSamplesRead += samplesRead;

        if (progress_callback)
        {
            progress_callback((float)totalSamplesRead / samples_to_decode, context);
        }
    }
    ////////////////////////////////////////////////////////////////////////////////

    // Clean up: closes files & frees buffers.
    free(buffer);
    free(samples);
    fclose(wavFile);
    fclose(wvFile);
    currentWvFile = NULL;

    // Check for errors
    if (WavpackGetNumErrors(wpc) > 0)
    {
        char err_msg[80];
        snprintf(err_msg, sizeof(err_msg), "Decoding failed with %d CRC errors", WavpackGetNumErrors(wpc));
        return make_error(ERR_DECODE_FAILED, err_msg);
    }

    // Check if all requested samples were decoded
    if (totalSamplesRead == samples_to_decode)
    {
        return make_success();
    }
    else
    {
        return make_error(ERR_DECODE_FAILED, "Failed to decode all requested samples");
    }
}