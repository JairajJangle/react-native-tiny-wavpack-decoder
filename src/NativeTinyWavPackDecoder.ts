import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';

export interface Spec extends TurboModule {
  decodeWavPack(
    inputPath: string,
    outputPath: string,
    maxSamples?: number,
    bitsPerSample?: number,
    verbose?: boolean
  ): Promise<string>;

  // Add these to satisfy NativeEventEmitter
  addListener: (eventType: string) => void;
  removeListeners: (count: number) => void;
}

export default TurboModuleRegistry.getEnforcing<Spec>(
  'TinyWavPackDecoderModule'
);
