# NeuroGuard Caretaker App (Flutter)

For **family / nurses** — not the patient. Instant seizure alerts with **loud alarm**, **vibration**, and **Google Maps**.

| App | Folder | User |
|-----|--------|------|
| Patient | `mobile_app/` | Wears device, BLE + GPS |
| **Caretaker** | `caretaker_app/` | Gets alerts + alarm + maps |

## Features

- Listens to Firebase `devices/neuroguard-001/events` and `vitals`
- **Full-screen red alarm** + system alarm sound (loop) + vibration
- Local notification (high priority) when a new alert arrives
- **Open Google Maps** from alert screen
- Live vitals on home screen

## Setup

### 1. Create Flutter platform folders (once)

```bash
cd caretaker_app
flutter create . --org com.neuroguard --project-name neuroguard_caretaker
flutter pub get
```

### 2. Firebase (same project as patient app)

1. Firebase Console → add **Android app** with package `com.neuroguard.neuroguard_caretaker`
2. Download `google-services.json` → `caretaker_app/android/app/`
3. Enable Realtime Database (same rules as `firebase/database.rules.json`)
4. Patient app must push vitals/events (BLE connected + Firebase initialized)

### 3. Android permissions

After `flutter create`, confirm `android/app/src/main/AndroidManifest.xml` includes:

```xml
<uses-permission android:name="android.permission.VIBRATE"/>
<uses-permission android:name="android.permission.POST_NOTIFICATIONS"/>
<uses-permission android:name="android.permission.WAKE_LOCK"/>
<uses-permission android:name="android.permission.USE_FULL_SCREEN_INTENT"/>
```

### 4. Run

```bash
flutter run
```

Keep the app installed on the caretaker phone. Allow **notifications** when prompted.

## How alerts reach the caretaker

```
ESP32 seizure → Patient Flutter app → Firebase RTDB
                                      ↓
                         Caretaker app (this) → ALARM + Maps
```

SMS from the wristband still works without this app.

## Alarm behaviour

- Starts on **new** Firebase event or vitals state change to seizure/alert
- Tap **Stop alarm & dismiss** to silence
- Uses device **alarm** ringtone (not media volume on some phones — check alarm volume)

## Optional: push when app is fully closed

RTDB works best while the app is recent/background. For FCM when killed, deploy a Cloud Function (see `firebase/README.md`).

## Also available

Web caretaker UI: `caretaker-dashboard/` (browser, no alarm sound).
