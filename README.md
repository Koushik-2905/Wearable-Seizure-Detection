# NeuroGuard

Affordable wearable seizure detection: ESP32 + IMU + heart rate, alerts via SMS/BLE, **GPS from phone over BLE** (no NEO-6M).

## Repository layout

```
Neuroguard/
├── src/                 # Documentation website (Vite + React)
├── firmware/            # ESP32 Arduino firmware v2.0
├── mobile_app/          # Flutter companion app
├── ml/                  # TinyML model (your team — placeholder)
├── hardware/            # BOM + wiring
├── firebase/            # RTDB rules
├── caretaker-dashboard/ # React caregiver UI (local :5174)
└── PROJECT.md           # Full map
```

## Caretaker dashboard

```bash
cd caretaker-dashboard && npm install && npm run dev
```

Opens http://localhost:5174 — live vitals, map link, alert history (demo data until Firebase is wired).

## Documentation site

**Live:** https://neuroguard-ruby.vercel.app

```bash
npm install && npm run dev
```

## Firmware

See [firmware/README.md](firmware/README.md). Open `firmware/neuroguard_main/neuroguard_main.ino` in Arduino IDE.

## Mobile app

See [mobile_app/README.md](mobile_app/README.md). Requires Flutter SDK.

## Machine learning

Detection uses **5-feature threshold voting** today. When your TinyML model is ready, follow [ml/README.md](ml/README.md) and set `ENABLE_ML` in `firmware/neuroguard_main/config.h`.

## Deploy docs site

```bash
npx vercel --prod
```

## License

MIT
