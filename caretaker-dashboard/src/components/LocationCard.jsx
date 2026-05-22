export default function LocationCard({ lat, lng, gpsOk }) {
  const mapsUrl =
    lat != null && lng != null
      ? `https://maps.google.com/?q=${lat},${lng}`
      : null;

  return (
    <section className="card location-card">
      <h2>📍 Last known location</h2>
      {gpsOk && mapsUrl ? (
        <>
          <p className="coords">
            {lat.toFixed(5)}, {lng.toFixed(5)}
          </p>
          <p className="loc-hint">From patient&apos;s phone via BLE</p>
          <a
            href={mapsUrl}
            target="_blank"
            rel="noopener noreferrer"
            className="btn-maps"
          >
            Open in Google Maps →
          </a>
        </>
      ) : (
        <p className="loc-off">GPS unavailable — ask patient to open the app</p>
      )}
    </section>
  );
}
