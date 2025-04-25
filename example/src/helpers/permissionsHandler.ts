import { Platform, Alert } from 'react-native';
import {
  request,
  requestMultiple,
  PERMISSIONS,
  RESULTS,
} from 'react-native-permissions';

// Function to handle Android permissions based on API level
export const requestAndroidPermissions = async (): Promise<boolean> => {
  try {
    const sdkVersion = parseInt(Platform.Version.toString(), 10);

    // For Android 13+ (API level 33+)
    if (sdkVersion >= 33) {
      const permissions = [PERMISSIONS.ANDROID.READ_MEDIA_AUDIO];

      const results = await requestMultiple(permissions);

      if (results[PERMISSIONS.ANDROID.READ_MEDIA_AUDIO] === RESULTS.GRANTED) {
        console.log('Audio permission granted');
        return true;
      } else {
        handlePermissionDenied();
        return false;
      }
    }
    // For Android 12+ (API level 31-32)
    else if (sdkVersion >= 31) {
      const result = await request(PERMISSIONS.ANDROID.READ_EXTERNAL_STORAGE);

      if (result === RESULTS.GRANTED) {
        console.log('Storage permission granted');
        return true;
      } else {
        handlePermissionDenied();
        return false;
      }
    }
    // For older Android versions
    else {
      Alert.alert(
        'Unsupported Android Version',
        'This app is designed for Android 12 and newer.'
      );
      return false;
    }
  } catch (err) {
    console.warn('Permission error:', err);
    return false;
  }
};

// Function to check if we need to request permissions
export const checkPermissions = async (): Promise<boolean> => {
  if (Platform.OS === 'ios') {
    // iOS doesn't need explicit permissions for document picker
    return true;
  } else if (Platform.OS === 'android') {
    return await requestAndroidPermissions();
  }
  return false;
};

// Handle permission denied scenarios
const handlePermissionDenied = () => {
  console.log('Permission denied');
  Alert.alert(
    'Permission Required',
    'This app needs access to your storage to function properly. Please grant the permission in app settings.',
    [
      { text: 'Cancel', style: 'cancel' },
      { text: 'Open Settings', onPress: openAppSettings },
    ]
  );
};

// Open app settings
const openAppSettings = () => {
  console.log('Should open app settings');
  // For actual implementation with v5.3.0:
  // import { openSettings } from 'react-native-permissions';
  // openSettings().catch(() => console.log('Cannot open settings'));
};
