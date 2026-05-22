export default function StatCard({ label, value, unit, hint, accent = "accent" }) {
  return (
    <article className={`stat-card stat-${accent}`}>
      <span className="stat-label">{label}</span>
      <div className="stat-value-row">
        <span className="stat-value">{value}</span>
        {unit && <span className="stat-unit">{unit}</span>}
      </div>
      {hint && <span className="stat-hint">{hint}</span>}
    </article>
  );
}
