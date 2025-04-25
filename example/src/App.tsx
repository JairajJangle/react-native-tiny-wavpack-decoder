import { useEffect, useState } from 'react';
import {
  ActivityIndicator,
  Alert,
  Button,
  StyleSheet,
  Text,
  View,
} from 'react-native';
import * as RNFS from '@dr.pogodin/react-native-fs';
import TinyWavPackDecoder from 'react-native-tiny-wavpack-decoder';
import { checkPermissions } from './helpers/permissionsHandler';
import FileService, { type FileInfo } from './helpers/fileService';

export default function App() {
  const [selectedFile, setSelectedFile] = useState<FileInfo | null>(null);
  const [outputFile, setOutputFile] = useState<string | null>(null);
  const [result, setResult] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [progress, setProgress] = useState(0);
  const [isLoading, setIsLoading] = useState(false);
  const [isDecoding, setIsDecoding] = useState(false);

  useEffect(() => {
    // Setup permissions on component mount
    const setup = async () => {
      await checkPermissions();
    };
    setup();

    // Set up progress callback
    const subscription = TinyWavPackDecoder.addProgressListener((p: number) => {
      console.log(`Progress update: ${p}`);
      setProgress(p);
    });

    // Cleanup on unmount
    return () => {
      console.log('Cleaning up progress listener');
      subscription.remove();
    };
  }, []);

  const pickFile = async () => {
    try {
      setIsLoading(true);
      setError(null);

      const file = await FileService.pickWavPackFile();
      if (file) {
        setSelectedFile(file);
        setResult(null);
        setOutputFile(null);
        console.log('Selected file:', file.name);
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Error picking file');
    } finally {
      setIsLoading(false);
    }
  };

  const decode = async () => {
    if (!selectedFile) {
      setError('Please select a WavPack file first');
      return;
    }

    try {
      setIsDecoding(true);
      setError(null);
      setResult(null);

      const outputPath = `${RNFS.DocumentDirectoryPath}/output_${Date.now()}.wav`;

      const decodeResult = await TinyWavPackDecoder.decode(
        selectedFile.path,
        outputPath,
        {
          maxSamples: -1,
          bitsPerSample: 16,
        }
      );

      setOutputFile('Saved');
      setResult(decodeResult);
      setError(null);
    } catch (err) {
      setError(
        err instanceof Error ? err.message : 'Unknown error during decoding'
      );
    } finally {
      setIsDecoding(false);
    }
  };

  const saveDecodedFile = async () => {
    if (!outputFile) {
      setError('No decoded file to save');
      return;
    }

    try {
      setIsLoading(true);
      const outputFileName =
        selectedFile?.name?.replace('.wv', '.wav') || 'output.wav';
      const saved = await FileService.saveWavFile(outputFile, outputFileName);

      if (saved) {
        Alert.alert('Success', 'File saved successfully');
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'Error saving file');
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>WavPack Decoder</Text>

      <Button
        title="Select WavPack File"
        onPress={pickFile}
        disabled={isLoading || isDecoding}
      />

      {selectedFile && (
        <Text style={styles.info}>Selected: {selectedFile.name}</Text>
      )}

      <View style={styles.buttonContainer}>
        <Button
          title="Decode WavPack File"
          onPress={decode}
          disabled={!selectedFile || isDecoding || isLoading}
        />
      </View>

      {isDecoding && (
        <View style={styles.progressContainer}>
          <ActivityIndicator size="small" color="#0000ff" />
          <Text style={styles.progressText}>
            Decoding... {(progress * 100).toFixed(1)}%
          </Text>
        </View>
      )}

      {outputFile && (
        <View style={styles.buttonContainer}>
          <Button
            title="Save Decoded WAV File"
            onPress={saveDecodedFile}
            disabled={isLoading}
          />
        </View>
      )}

      {result && <Text style={styles.result}>Result: {result}</Text>}
      {error && <Text style={styles.error}>Error: {error}</Text>}

      {isLoading && (
        <View style={styles.loadingContainer}>
          <ActivityIndicator size="large" color="#0000ff" />
          <Text>Processing...</Text>
        </View>
      )}
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
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 20,
  },
  buttonContainer: {
    marginTop: 15,
    width: '100%',
    maxWidth: 250,
  },
  info: {
    marginTop: 10,
    color: '#333',
    fontSize: 14,
  },
  progressContainer: {
    flexDirection: 'row',
    alignItems: 'center',
    marginTop: 15,
  },
  progressText: {
    marginLeft: 10,
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
  loadingContainer: {
    position: 'absolute',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: 'rgba(255,255,255,0.7)',
  },
});
