// Generated from android/app/google-services.json (project neuroguard-df097).
import 'package:firebase_core/firebase_core.dart' show FirebaseOptions;
import 'package:flutter/foundation.dart'
    show defaultTargetPlatform, kIsWeb, TargetPlatform;

class DefaultFirebaseOptions {
  static FirebaseOptions get currentPlatform {
    if (kIsWeb) {
      throw UnsupportedError('NeuroGuard mobile app does not support web.');
    }
    switch (defaultTargetPlatform) {
      case TargetPlatform.android:
        return android;
      case TargetPlatform.iOS:
        return ios;
      default:
        throw UnsupportedError(
          'DefaultFirebaseOptions are not configured for $defaultTargetPlatform.',
        );
    }
  }

  static const FirebaseOptions android = FirebaseOptions(
    apiKey: 'AIzaSyBWYq6X9zfuvNiz0RAcz-pooqsIjh9dc7U',
    appId: '1:574358570796:android:8a5323b86432ee93b88e89',
    messagingSenderId: '574358570796',
    projectId: 'neuroguard-df097',
    databaseURL: 'https://neuroguard-df097-default-rtdb.firebaseio.com',
    storageBucket: 'neuroguard-df097.firebasestorage.app',
  );

  /// Add iOS app in Firebase Console and run flutterfire configure to replace.
  static const FirebaseOptions ios = FirebaseOptions(
    apiKey: 'AIzaSyBWYq6X9zfuvNiz0RAcz-pooqsIjh9dc7U',
    appId: '1:574358570796:android:8a5323b86432ee93b88e89',
    messagingSenderId: '574358570796',
    projectId: 'neuroguard-df097',
    databaseURL: 'https://neuroguard-df097-default-rtdb.firebaseio.com',
    storageBucket: 'neuroguard-df097.firebasestorage.app',
  );
}
