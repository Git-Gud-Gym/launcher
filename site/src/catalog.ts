import { useEffect, useState } from "react";
import sampleCatalog from "./data/sample-catalog.json";
import { RELEASES_URL } from "./config";

/* ------------------------------------------------------------------ */
/* Types: the contract produced by scripts/build-catalog.mjs (schema 1) */
/* ------------------------------------------------------------------ */

export interface CatalogFile {
  name: string;
  url: string;
  size: number;
  sha256: string;
}

export interface CatalogApp {
  version: string;
  releaseUrl: string;
  download: CatalogFile;
}

export type ModuleStatus = "available" | "planned";

export interface CatalogGame {
  exe: string;
  steamAppId: number;
  testedPatches: string[];
}

export interface CatalogModule {
  id: string;
  name: string;
  status: ModuleStatus;
  tagline: string;
  features: string[];
  version: string | null;
  releaseUrl: string | null;
  publishedAt: string | null;
  game: CatalogGame | null;
  files: CatalogFile[];
}

export interface Catalog {
  schema: 1;
  generatedAt: string;
  repo: string;
  app: CatalogApp | null;
  modules: CatalogModule[];
}

export type CatalogSource = "live" | "sample";

export interface CatalogState {
  catalog: Catalog;
  source: CatalogSource;
  loading: boolean;
}

/* ------------------------------------------------------------------ */
/* Loading                                                             */
/* ------------------------------------------------------------------ */

const SAMPLE = sampleCatalog as Catalog;

function isCatalog(value: unknown): value is Catalog {
  if (!value || typeof value !== "object") return false;
  const v = value as Partial<Catalog>;
  return (
    v.schema === 1 &&
    Array.isArray(v.modules) &&
    v.modules.every(
      (m) =>
        m &&
        typeof m.id === "string" &&
        typeof m.name === "string" &&
        (m.status === "available" || m.status === "planned"),
    )
  );
}

/** Fills in optional fields so components never have to guard against a sparse catalog. */
function normalise(c: Catalog): Catalog {
  return {
    ...c,
    app: c.app ?? null,
    modules: c.modules.map((m) => ({
      ...m,
      tagline: m.tagline ?? "",
      features: Array.isArray(m.features) ? m.features : [],
      version: m.version ?? null,
      releaseUrl: m.releaseUrl ?? null,
      publishedAt: m.publishedAt ?? null,
      game: m.game ?? null,
      files: Array.isArray(m.files) ? m.files : [],
    })),
  };
}

let pending: Promise<{ catalog: Catalog; source: CatalogSource }> | null = null;

export function loadCatalog() {
  if (!pending) {
    pending = fetch(`${import.meta.env.BASE_URL}catalog.json`, { cache: "no-cache" })
      .then((res) => {
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        return res.json();
      })
      .then((json: unknown) => {
        if (!isCatalog(json)) throw new Error("catalog.json does not match schema 1");
        return { catalog: normalise(json), source: "live" as const };
      })
      .catch(() => ({ catalog: normalise(SAMPLE), source: "sample" as const }));
  }
  return pending;
}

/** The catalog, starting from the bundled sample and swapping in the live one once fetched. */
export function useCatalog(): CatalogState {
  const [state, setState] = useState<CatalogState>({
    catalog: normalise(SAMPLE),
    source: "sample",
    loading: true,
  });

  useEffect(() => {
    let alive = true;
    loadCatalog().then((r) => {
      if (alive) setState({ ...r, loading: false });
    });
    return () => {
      alive = false;
    };
  }, []);

  return state;
}

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

/** Where the main "Download for Windows" button should point. */
export function appDownload(catalog: Catalog): { href: string; label: string; direct: boolean } {
  if (catalog.app?.download?.url) {
    return { href: catalog.app.download.url, label: catalog.app.download.name, direct: true };
  }
  if (catalog.app?.releaseUrl) {
    return { href: catalog.app.releaseUrl, label: "Latest release", direct: false };
  }
  return { href: RELEASES_URL, label: "GitHub releases", direct: false };
}

export function moduleReleaseHref(m: CatalogModule): string {
  return m.releaseUrl ?? RELEASES_URL;
}

export function availableModules(c: Catalog) {
  return c.modules.filter((m) => m.status === "available");
}

export function formatBytes(bytes: number): string {
  if (!Number.isFinite(bytes) || bytes <= 0) return "";
  const units = ["B", "KB", "MB", "GB"];
  let i = 0;
  let n = bytes;
  while (n >= 1024 && i < units.length - 1) {
    n /= 1024;
    i++;
  }
  return `${n.toFixed(n < 10 && i > 0 ? 1 : 0)} ${units[i]}`;
}

export function formatDate(iso: string | null): string | null {
  if (!iso) return null;
  const d = new Date(iso);
  if (Number.isNaN(d.getTime())) return null;
  return d.toLocaleDateString("en", { year: "numeric", month: "short", day: "numeric" });
}
