import { useState } from "react";
import { Link } from "react-router-dom";
import { ArrowRight, Check, Copy, Download, ExternalLink, FileArchive } from "lucide-react";
import {
  formatBytes,
  formatDate,
  moduleReleaseHref,
  useCatalog,
  type CatalogFile,
  type CatalogModule,
} from "../catalog";
import GameArt from "../components/GameArt";
import { AddGameCard } from "../components/GameCard";
import { Glow, Reveal, StatusBadge } from "../components/ui";
import { useDocumentTitle } from "../lib/useDocumentTitle";

function CopyHash({ hash }: { hash: string }) {
  const [copied, setCopied] = useState(false);
  const copy = async () => {
    try {
      await navigator.clipboard.writeText(hash);
      setCopied(true);
      window.setTimeout(() => setCopied(false), 1600);
    } catch {
      /* clipboard blocked: ignore */
    }
  };
  return (
    <button
      type="button"
      onClick={copy}
      className="inline-flex items-center gap-1.5 rounded-md px-1.5 py-0.5 font-mono text-xs text-muted transition-colors hover:bg-ink-600 hover:text-bone"
      title={hash}
      aria-label={copied ? "SHA-256 copied" : `Copy SHA-256 ${hash}`}
    >
      sha256:{hash.slice(0, 12)}…
      {copied ? <Check className="h-3.5 w-3.5 text-moss" aria-hidden /> : <Copy className="h-3.5 w-3.5" aria-hidden />}
    </button>
  );
}

function FileRow({ f }: { f: CatalogFile }) {
  return (
    <li className="flex flex-col gap-2 rounded-xl border border-ink-500 bg-ink/50 p-3 sm:flex-row sm:items-center sm:justify-between">
      <div className="flex min-w-0 items-center gap-3">
        <FileArchive className="h-4 w-4 shrink-0 text-gold" aria-hidden />
        <span className="truncate font-mono text-sm text-bone">{f.name}</span>
        {f.size > 0 && <span className="shrink-0 text-xs text-muted">{formatBytes(f.size)}</span>}
      </div>
      <div className="flex items-center gap-2">
        {f.sha256 && <CopyHash hash={f.sha256} />}
        <a
          href={f.url}
          className="inline-flex items-center gap-1.5 rounded-lg border border-ink-400 px-2.5 py-1 text-xs font-semibold text-bone transition-colors hover:border-gold/60 hover:text-gold-hover"
        >
          <Download className="h-3.5 w-3.5" aria-hidden /> Download
          <span className="sr-only"> {f.name}</span>
        </a>
      </div>
    </li>
  );
}

function AvailableModule({ m }: { m: CatalogModule }) {
  const published = formatDate(m.publishedAt);
  return (
    <Reveal as="article" className="scroll-mt-24">
      <div
        id={m.id}
        className="overflow-hidden rounded-3xl border border-ink-500 bg-ink-700/80 lg:grid lg:grid-cols-[minmax(0,22rem)_minmax(0,1fr)]"
      >
        <GameArt id={m.id} name={m.name} className="aspect-[16/9] lg:aspect-auto lg:min-h-full" />
        <div className="p-6 sm:p-8">
          <div className="flex flex-wrap items-center gap-3">
            <StatusBadge status={m.status} />
            {m.version && <span className="chip font-mono">v{m.version}</span>}
            {published && <span className="text-xs text-muted">Released {published}</span>}
          </div>
          <h2 className="mt-4 font-display text-4xl font-bold uppercase tracking-tight text-bone">{m.name}</h2>
          {m.tagline && <p className="mt-2 text-muted">{m.tagline}</p>}

          {m.features.length > 0 && (
            <ul className="mt-6 grid gap-2.5 sm:grid-cols-2">
              {m.features.map((f) => (
                <li key={f} className="flex gap-2.5 text-sm text-bone/90">
                  <Check className="mt-0.5 h-4 w-4 shrink-0 text-gold" aria-hidden />
                  {f}
                </li>
              ))}
            </ul>
          )}

          {m.game && (
            <dl className="mt-6 grid grid-cols-2 gap-4 rounded-2xl border border-ink-500 bg-ink/40 p-4 text-sm sm:grid-cols-3">
              <div>
                <dt className="text-xs uppercase tracking-wider text-muted">Tested patches</dt>
                <dd className="mt-1 font-mono text-bone">{m.game.testedPatches.join(", ") || "None yet"}</dd>
              </div>
              <div>
                <dt className="text-xs uppercase tracking-wider text-muted">Executable</dt>
                <dd className="mt-1 break-all font-mono text-bone">{m.game.exe}</dd>
              </div>
              <div>
                <dt className="text-xs uppercase tracking-wider text-muted">Steam app ID</dt>
                <dd className="mt-1 font-mono text-bone">{m.game.steamAppId}</dd>
              </div>
            </dl>
          )}

          {m.files.length > 0 && (
            <div className="mt-6">
              <h3 className="font-display text-sm font-semibold uppercase tracking-[0.2em] text-muted">Files</h3>
              <ul className="mt-3 space-y-2">
                {m.files.map((f) => (
                  <FileRow key={f.name} f={f} />
                ))}
              </ul>
            </div>
          )}

          <div className="mt-8 flex flex-col gap-3 sm:flex-row sm:items-center">
            <a href={moduleReleaseHref(m)} className="btn-gold">
              {m.releaseUrl ? "Release notes" : "Releases"}
              <ExternalLink className="h-4 w-4" aria-hidden />
            </a>
            <Link to="/docs/getting-started" className="btn-ghost">
              How to install
            </Link>
          </div>
          <p className="mt-4 text-xs text-muted">
            Untested patches may still work: the app shows a warning and you can{" "}
            <Link to="/docs/troubleshooting" className="text-gold hover:underline">
              report what you find
            </Link>
            .
          </p>
        </div>
      </div>
    </Reveal>
  );
}

function PlannedModule({ m }: { m: CatalogModule }) {
  return (
    <div
      id={m.id}
      className="flex h-full scroll-mt-24 flex-col overflow-hidden rounded-2xl border border-ink-500 bg-ink-700/60"
    >
      <GameArt id={m.id} name={m.name} muted className="aspect-[16/9]" />
      <div className="flex flex-1 flex-col gap-3 p-5">
        <div className="self-start">
          <StatusBadge status="planned" />
        </div>
        <p className="text-sm text-muted">{m.tagline || "Planned. Not available yet."}</p>
        <Link
          to="/docs/adding-a-game"
          className="mt-auto inline-flex items-center gap-1.5 pt-2 text-sm font-semibold text-gold hover:text-gold-hover"
        >
          Help build it <ArrowRight className="h-4 w-4" aria-hidden />
        </Link>
      </div>
    </div>
  );
}

export default function Games() {
  useDocumentTitle("Games");
  const { catalog, source } = useCatalog();
  const available = catalog.modules.filter((m) => m.status === "available");
  const planned = catalog.modules.filter((m) => m.status === "planned");
  const updated = source === "live" ? formatDate(catalog.generatedAt) : null;

  return (
    <div className="relative">
      <div aria-hidden className="pointer-events-none absolute inset-x-0 top-0 h-[28rem] overflow-hidden">
        <div className="grid-bg absolute inset-0 [mask-image:radial-gradient(ellipse_60%_80%_at_50%_0%,black,transparent_75%)]" />
        <Glow className="left-1/2 top-[-12rem] h-[28rem] w-[48rem] -translate-x-1/2" />
      </div>

      <div className="container-site relative pb-8 pt-16 sm:pt-24">
        <Reveal>
          <p className="eyebrow">
            <span className="h-px w-6 bg-gold/70" aria-hidden />
            Game library
          </p>
          <h1 className="mt-3 font-display text-5xl font-bold uppercase leading-[0.9] tracking-tight sm:text-7xl">
            Pick your <span className="gold-text">poison</span>
          </h1>
          <p className="mt-5 max-w-2xl text-lg text-muted">
            Every game is its own module with its own version. Install Git Gud Gym once, then add the games you play.
          </p>
          {updated && <p className="mt-3 text-xs text-muted">Catalog updated {updated}.</p>}
        </Reveal>

        <section className="mt-14" aria-labelledby="available-title">
          <h2 id="available-title" className="sr-only">
            Available games
          </h2>
          {available.length > 0 ? (
            <div className="space-y-8">
              {available.map((m) => (
                <AvailableModule key={m.id} m={m} />
              ))}
            </div>
          ) : (
            <p className="rounded-2xl border border-ink-500 bg-ink-700/60 p-6 text-muted">
              No game modules have been released yet.
            </p>
          )}
        </section>

        <section className="mt-20" aria-labelledby="planned-title">
          <Reveal>
            <h2 id="planned-title" className="font-display text-3xl font-semibold uppercase tracking-tight sm:text-4xl">
              On the horizon
            </h2>
            <p className="mt-2 max-w-2xl text-muted">
              These games are planned but not available yet. No dates promised: they arrive when someone builds them.
            </p>
          </Reveal>
          <ul className="mt-8 grid gap-5 sm:grid-cols-2 lg:grid-cols-3">
            {planned.map((m, i) => (
              <Reveal as="li" key={m.id} delay={i * 0.06}>
                <PlannedModule m={m} />
              </Reveal>
            ))}
            <Reveal as="li" delay={planned.length * 0.06}>
              <AddGameCard />
            </Reveal>
          </ul>
        </section>
      </div>
    </div>
  );
}
