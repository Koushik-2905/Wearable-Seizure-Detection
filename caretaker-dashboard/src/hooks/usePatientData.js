import { useEffect, useState } from "react";
import { DEVICE_ID, isFirebaseConfigured } from "../firebase.js";
import { INITIAL_VITALS, PATIENT, seedAlerts } from "../data/mockPatient.js";

const STATE_LABELS = ["monitoring", "detected", "alert_sent", "cancelled"];

function mapVitals(raw) {
  if (!raw || raw.hr == null) return null;
  const stateIdx = Number(raw.state ?? 0);
  return {
    heartRate: Number(raw.hr ?? 0),
    confidence: Number(raw.conf ?? 0),
    mlScore: Number(raw.ml ?? 0),
    gpsOk: raw.gpsOk === true,
    lat: Number(raw.lat ?? 0),
    lng: Number(raw.lng ?? 0),
    battery: Number(raw.battery ?? INITIAL_VITALS.battery),
    bleConnected: true,
    state: STATE_LABELS[stateIdx] ?? "monitoring",
    locSrc: raw.locSrc || (raw.gpsOk ? "phone" : "none"),
    lastUpdate: raw.ts ? new Date(raw.ts) : new Date(),
  };
}

function mapAlert(raw) {
  return {
    id: raw.id || `e${raw.ts}`,
    type: raw.type || "SEIZURE_ALERT",
    conf: Number(raw.conf ?? 0),
    hr: Number(raw.hr ?? 0),
    lat: Number(raw.lat ?? 0),
    lng: Number(raw.lng ?? 0),
    valid: raw.valid === true,
    ts: raw.ts ? new Date(raw.ts) : new Date(),
    acknowledged: false,
  };
}

async function pollRelay(setVitals, setAlerts, setLoading, setLive, setError) {
  try {
    const [vRes, eRes] = await Promise.all([
      fetch("/api/vitals"),
      fetch("/api/events"),
    ]);
    const v = await vRes.json();
    const events = await eRes.json();
    const mapped = mapVitals(v);
    if (mapped) {
      setVitals(mapped);
      setLive(true);
      setError(null);
    } else {
      setError(
        "Waiting for phone app — open Flutter Live Monitor, connect to NeuroGuard on the same Wi‑Fi as this PC."
      );
    }
    if (Array.isArray(events) && events.length) {
      setAlerts(events.map(mapAlert));
    }
    setLoading(false);
  } catch (err) {
    setError(err.message);
    setLoading(false);
  }
}

export function usePatientData() {
  const [vitals, setVitals] = useState(INITIAL_VITALS);
  const [alerts, setAlerts] = useState(seedAlerts);
  const [loading, setLoading] = useState(true);
  const [live, setLive] = useState(false);
  const [mode, setMode] = useState("relay");
  const [error, setError] = useState(null);

  useEffect(() => {
    let cancelled = false;
    let interval;

    async function startFirebase() {
      try {
        const { onValue, ref, getDatabase } = await import("firebase/database");
        const { getFirebaseDb } = await import("../firebase.js");
        const db = getFirebaseDb();
        if (!db || cancelled) return false;

        setMode("firebase");
        const vitalsRef = ref(db, `devices/${DEVICE_ID}/vitals`);
        const eventsRef = ref(db, `devices/${DEVICE_ID}/events`);

        onValue(vitalsRef, (snap) => {
          const mapped = mapVitals(snap.val());
          if (mapped) {
            setVitals(mapped);
            setLive(true);
            setError(null);
          }
          setLoading(false);
        });

        onValue(eventsRef, (snap) => {
          const list = [];
          snap.forEach((child) => list.push(mapAlert({ id: child.key, ...child.val() })));
          list.sort((a, b) => b.ts - a.ts);
          if (list.length) setAlerts(list);
        });

        return true;
      } catch {
        return false;
      }
    }

    async function start() {
      if (isFirebaseConfigured()) {
        const ok = await startFirebase();
        if (ok && !cancelled) return;
      }

      setMode("relay");
      await pollRelay(setVitals, setAlerts, setLoading, setLive, setError);
      interval = setInterval(
        () => pollRelay(setVitals, setAlerts, () => {}, setLive, setError),
        1000
      );
    }

    start();

    const stale = setInterval(() => {
      setVitals((v) => {
        if (!v.lastUpdate || !v.bleConnected) return v;
        if (Date.now() - v.lastUpdate.getTime() > 15000) {
          return { ...v, bleConnected: false };
        }
        return v;
      });
    }, 3000);

    return () => {
      cancelled = true;
      clearInterval(interval);
      clearInterval(stale);
    };
  }, []);

  const acknowledgeAlert = (id) => {
    setAlerts((list) =>
      list.map((a) => (a.id === id ? { ...a, acknowledged: true } : a))
    );
  };

  return {
    patient: { ...PATIENT, id: DEVICE_ID },
    vitals,
    alerts,
    loading,
    live,
    mode,
    error,
    acknowledgeAlert,
  };
}
