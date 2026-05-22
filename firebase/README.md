# Firebase (NeuroGuard)

## Setup

1. Create project at https://console.firebase.google.com
2. Enable **Realtime Database** and **Cloud Messaging**
3. Run `flutterfire configure` in `mobile_app/`
4. Deploy rules: `firebase deploy --only database` (after installing Firebase CLI)

## Data shape

```
devices/neuroguard-001/events/{pushId}
  type: "SEIZURE_ALERT"
  ts: server timestamp
  conf, hr, lat, lng, valid
```

Adjust rules in `database.rules.json` for production auth.
