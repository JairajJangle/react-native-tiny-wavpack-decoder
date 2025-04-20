#ifndef TINY_WAVPACK_DECODER_INTERFACE_H
#define TINY_WAVPACK_DECODER_INTERFACE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// WAV header structure
typedef struct {
    char riff[4];           // "RIFF"
    uint32_t fileSize;      // File size - 8
    char wave[4];           // "WAVE"
    char fmt[4];            // "fmt "
    uint32_t fmtSize;       // 16 for PCM
    uint16_t audioFormat;   // 1 for PCM
    uint16_t numChannels;   // 1 or 2
    uint32_t sampleRate;    // e.g., 44100
    uint32_t byteRate;      // sampleRate * numChannels * bitsPerSample / 8
    uint16_t blockAlign;    // numChannels * bitsPerSample / 8
    uint16_t bitsPerSample; // 8, 16, 24, or 32
    char data[4];           // "data"
    uint32_t dataSize;      // Size of the data chunk
} WavHeader;

// Result struct to handle decoding results and errors
typedef struct {
    int success;        // 1 for success, 0 for failure
    char error[80];     // Error message if any
} DecoderResult;

// Callback function pointer type
typedef void (*ProgressCallback)(float progress, void* context);

// Core decoding function
DecoderResult decode_wavpack_to_wav(
    const char* inputPath,
    const char* outputPath,
    int max_samples,
    int force_bps,
    ProgressCallback progress_callback,
    void* context
);

#ifdef __cplusplus
}
#endif

#endif // TINY_WAVPACK_DECODER_INTERFACE_H