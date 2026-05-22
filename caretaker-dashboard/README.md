# NeuroGuard Caretaker Dashboard

Simple React dashboard for caregivers: live vitals, GPS, seizure alerts.

## Run locally

```bash
cd caretaker-dashboard
npm install
npm run dev
```

Open http://localhost:5174

## Demo vs live data

- **Now:** mock patient data updates every 3 seconds
- **Later:** point `usePatientData.js` at Firebase RTDB path `devices/neuroguard-001/events` (same shape as ESP32/app alerts)

## Deploy

```bash
npm run build
```

Deploy `dist/` to Vercel/Netlify as a second site, or merge into main NeuroGuard monorepo.
