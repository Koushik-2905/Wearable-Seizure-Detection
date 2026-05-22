# NeuroGuard Flutter App

Companion app: scan **NeuroGuard** ESP32, send phone GPS over BLE every 10s, receive seizure alerts + live sensor stream.

## First-time setup

Flutter SDK required: https://docs.flutter.dev/get-started/install

```powershell
cd mobile_app
flutter create . --org com.neuroguard --project-name neuroguard_app
flutter pub get
```

This generates `android/` and `ios/` folders. Our `lib/` code is already in place.

### Android permissions

After `flutter create`, merge into `android/app/src/main/AndroidManifest.xml` (inside `<manifest>`):

```xml
<uses-permission android:name="android.permission.BLUETOOTH"/>
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN"/>
<uses-permission android:name="android.permission.BLUETOOTH_SCAN" android:usesPermissionFlags="neverForLocation"/>
<uses-permission android:name="android.permission.BLUETOOTH_CONNECT"/>
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION"/>
<uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION"/>
```

Google Maps: add API key in `AndroidManifest.xml` under `<application>`:

```xml
<meta-data android:name="com.google.android.geo.API_KEY" android:value="YOUR_KEY"/>
```

### iOS (Info.plist)

```xml
<key>NSBluetoothAlwaysUsageDescription</key>
<string>NeuroGuard connects to the wearable device</string>
<key>NSLocationWhenInUseUsageDescription</key>
<string>GPS location is sent to the wearable for emergency alerts</string>
```

### Firebase (optional)

1. Create Firebase project
2. Add Android app → download `google-services.json` → `android/app/`
3. Add iOS app → `GoogleService-Info.plist` → `ios/Runner/`
4. Follow FlutterFire CLI: `flutterfire configure`

Without Firebase, the app still works over BLE; cloud logging is skipped.

## Run

```bash
flutter run
```

## Features (matches docs site)

- BLE scan + connect to `NeuroGuard`
- GPS write every 10s → ESP32
- Alert notify → full-screen dialog + Maps link
- Live confidence chart + map pin
- Firebase event log when configured
