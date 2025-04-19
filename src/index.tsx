import NativeTinyWavPackDecoder from './NativeTinyWavPackDecoder';

export interface WavPackDecoderOptions {
  maxSamples?: number;
  bitsPerSample?: 8 | 16 | 24 | 32;
  verbose?: boolean;
}

const WavPackDecoder = {
  decode: async (
    inputPath: string,
    outputPath: string,
    options: WavPackDecoderOptions = {}
  ): Promise<string> => {
    const { maxSamples = -1, bitsPerSample = 16, verbose = false } = options;
    if (![8, 16, 24, 32].includes(bitsPerSample)) {
      throw new Error('bitsPerSample must be 8, 16, 24, or 32');
    }
    return NativeTinyWavPackDecoder.decodeWavPack(
      inputPath,
      outputPath,
      maxSamples,
      bitsPerSample,
      verbose
    );
  },
};

export default WavPackDecoder;
