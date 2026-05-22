# One-time fix for eloquentarduino/tflm_esp32 on ESP32:
# - Keeps src/signal/micro (kernels + headers for micro_ops.h)
# - Keeps src/signal/src/*.h (headers included by src/*.cc)
# - Excludes src/signal/src/*.cpp (duplicate of src/*.cc — causes linker errors)
# - Removes broken precompiled=full (binaries missing under src/esp32)
$lib = Join-Path $env:USERPROFILE "Documents\Arduino\libraries\tflm_esp32"
if (-not (Test-Path $lib)) {
    Write-Error "tflm_esp32 not found at $lib — clone it first."
    exit 1
}

# Undo legacy patch that moved the whole signal tree out.
$legacy = Join-Path $lib "signal_excluded"
$signal = Join-Path $lib "src\signal"
if (Test-Path $legacy) {
    if (Test-Path $signal) { Remove-Item $signal -Recurse -Force }
    Move-Item $legacy $signal
    Write-Host "Restored signal_excluded -> src/signal"
}

$dupSrc = Join-Path $signal "src"
$excluded = Join-Path $lib "signal_src_excluded"

if (Test-Path $dupSrc) {
    # Already patched: ensure no .cpp remain in signal/src
    Get-ChildItem $dupSrc -Recurse -Include *.cpp -ErrorAction SilentlyContinue | ForEach-Object {
        $rel = $_.FullName.Substring($dupSrc.Length + 1)
        $target = Join-Path $excluded $rel
        $targetDir = Split-Path $target -Parent
        if (-not (Test-Path $targetDir)) { New-Item -ItemType Directory -Force -Path $targetDir | Out-Null }
        Move-Item $_.FullName $target -Force
        Write-Host "Moved duplicate: signal/src/$rel"
    }
} elseif (Test-Path $excluded) {
    New-Item -ItemType Directory -Force -Path $dupSrc | Out-Null
    Get-ChildItem $excluded -Recurse -Include *.h | ForEach-Object {
        $rel = $_.FullName.Substring($excluded.Length + 1)
        $out = Join-Path $dupSrc $rel
        $outDir = Split-Path $out -Parent
        if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Force -Path $outDir | Out-Null }
        Copy-Item $_.FullName $out -Force
    }
    Write-Host "Restored headers to src/signal/src (kept .cpp in signal_src_excluded)"
} else {
    Write-Warning "src/signal/src not found and signal_src_excluded missing — reinstall tflm_esp32."
}

$props = Join-Path $lib "library.properties"
if (Test-Path $props) {
    (Get-Content $props) | Where-Object { $_ -notmatch '^precompiled=' } | Set-Content $props
    Write-Host "Removed precompiled=full from library.properties"
}

if (-not (Test-Path (Join-Path $signal "micro\kernels\irfft.h"))) {
    Write-Error "Missing src/signal/micro/kernels/irfft.h — reinstall tflm_esp32."
    exit 1
}
if (-not (Test-Path (Join-Path $dupSrc "circular_buffer.h"))) {
    Write-Error "Missing src/signal/src/circular_buffer.h — reinstall tflm_esp32."
    exit 1
}
Write-Host "Done. Rebuild neuroguard_main in Arduino IDE."
