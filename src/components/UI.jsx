import { useState } from "react";

export const CodeBlock = ({ code, lang = "cpp" }) => {
  const [copied, setCopied] = useState(false);
  return (
    <div style={{ position: "relative", margin: "1.5rem 0" }}>
      <div style={{ background: "#0a0a0f", border: "1px solid #00ff9d30", borderRadius: "8px", overflow: "hidden" }}>
        <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center",
          padding: "0.5rem 1rem", background: "#00ff9d10", borderBottom: "1px solid #00ff9d20",
          fontFamily: "'Space Mono', monospace", fontSize: "0.7rem", color: "#00ff9d80" }}>
          <span>{lang.toUpperCase()}</span>
          <button onClick={() => { navigator.clipboard?.writeText(code); setCopied(true); setTimeout(() => setCopied(false), 2000); }}
            style={{ background: "none", border: "1px solid #00ff9d40", color: "#00ff9d",
              padding: "2px 10px", borderRadius: "4px", cursor: "pointer",
              fontFamily: "'Space Mono', monospace", fontSize: "0.65rem" }}>
            {copied ? "COPIED!" : "COPY"}
          </button>
        </div>
        <pre style={{ margin: 0, padding: "1.25rem", fontFamily: "'Space Mono', monospace",
          fontSize: "0.72rem", lineHeight: 1.7, color: "#c0ffd0", overflowX: "auto", whiteSpace: "pre" }}>
          {code}
        </pre>
      </div>
    </div>
  );
};

export const Tag = ({ children, color = "#00ff9d" }) => (
  <span style={{ display: "inline-block", background: color + "15", border: `1px solid ${color}40`,
    color, padding: "2px 10px", borderRadius: "20px", fontSize: "0.72rem",
    fontFamily: "'Space Mono', monospace", margin: "3px" }}>{children}</span>
);

export const Table = ({ headers, rows }) => (
  <div style={{ overflowX: "auto", margin: "1rem 0" }}>
    <table style={{ width: "100%", borderCollapse: "collapse" }}>
      <thead>
        <tr>{headers.map((h, i) => (
          <th key={i} style={{ padding: "0.6rem 1rem", textAlign: "left",
            background: "#00ff9d15", border: "1px solid #00ff9d20",
            color: "#00ff9d", fontFamily: "'Space Mono', monospace", fontSize: "0.72rem", fontWeight: 600 }}>{h}</th>
        ))}</tr>
      </thead>
      <tbody>{rows.map((row, i) => (
        <tr key={i} style={{ background: i % 2 === 0 ? "#ffffff05" : "transparent" }}>
          {row.map((cell, j) => (
            <td key={j} style={{ padding: "0.6rem 1rem", border: "1px solid #ffffff10",
              color: "#c8d8c8", fontFamily: "'Space Mono', monospace", fontSize: "0.71rem", lineHeight: 1.5 }}>{cell}</td>
          ))}
        </tr>
      ))}</tbody>
    </table>
  </div>
);

export const Card = ({ title, children, accent = "#00ff9d" }) => (
  <div style={{ background: "#0d1117", border: `1px solid ${accent}25`,
    borderLeft: `3px solid ${accent}`, borderRadius: "8px", padding: "1.25rem 1.5rem", margin: "1rem 0" }}>
    {title && <div style={{ color: accent, fontFamily: "'Space Mono', monospace",
      fontSize: "0.75rem", fontWeight: 700, marginBottom: "0.75rem", letterSpacing: "0.05em" }}>▸ {title}</div>}
    <div>{children}</div>
  </div>
);

export const Callout = ({ emoji, title, children, accent = "#00ff9d" }) => (
  <div style={{ background: accent + "10", border: `1px solid ${accent}30`,
    borderRadius: "8px", padding: "1rem 1.25rem", margin: "1rem 0" }}>
    <div style={{ color: accent, fontFamily: "'Space Mono', monospace",
      fontSize: "0.75rem", fontWeight: 700, marginBottom: "0.5rem" }}>{emoji} {title}</div>
    <div>{children}</div>
  </div>
);

export const P = ({ children }) => <p style={{ color: "#a8c4a8", fontFamily: "'DM Sans', sans-serif", lineHeight: 1.75, fontSize: "0.9rem", margin: "0.6rem 0" }}>{children}</p>;
export const H2 = ({ children }) => <h2 style={{ color: "#e8f5e8", fontFamily: "'Space Mono', monospace", fontSize: "1.1rem", fontWeight: 700, margin: "1.75rem 0 0.75rem", paddingBottom: "0.4rem", borderBottom: "1px solid #00ff9d20", letterSpacing: "0.03em" }}>{children}</h2>;
export const H3 = ({ children }) => <h3 style={{ color: "#00ff9d", fontFamily: "'Space Mono', monospace", fontSize: "0.85rem", fontWeight: 700, margin: "1.25rem 0 0.5rem", letterSpacing: "0.04em" }}>{children}</h3>;
export const Li = ({ children }) => <li style={{ color: "#a8c4a8", fontFamily: "'DM Sans', sans-serif", lineHeight: 1.7, fontSize: "0.88rem", marginBottom: "0.3rem" }}>{children}</li>;
