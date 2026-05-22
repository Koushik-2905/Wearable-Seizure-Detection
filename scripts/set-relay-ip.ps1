# Updates Flutter relay_config.dart with this PC's Wi-Fi IPv4 (for caretaker dashboard).
$wifi = Get-NetIPAddress -AddressFamily IPv4 |
  Where-Object { $_.InterfaceAlias -match 'Wi-?Fi|WLAN' -and $_.IPAddress -notmatch '^169\.' } |
  Select-Object -First 1
if (-not $wifi) {
  $wifi = Get-NetIPAddress -AddressFamily IPv4 |
    Where-Object { $_.IPAddress -notmatch '^127\.|^169\.|^192\.168\.(222|184)\.' } |
    Select-Object -First 1
}
$ip = $wifi.IPAddress
if (-not $ip) { Write-Error "No LAN IP found. Set relayBaseUrl manually in mobile_app/lib/constants/relay_config.dart"; exit 1 }

$dart = Join-Path $PSScriptRoot "..\mobile_app\lib\constants\relay_config.dart"
$content = @"
/// PC running caretaker dashboard (npm run dev). Phone must be on same Wi-Fi.
/// Regenerate: scripts/set-relay-ip.ps1
const String relayBaseUrl = 'http://${ip}:5174';

const bool relayEnabled = true;
"@
Set-Content -Path $dart -Value $content -NoNewline
Write-Host "relayBaseUrl -> http://${ip}:5174"
