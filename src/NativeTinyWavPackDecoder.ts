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
}

export default TurboModuleRegistry.getEnforcing<Spec>(
  'TinyWavPackDecoderModule'
);
