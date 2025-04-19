import { useState } from 'react';
import { Button, StyleSheet, Text, View } from 'react-native';
import * as RNFS from '@dr.pogodin/react-native-fs';
import TinyWavPackDecoder from 'react-native-tiny-wavpack-decoder';

export default function App() {
  const [result, setResult] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);

  const decode = async () => {
    const inputPath = `${RNFS.DocumentDirectoryPath}/sample.wv`;
    const outputPath = `${RNFS.DocumentDirectoryPath}/output.wav`;

    try {
      const decodeResult = await TinyWavPackDecoder.decode(
        inputPath,
        outputPath,
        {
          maxSamples: -1,
          bitsPerSample: 16,
          verbose: true,
        }
      );
      setResult(decodeResult);
      setError(null);
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Unknown error');
      setResult(null);
    }
  };

  return (
    <View style={styles.container}>
      <Button title="Decode WavPack File" onPress={decode} />
      {result && <Text style={styles.result}>Result: {result}</Text>}
      {error && <Text style={styles.error}>Error: {error}</Text>}
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
    padding: 20,
  },
  result: {
    marginTop: 20,
    color: 'green',
    fontSize: 16,
  },
  error: {
    marginTop: 20,
    color: 'red',
    fontSize: 16,
  },
});
