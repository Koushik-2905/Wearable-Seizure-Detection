export default function Header({ patient, connected }) {
  return (
    <header className="header">
      <div className="header-brand">
        <div className="logo">⚕</div>
        <div>
          <h1>NeuroGuard</h1>
          <p className="subtitle">Caretaker dashboard</p>
        </div>
      </div>
      <div className="header-meta">
        <span className={`pill ${connected ? "pill-ok" : "pill-warn"}`}>
          {connected ? "● Live" : "○ Offline"}
        </span>
        <span className="patient-tag">{patient.name}</span>
      </div>
    </header>
  );
}
