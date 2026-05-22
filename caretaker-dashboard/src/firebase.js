import { initializeApp } from "firebase/app";
import { getDatabase } from "firebase/database";

const config = {
  apiKey: import.meta.env.VITE_FIREBASE_API_KEY,
  authDomain: import.meta.env.VITE_FIREBASE_AUTH_DOMAIN,
  databaseURL: import.meta.env.VITE_FIREBASE_DATABASE_URL,
  projectId: import.meta.env.VITE_FIREBASE_PROJECT_ID,
  storageBucket: import.meta.env.VITE_FIREBASE_STORAGE_BUCKET,
  messagingSenderId: import.meta.env.VITE_FIREBASE_MESSAGING_SENDER_ID,
  appId: import.meta.env.VITE_FIREBASE_APP_ID,
};

export function isFirebaseConfigured() {
  return Boolean(
    config.apiKey &&
      config.databaseURL &&
      config.projectId
  );
}

let app;
let db;

export function getFirebaseDb() {
  if (!isFirebaseConfigured()) return null;
  if (!app) {
    app = initializeApp(config);
    db = getDatabase(app);
  }
  return db;
}

export const DEVICE_ID =
  import.meta.env.VITE_DEVICE_ID || "neuroguard-001";
