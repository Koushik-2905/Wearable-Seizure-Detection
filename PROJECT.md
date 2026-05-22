# NeuroGuard — Full Project Map

| Path | What it is |
|------|------------|
| **Docs site** | `npm run dev` — Vite React (deployed on Vercel) |
| **firmware/** | ESP32 Arduino sketch v2.0, BLE GPS, 5-feature detection |
| **mobile_app/** | Flutter BLE + GPS + alerts + Firebase |
| **ml/** | TinyML placeholder — **you build model, we integrate** |
| **hardware/** | BOM + wiring |
| **firebase/** | RTDB rules template |

## Quick start order

1. Flash firmware (`firmware/README.md`) — try `SIMULATION_MODE 1` first
2. `flutter create` + `flutter run` in `mobile_app/`
3. Pair phone to ESP32; confirm GPS lines on Serial
4. When ML model is ready → `ml/README.md` + `ENABLE_ML 1`

## Docs

https://neuroguard-ruby.vercel.app
