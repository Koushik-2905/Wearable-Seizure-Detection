import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

/** In-memory relay: Flutter POSTs vitals → dashboard polls (no Firebase required). */
function neuroguardRelay() {
  let vitals = null;
  const events = [];

  const cors = (res) => {
    res.setHeader("Access-Control-Allow-Origin", "*");
    res.setHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.setHeader("Access-Control-Allow-Headers", "Content-Type");
  };

  const readBody = (req) =>
    new Promise((resolve, reject) => {
      let body = "";
      req.on("data", (c) => (body += c));
      req.on("end", () => {
        try {
          resolve(body ? JSON.parse(body) : {});
        } catch (e) {
          reject(e);
        }
      });
    });

  return {
    name: "neuroguard-relay",
    configureServer(server) {
      server.middlewares.use(async (req, res, next) => {
        const url = req.url?.split("?")[0];
        if (!url?.startsWith("/api/")) return next();

        if (req.method === "OPTIONS") {
          cors(res);
          res.statusCode = 204;
          res.end();
          return;
        }

        try {
          if (url === "/api/vitals" && req.method === "GET") {
            cors(res);
            res.setHeader("Content-Type", "application/json");
            res.end(JSON.stringify(vitals || {}));
            return;
          }

          if (url === "/api/vitals" && req.method === "POST") {
            const data = await readBody(req);
            vitals = { ...data, ts: Date.now() };
            cors(res);
            res.setHeader("Content-Type", "application/json");
            res.end("{}");
            return;
          }

          if (url === "/api/events" && req.method === "GET") {
            cors(res);
            res.setHeader("Content-Type", "application/json");
            res.end(JSON.stringify(events.slice(-50).reverse()));
            return;
          }

          if (url === "/api/events" && req.method === "POST") {
            const data = await readBody(req);
            events.push({
              id: `e${Date.now()}`,
              ...data,
              ts: Date.now(),
            });
            cors(res);
            res.setHeader("Content-Type", "application/json");
            res.end("{}");
            return;
          }
        } catch {
          res.statusCode = 400;
          res.end("{}");
          return;
        }

        next();
      });
    },
  };
}

export default defineConfig({
  plugins: [react(), neuroguardRelay()],
  server: {
    host: true,
    port: 5174,
  },
});
