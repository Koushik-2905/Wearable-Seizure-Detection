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

Without Firebase, vitals go to the **caretaker dashboard relay** (`relay_config.dart`). Run from repo root:

```powershell
.\scripts\set-relay-ip.ps1
cd caretaker-dashboard
npm.cmd install
npm.cmd run dev
```

Phone and PC must be on the **same Wi‑Fi**. Rebuild the APK after changing `relay_config.dart`.

### App not updating?

1. **Patient app (Live Monitor)** — must show **Last BLE update: Just now** every ~1s. If not:
   - ESP32 Serial shows `Phone connected`
   - Reconnect from scan screen; keep Live Monitor open
2. **Caretaker dashboard** — patient app forwards data; dashboard does not talk to ESP32 directly
3. **Caretaker Flutter app** — needs `google-services.json` on **both** patient and caretaker apps + Firebase RTDB rules deployed
4. **Relay IP** — wrong IP in `relay_config.dart` is the #1 cause; run `set-relay-ip.ps1` then `flutter build apk`

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
