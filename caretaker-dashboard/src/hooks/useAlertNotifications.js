import { useEffect, useRef } from "react";

function mapsUrl(lat, lng) {
  return `https://maps.google.com/?q=${lat},${lng}`;
}

export function useAlertNotifications(alerts, vitals) {
  const seenIds = useRef(new Set());

  useEffect(() => {
    if (typeof Notification === "undefined") return;

    for (const a of alerts) {
      if (a.acknowledged || seenIds.current.has(a.id)) continue;
      seenIds.current.add(a.id);

      if (Notification.permission !== "granted") continue;

      const body = a.valid
        ? `HR ${a.hr} BPM · ${a.conf}% confidence · Tap to open Maps`
        : `HR ${a.hr} BPM · GPS unavailable`;

      const n = new Notification("NeuroGuard — Seizure detected", {
        body,
        tag: a.id,
        requireInteraction: true,
      });

      n.onclick = () => {
        window.focus();
        if (a.valid) window.open(mapsUrl(a.lat, a.lng), "_blank");
      };
    }
  }, [alerts]);

  useEffect(() => {
    if (typeof Notification === "undefined" || Notification.permission !== "granted") return;
    if (vitals.state !== "detected" && vitals.state !== "alert_sent") return;

    const tag = `state-${vitals.state}`;
    if (seenIds.current.has(tag)) return;
    seenIds.current.add(tag);

    new Notification("NeuroGuard — Possible seizure", {
      body: `Device reports: ${vitals.state}. Heart rate ${vitals.heartRate} BPM.`,
      tag,
      requireInteraction: true,
    });
  }, [vitals.state, vitals.heartRate]);

  const requestPermission = async () => {
    if (typeof Notification === "undefined") return "unsupported";
    const result = await Notification.requestPermission();
    return result;
  };

  return { requestPermission };
}
