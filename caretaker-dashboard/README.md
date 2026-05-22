# NeuroGuard Caretaker App

Web app for **family, nurses, or caregivers** — not the patient. Patients use the Flutter app in `mobile_app/`.

You get:

- **Seizure alerts** (HR, confidence, time)
- **Open in Google Maps** on every alert and for live location
- **Live vitals** (heart rate, device status, GPS from the patient’s phone)
- **Browser notifications** (enable when prompted)
- **Install on phone** — Chrome → menu → “Install app” (PWA)

Runs at **http://localhost:5174** — works **without Firebase** via a built-in relay.

## Quick start (no Firebase)

1. **PC:** start dashboard (must listen on all interfaces):

   ```powershell
   cd caretaker-dashboard
   npm.cmd install
   npm.cmd run dev
   ```

2. **Phone + PC on the same Wi‑Fi.** Set relay IP (once per network):

   ```powershell
   cd ..
   .\scripts\set-relay-ip.ps1
   ```

3. **Phone:** rebuild/run Flutter app, connect to NeuroGuard, open **Live Monitor** and leave it open.

4. **Browser:** http://localhost:5174 — banner should show **Live relay** when vitals arrive.

## How it works

```
ESP32 ──BLE──► Flutter app ──HTTP POST──► PC :5174/api/vitals ──poll──► Dashboard
```

Optional: add `.env.local` (see `.env.example`) to use **Firebase** instead of the relay.

## Troubleshooting

| Issue | Fix |
|--------|-----|
| `firebase` import error | Run `npm.cmd install` in `caretaker-dashboard`, restart `npm.cmd run dev` |
| Dashboard “Waiting for phone app” | Run `set-relay-ip.ps1`, rebuild Flutter app, same Wi‑Fi |
| Phone cannot reach PC | Windows Firewall: allow Node/Vite on private network for port **5174** |

## Deploy

```powershell
npm.cmd run build
```

For production, use Firebase (`.env.local`) — the Vite relay is dev-only.
