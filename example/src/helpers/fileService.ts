import * as RNFS from '@dr.pogodin/react-native-fs';
import { Platform } from 'react-native';

import * as DocumentPicker from '@react-native-documents/picker';
import type { DocumentPickerResponse } from '@react-native-documents/picker';

import Share from 'react-native-share';

import { checkPermissions } from './permissionsHandler';

export interface FileInfo {
  uri: string;
  name: string;
  path: string;
}

class FileService {
  // Pick a WavPack file from device storage
  async pickWavPackFile(): Promise<FileInfo | null> {
    try {
      // Check permissions first
      const hasPermission = await checkPermissions();
      if (!hasPermission) {
        throw new Error('Storage permission not granted');
      }

      // Pick file - updated for @react-native-documents/picker
      const result = await DocumentPicker.pick({
        type: [DocumentPicker.types.allFiles],
        allowMultiSelection: false,
      });

      if (!result || result.length === 0) return null;

      const file: DocumentPickerResponse = result[0];
      const fileName = file.name || 'unknown.wv';

      // Check if the file is a WavPack file
      if (!fileName.toLowerCase().endsWith('.wv')) {
        throw new Error('Please select a WavPack (.wv) file');
      }

      // Copy the file to app's documents directory for reliable access
      const destinationPath = `${RNFS.DocumentDirectoryPath}/${fileName}`;
      await this.copyFileToLocalStorage(file, destinationPath);

      return {
        uri: file.uri,
        name: fileName,
        path: destinationPath,
      };
    } catch (err: any) {
      // In @react-native-documents/picker, cancellation is detected by error name
      if (
        err.name === 'DocumentPickerCanceledError' ||
        err.code === 'DOCUMENT_PICKER_CANCELED'
      ) {
        console.log('User cancelled file picker');
      } else {
        console.error('Error picking file:', err);
        throw err;
      }
      return null;
    }
  }

  // Copy file from picked location to app's local storage
  private async copyFileToLocalStorage(
    file: DocumentPickerResponse,
    destinationPath: string
  ): Promise<void> {
    try {
      const fileExists = await RNFS.exists(destinationPath);

      if (fileExists) {
        await RNFS.unlink(destinationPath); // Delete the existing file
      }

      // Use the file URI directly
      await RNFS.copyFile(file.uri, destinationPath);
      console.log('File copied to app storage successfully');
    } catch (err) {
      console.error('Error copying file to local storage:', err);
      throw new Error('Failed to copy file to app storage');
    }
  }

  // Save the decoded WAV file to user's desired location
  async saveWavFile(
    sourcePath: string,
    defaultFileName: string = 'output.wav'
  ): Promise<boolean> {
    try {
      // Prepare the share options
      const options = {
        title: 'Save WAV File',
        url: Platform.OS === 'android' ? `file://${sourcePath}` : sourcePath,
        type: 'audio/wav',
        filename: defaultFileName,
        saveToFiles: true, // TODO: Should be false for iOS simulator
      };

      // Show share sheet
      const shareResponse = await Share.open(options);

      console.log('Share response:', shareResponse);

      // If we reach here, consider it a success
      return true;
    } catch (err: any) {
      console.error('Error saving WAV file:', err);
      if (
        err.message &&
        (err.message.includes('User did not share') ||
          err.message.includes('canceled') ||
          err.message.includes('cancelled'))
      ) {
        console.log('User cancelled file saving');
        return false;
      }
      throw err;
    }
  }

  // Delete temporary files
  async cleanupTempFiles(filePaths: string[]): Promise<void> {
    try {
      for (const path of filePaths) {
        if (await RNFS.exists(path)) {
          await RNFS.unlink(path);
        }
      }
      console.log('Temporary files cleaned up');
    } catch (err) {
      console.error('Error cleaning up temp files:', err);
    }
  }
}

export default new FileService();
