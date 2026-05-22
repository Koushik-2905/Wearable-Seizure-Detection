import { useEffect, useState } from "react";
import { INITIAL_VITALS, PATIENT, seedAlerts, randomVitals } from "../data/mockPatient.js";

/**
 * Demo mode: simulates live vitals every 3s.
 * Wire to Firebase RTDB: devices/{deviceId}/events when ready.
 */
export function usePatientData() {
  const [vitals, setVitals] = useState(INITIAL_VITALS);
  const [alerts, setAlerts] = useState(seedAlerts);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const t = setTimeout(() => setLoading(false), 600);
    return () => clearTimeout(t);
  }, []);

  useEffect(() => {
    const interval = setInterval(() => {
      setVitals((v) => randomVitals(v));
    }, 3000);
    return () => clearInterval(interval);
  }, []);

  const acknowledgeAlert = (id) => {
    setAlerts((list) =>
      list.map((a) => (a.id === id ? { ...a, acknowledged: true } : a))
    );
  };

  return { patient: PATIENT, vitals, alerts, loading, acknowledgeAlert };
}
