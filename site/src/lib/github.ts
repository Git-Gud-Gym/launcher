import { useEffect, useState } from "react";
import { API_REPO_URL } from "../config";

export interface RepoStats {
  stars: number;
  forks: number;
  openIssues: number;
}

const CACHE_KEY = "ggg:repo-stats";
const CACHE_MS = 30 * 60 * 1000;
let pending: Promise<RepoStats | null> | null = null;

function readCache(): RepoStats | null {
  try {
    const raw = sessionStorage.getItem(CACHE_KEY);
    if (!raw) return null;
    const { at, data } = JSON.parse(raw) as { at: number; data: RepoStats };
    return Date.now() - at < CACHE_MS ? data : null;
  } catch {
    return null;
  }
}

function fetchStats(): Promise<RepoStats | null> {
  const cached = readCache();
  if (cached) return Promise.resolve(cached);
  if (!pending) {
    pending = fetch(API_REPO_URL, { headers: { Accept: "application/vnd.github+json" } })
      .then((r) => (r.ok ? r.json() : null))
      .then((j: Record<string, unknown> | null) => {
        if (!j || typeof j.stargazers_count !== "number") return null;
        const data: RepoStats = {
          stars: j.stargazers_count,
          forks: typeof j.forks_count === "number" ? j.forks_count : 0,
          openIssues: typeof j.open_issues_count === "number" ? j.open_issues_count : 0,
        };
        try {
          sessionStorage.setItem(CACHE_KEY, JSON.stringify({ at: Date.now(), data }));
        } catch {
          /* storage unavailable: fine */
        }
        return data;
      })
      .catch(() => null);
  }
  return pending;
}

/** Live repo numbers from the GitHub API, or null if unavailable (rate limit, offline...). */
export function useRepoStats(): RepoStats | null {
  const [stats, setStats] = useState<RepoStats | null>(() => readCache());
  useEffect(() => {
    let alive = true;
    fetchStats().then((s) => alive && setStats(s));
    return () => {
      alive = false;
    };
  }, []);
  return stats;
}

export function compactNumber(n: number): string {
  return new Intl.NumberFormat("en", { notation: "compact", maximumFractionDigits: 1 }).format(n);
}

/** Star counts are only shown once they're meaningful; below that the button just says "GitHub". */
export const MIN_STARS_SHOWN = 10;
export function showStars(stats: RepoStats | null): boolean {
  return !!stats && stats.stars >= MIN_STARS_SHOWN;
}
