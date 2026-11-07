#!/usr/bin/env node
// Builds catalog.json, the game library the app and the website read.
//
//   node scripts/build-catalog.mjs --out site/public/catalog.json [--repo owner/name]
//
// Sources: src/games/*/module.json (what each module is), catalog/planned.json (games we
// intend to support), and the repo's GitHub releases (what's actually downloadable).
// Every downloadable file gets a SHA-256 that the app verifies before installing.
// Set GH_TOKEN (or GITHUB_TOKEN) to avoid API rate limits.

import { createHash } from 'node:crypto';
import { readFile, readdir, writeFile, mkdir } from 'node:fs/promises';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = resolve(dirname(fileURLToPath(import.meta.url)), '..');
const args = Object.fromEntries(
  process.argv.slice(2).reduce((pairs, arg, i, all) => {
    if (arg.startsWith('--')) pairs.push([arg.slice(2), all[i + 1]]);
    return pairs;
  }, []),
);
const out = resolve(args.out ?? 'catalog.json');
const repo = args.repo ?? process.env.GITHUB_REPOSITORY ?? 'Git-Gud-Gym/git-gud-gym';
const token = process.env.GH_TOKEN ?? process.env.GITHUB_TOKEN;

const api = async (path) => {
  const res = await fetch(`https://api.github.com${path}`, {
    headers: {
      Accept: 'application/vnd.github+json',
      'User-Agent': 'git-gud-gym-catalog',
      ...(token ? { Authorization: `Bearer ${token}` } : {}),
    },
  });
  if (!res.ok) throw new Error(`GitHub API ${path}: HTTP ${res.status}`);
  return res.json();
};

const semver = (v) => v.split('.').map((n) => parseInt(n, 10) || 0);
const newer = (a, b) => {
  const [x, y] = [semver(a), semver(b)];
  for (let i = 0; i < 3; i++) if ((x[i] ?? 0) !== (y[i] ?? 0)) return (x[i] ?? 0) > (y[i] ?? 0);
  return false;
};

// Latest published (non-draft, non-prerelease) release whose tag is `${prefix}-v<semver>` and that
// passes the filter.
const latest = (releases, prefix, accept = () => true) => {
  const re = new RegExp(`^${prefix}-v(\\d+\\.\\d+\\.\\d+)$`);
  let best = null;
  for (const r of releases) {
    const m = re.exec(r.tag_name);
    if (!m || r.draft || r.prerelease || !accept(r)) continue;
    if (!best || newer(m[1], best.version)) best = { version: m[1], release: r };
  }
  return best;
};

const describeFile = async (asset) => {
  let sha256 = asset.digest?.startsWith('sha256:') ? asset.digest.slice(7) : null;
  if (!sha256) {
    const res = await fetch(asset.browser_download_url, { headers: { 'User-Agent': 'git-gud-gym-catalog' } });
    if (!res.ok) throw new Error(`Download ${asset.name}: HTTP ${res.status}`);
    sha256 = createHash('sha256').update(Buffer.from(await res.arrayBuffer())).digest('hex');
  }
  return { name: asset.name, url: asset.browser_download_url, size: asset.size, sha256 };
};

const readJson = async (path) => JSON.parse(await readFile(path, 'utf8'));

async function main() {
  const releases = [];
  for (let page = 1; page <= 5; page++) {
    const batch = await api(`/repos/${repo}/releases?per_page=100&page=${page}`);
    releases.push(...batch);
    if (batch.length < 100) break;
  }

  // The hub app.
  let app = null;
  const appRelease = latest(releases, 'app', (r) => r.assets.some((a) => /^GitGudGym-v.*\.zip$/.test(a.name)));
  if (appRelease) {
    const asset = appRelease.release.assets.find((a) => /^GitGudGym-v.*\.zip$/.test(a.name));
    app = { version: appRelease.version, releaseUrl: appRelease.release.html_url, download: await describeFile(asset) };
  }

  // Game modules from the source tree, matched with their latest release.
  const modules = [];
  const gamesDir = join(root, 'src', 'games');
  for (const id of (await readdir(gamesDir, { withFileTypes: true })).filter((d) => d.isDirectory()).map((d) => d.name)) {
    const manifest = await readJson(join(gamesDir, id, 'module.json'));
    const files = manifest.files ?? [manifest.dll, 'module.json'];
    // Only releases that carry the module's individual files can be installed by the app.
    const published = latest(releases, manifest.id, (r) => files.every((f) => r.assets.some((a) => a.name === f)));
    modules.push({
      id: manifest.id,
      name: manifest.name,
      status: 'available',
      tagline: manifest.tagline ?? '',
      features: manifest.features ?? [],
      version: published?.version ?? null,
      releaseUrl: published?.release.html_url ?? null,
      publishedAt: published?.release.published_at ?? null,
      game: {
        exe: manifest.game.exe,
        steamAppId: manifest.game.steamAppId ?? null,
        testedPatches: (manifest.game.versions ?? []).filter((v) => v.tested).map((v) => v.patch),
      },
      files: published
        ? await Promise.all(files.map((f) => describeFile(published.release.assets.find((a) => a.name === f))))
        : [],
    });
  }

  // Games we plan to support but haven't built yet.
  const planned = await readJson(join(root, 'catalog', 'planned.json')).catch(() => []);
  for (const p of planned) {
    if (modules.some((m) => m.id === p.id)) continue;
    modules.push({
      id: p.id, name: p.name, status: 'planned', tagline: p.tagline ?? '', features: [],
      version: null, releaseUrl: null, publishedAt: null, game: null, files: [],
    });
  }

  const catalog = { schema: 1, generatedAt: new Date().toISOString(), repo, app, modules };
  await mkdir(dirname(out), { recursive: true });
  await writeFile(out, `${JSON.stringify(catalog, null, 2)}\n`);
  console.log(`Wrote ${out}: app ${app?.version ?? '(unreleased)'}, ` +
    modules.map((m) => `${m.id} ${m.version ?? m.status}`).join(', '));
}

main().catch((err) => {
  console.error(err.message);
  process.exit(1);
});
