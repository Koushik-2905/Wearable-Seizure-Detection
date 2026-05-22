# ML model package (integrated into firmware)

These files are the **source** for the on-device seizure classifier.  
**Active integration:** `firmware/neuroguard_main/` (copied + adapted).

| File | Role |
|------|------|
| `seizure_model.h` | TFLite weights (~6.7 KB) |
| `model_params.h` | Feature normalization (60 means/stds) |
| `seizure_features.h` | Original feature extractor (reference) |
| `seizure_tflite.h` | Original EloquentTinyML wrapper (reference) |

Do not flash `mlmodel/neuroguard_main.ino` for production — it is ML-only demo code.

**Production sketch:** `firmware/neuroguard_main/neuroguard_main.ino`  
(BLE GPS, SMS, threshold + ML blend)
