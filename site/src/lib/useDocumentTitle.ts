import { useEffect } from "react";

const BASE = "Git Gud Gym";
const HOME = "Git Gud Gym · Endless boss practice for souls-likes";

/** Sets document.title; pass null for the home page title. */
export function useDocumentTitle(title: string | null) {
  useEffect(() => {
    document.title = title ? `${title} · ${BASE}` : HOME;
  }, [title]);
}
