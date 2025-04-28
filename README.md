# react-native-tiny-wavpack-decoder

A lightweight React Native Turbo Module for decoding WavPack audio files to WAV format on iOS and Android. Built with the New Architecture for optimal performance, this module supports progress updates during decoding and is designed for seamless integration into React Native apps.

[![npm version](https://img.shields.io/npm/v/react-native-tiny-wavpack-decoder)](https://badge.fury.io/js/react-native-tiny-wavpack-decoder) [![License](https://img.shields.io/github/license/JairajJangle/react-native-tiny-wavpack-decoder)](https://github.com/JairajJangle/react-native-tiny-wavpack-decoder/blob/main/LICENSE) [![Workflow Status](https://github.com/JairajJangle/react-native-tiny-wavpack-decoder/actions/workflows/ci.yml/badge.svg)](https://github.com/JairajJangle/react-native-tiny-wavpack-decoder/actions/workflows/ci.yml) ![Android](https://img.shields.io/badge/-Android-555555?logo=android&logoColor=3DDC84) ![iOS](https://img.shields.io/badge/-iOS-555555?logo=apple&logoColor=white) [![GitHub issues](https://img.shields.io/github/issues/JairajJangle/react-native-tiny-wavpack-decoder)](https://github.com/JairajJangle/react-native-tiny-wavpack-decoder/issues?q=is%3Aopen+is%3Aissue) ![TS](https://img.shields.io/badge/TypeScript-strict_💪-blue) [![Known Vulnerabilities](https://snyk.io/test/github/jairajjangle/react-native-tiny-wavpack-decoder/badge.svg)](https://snyk.io/test/github/jairajjangle/react-native-tiny-wavpack-decoder) ![npm bundle size](https://img.shields.io/bundlephobia/minzip/react-native-tiny-wavpack-decoder)

## Features
- Decode WavPack (.wv) files to WAV format.
- Cross-platform support for iOS (13.0+) and Android (API 21+).
- Progress updates via event emitter for real-time feedback.
- Configurable decoding options (e.g., max samples, bits per sample.
- Thread-safe decoding on iOS with concurrent queue support.

## Requirements
- React Native 0.75 or higher with New Architecture enabled.
- iOS 13.0 or later.
- Android API 21 or later.
- Node.js 16+ for development.

## Installation

1. **Install the package**:
   
   Using yarn:
   
   ```bash
   yarn add react-native-tiny-wavpack-decoder
   ```
   
   using npm:
   
   ```bash
   npm install react-native-tiny-wavpack-decoder
   ```

2. **Link native dependencies**:

   - For iOS, install CocoaPods dependencies:
     ```bash
     cd ios && pod install
     ```
   - For Android, the module is auto-linked via React Native.

3. **Enable New Architecture** (if not already enabled):
   - In `ios/Podfile`, ensure:
     ```ruby
     use_react_native!(
       :path => '../node_modules/react-native',
       :new_architecture => true
     )
     ```
   - In `android/app/build.gradle`, enable Turbo Modules:
     ```gradle
     newArchEnabled=true
     ```

4. **Rebuild the app**:
   ```bash
   npx react-native run-ios
   # or
   npx react-native run-android
   ```

## Usage

### Basic Decoding
Decode a WavPack file to WAV using the `decode` method. The module requires file paths accessible by the app (e.g., in the document directory).

```typescript
import TinyWavPackDecoder from 'react-native-tiny-wavpack-decoder';
import * as RNFS from '@dr.pogodin/react-native-fs';

const decodeWavPack = async () => {
  const inputPath = `${RNFS.DocumentDirectoryPath}/sample.wv`; // Ensure file exists
  const outputPath = `${RNFS.DocumentDirectoryPath}/output.wav`;

  try {
    const result = await TinyWavPackDecoder.decode(inputPath, outputPath, {
      maxSamples: -1, // Decode all samples
      bitsPerSample: 16, // Output bit depth (8, 16, 24, or 32)
    });
    console.log('Decode result:', result); // "Success"
  } catch (error) {
    console.error('Decode error:', error.message);
  }
};
```

### Progress Updates
Subscribe to decoding progress events (0.0 to 1.0) using `addProgressListener`. 

```typescript
import { useEffect } from 'react';
import TinyWavPackDecoder from 'react-native-tiny-wavpack-decoder';

const App = () => {
  useEffect(() => {
    const subscription = TinyWavPackDecoder.addProgressListener((progress) => {
      console.log(`Progress: ${(progress * 100).toFixed(2)}%`);
    });
    return () => {
      subscription.remove();
    };
  }, []);

  // Trigger decodeWavPack() as shown above
};
```

### Options
The `decode` method accepts an options object with the following properties:

| Option          | Type                     | Default | Description                                                                 |
|-----------------|--------------------------|---------|-----------------------------------------------------------------------------|
| `maxSamples`    | `number`                | `-1`    | Maximum number of samples to decode. Use `-1` for all samples.              |
| `bitsPerSample` | `8 | 16 | 24 | 32`   | `16`    | Output bit depth. Must be 8, 16, 24, or 32.                                 |

### Example App
The `example/` directory contains a sample app demonstrating decoding and progress updates. To run it:

```bash
cd example
npm install
npm run ios
# or
npm run android
```

Place a `sample.wv` file in the app’s document directory (e.g., via `RNFS.DocumentDirectoryPath`).

## Notes
- **File Access**: Ensure input and output paths are valid and accessible. Use libraries like `@dr.pogodin/react-native-fs` to manage files.
- **Thread Safety**: iOS uses a concurrent dispatch queue for decoding, but rapid concurrent calls may require additional thread-safety measures (see Limitations).
- **Progress Events**: Ensure listeners are set up before starting decoding.
- **New Architecture**: This module requires Turbo Modules and the New Architecture. Ensure your app is configured accordingly.

## Limitations
- The module uses a static `FILE*` in the C code, which may cause crashes if multiple decoding operations are triggered concurrently. A thread-safe version is planned (see [issue #TBD]).
- Progress updates are tied to the decoding buffer size (4096 samples), which may result in coarse-grained updates for small files.
- Android performance may vary on low-end devices due to JNI overhead.

## Contributing
Contributions are welcome! Please open an issue or pull request on the [GitHub repository](https://github.com/JairajJangle/react-native-tiny-wavpack-decoder) for bugs, features, or improvements.

## License
MIT License. See [LICENSE](LICENSE) for details.

## Acknowledgments
- Built with [WavPack](https://www.wavpack.com/)'s Tiny Decoder source for efficient audio decoding.
- Uses React Native’s New Architecture for modern performance.