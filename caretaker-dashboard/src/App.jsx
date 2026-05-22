import Header from "./components/Header.jsx";
import StatCard from "./components/StatCard.jsx";
import ConfidenceBar from "./components/ConfidenceBar.jsx";
import LocationCard from "./components/LocationCard.jsx";
import AlertList from "./components/AlertList.jsx";
import { usePatientData } from "./hooks/usePatientData.js";
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
  const { patient, vitals, alerts, loading, acknowledgeAlert } = usePatientData();

  if (loading) {
    return <div className="loading">Loading patient data…</div>;
  }

  const updated = vitals.lastUpdate.toLocaleTimeString();

  return (
    <div className="app">
      <Header patient={patient} connected={vitals.bleConnected} />

      <main className="main">
        <p className="demo-banner">
          Demo data — connect Firebase RTDB (devices/{patient.id}/events) for live alerts from the wearable
        </p>

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
          <LocationCard lat={vitals.lat} lng={vitals.lng} gpsOk={vitals.gpsOk} />
          <AlertList alerts={alerts} onAcknowledge={acknowledgeAlert} />
        </div>
      </main>

      <footer className="footer">
        NeuroGuard v2.0 · Caretaker view · SMS + BLE + Firebase alerts
      </footer>
    </div>
  );
}
