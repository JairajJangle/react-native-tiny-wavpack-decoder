import NativeTinyWavPackDecoder from './NativeTinyWavPackDecoder';
import {
  NativeModules,
  NativeEventEmitter,
  type EmitterSubscription,
} from 'react-native';

export interface TinyWavPackDecoderOptions {
  maxSamples?: number;
  bitsPerSample?: 8 | 16 | 24 | 32;
}

const { TinyWavPackDecoderModule } = NativeModules;
const emitter = new NativeEventEmitter(TinyWavPackDecoderModule);

const TinyWavPackDecoder = {
  decode: async (
    inputPath: string,
    outputPath: string,
    options: TinyWavPackDecoderOptions = {}
  ): Promise<string> => {
    const { maxSamples = -1, bitsPerSample = 16 } = options;

    if (![8, 16, 24, 32].includes(bitsPerSample)) {
      throw new Error('bitsPerSample must be 8, 16, 24, or 32');
    }

    return NativeTinyWavPackDecoder.decodeWavPack(
      inputPath,
      outputPath,
      maxSamples,
      bitsPerSample
    );
  },

  /**
   * Subscribe to native progress updates
   */
  addProgressListener: (
    callback: (progress: number) => void
  ): EmitterSubscription => {
    return emitter.addListener('onProgressUpdate', (event) => {
      if (typeof event?.progress === 'number') {
        callback(event.progress);
      }
    });
  },

  /**
   * Remove all native listeners for progress updates
   */
  removeAllListeners: (): void => {
    emitter.removeAllListeners('onProgressUpdate');
  },
};

export default TinyWavPackDecoder;
