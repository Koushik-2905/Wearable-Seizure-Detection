export default function ConfidenceBar({ value }) {
  const pct = Math.round(value * 100);
  const level = pct >= 80 ? "high" : pct >= 40 ? "mid" : "low";

  return (
    <div className="confidence-block">
      <div className="confidence-head">
        <span>Seizure confidence</span>
        <span className={`conf-pct conf-${level}`}>{pct}%</span>
      </div>
      <div className="confidence-track">
        <div
          className={`confidence-fill conf-${level}`}
          style={{ width: `${Math.min(100, pct)}%` }}
        />
      </div>
      <p className="confidence-note">
        {level === "high"
          ? "Elevated — check patient immediately"
          : level === "mid"
            ? "Moderate activity — monitor closely"
            : "Normal range"}
      </p>
    </div>
  );
}
