# Firebase (NeuroGuard)

## Setup

1. Create project at https://console.firebase.google.com
2. Enable **Realtime Database** and **Cloud Messaging**
3. Run `flutterfire configure` in `mobile_app/`
4. Deploy rules: `firebase deploy --only database` (after installing Firebase CLI)

## Data shape

```
devices/neuroguard-001/vitals
  state, hr, conf, ml, gpsOk, lat, lng, ts   # updated ~every 2s from Flutter (BLE)

devices/neuroguard-001/events/{pushId}
  type: "SEIZURE_ALERT"
  ts: server timestamp
  conf, hr, lat, lng, valid
```

Caretaker dashboard (`caretaker-dashboard/`) and **caretaker Flutter app** (`caretaker_app/`) subscribe to both paths when Firebase is configured.

### Optional: FCM when caretaker app is killed

Deploy a Cloud Function that sends to topic `caretaker_neuroguard-001` when a new event is written (caretaker app already subscribes to this topic).

Adjust rules in `database.rules.json` for production auth.
