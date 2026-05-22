import Header from "./components/Header.jsx";
import StatCard from "./components/StatCard.jsx";
import ConfidenceBar from "./components/ConfidenceBar.jsx";
import LocationCard from "./components/LocationCard.jsx";
import AlertList from "./components/AlertList.jsx";
import ActiveAlertBanner from "./components/ActiveAlertBanner.jsx";
import { usePatientData } from "./hooks/usePatientData.js";
import { useAlertNotifications } from "./hooks/useAlertNotifications.js";
import "./App.css";

function stateLabel(state) {
  const map = {
    monitoring: "Monitoring",
    detected: "Possible seizure",
    alert_sent: "Alert sent",
    cancelled: "Cancelled",
  };
  return map[state] ?? state;
}

export default function App() {
  const { patient, vitals, alerts, loading, live, mode, error, acknowledgeAlert } =
    usePatientData();
  const { requestPermission } = useAlertNotifications(alerts, vitals);

  const latestActive = alerts.find((a) => !a.acknowledged) ?? alerts[0];

  if (loading) {
    return <div className="loading">Loading patient data…</div>;
  }

  const updated = vitals.lastUpdate.toLocaleTimeString();

  return (
    <div className="app">
      <Header patient={patient} connected={vitals.bleConnected} />

      <main className="main">
        <p className={`demo-banner ${live ? "demo-banner--live" : ""}`}>
          {live
            ? mode === "firebase"
              ? `Live Firebase · devices/${patient.id}/vitals`
              : "Live relay · phone app → this PC (port 5174) · keep Flutter monitor open on same Wi‑Fi"
            : error ||
              "Waiting for data — run npm.cmd run dev here, connect phone to ESP32 in Flutter Live Monitor (same Wi‑Fi)."}
        </p>

        <ActiveAlertBanner
          alert={latestActive}
          vitalsState={vitals.state}
          onAcknowledge={acknowledgeAlert}
          onEnableNotify={requestPermission}
        />

        <div className="grid-stats">
          <StatCard label="Heart rate" value={vitals.heartRate} unit="BPM" hint="From MAX30102" />
          <StatCard
            label="Status"
            value={stateLabel(vitals.state)}
            hint={vitals.bleConnected ? "BLE linked" : "No phone link"}
            accent="blue"
          />
          <StatCard label="Battery" value={vitals.battery} unit="%" hint="Wearable estimate" />
          <StatCard
            label="Device"
            value={patient.id}
            hint={patient.deviceLabel}
            accent="blue"
          />
        </div>

        <section className="card" style={{ marginBottom: "1.25rem" }}>
          <ConfidenceBar value={vitals.confidence} />
          <p className="last-update">
            Last update: {updated}
          </p>
        </section>

        <div className="grid-main">
          <LocationCard
            lat={vitals.lat}
            lng={vitals.lng}
            gpsOk={vitals.gpsOk}
            locSrc={vitals.locSrc}
          />
          <AlertList alerts={alerts} onAcknowledge={acknowledgeAlert} />
        </div>
      </main>

      <footer className="footer">
        NeuroGuard v2.0 · Caretaker view · SMS + BLE + Firebase alerts
      </footer>
    </div>
  );
}
