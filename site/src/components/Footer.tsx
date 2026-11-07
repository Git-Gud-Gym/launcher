import { Link } from "react-router-dom";
import {
  CHANGELOG_URL,
  CONTRIBUTING_URL,
  DISCUSSIONS_URL,
  GITHUB_URL,
  ISSUES_URL,
  LICENSE_URL,
  RELEASES_URL,
} from "../config";
import { LogoMark, Wordmark } from "./Logo";

const columns: { title: string; links: { label: string; to?: string; href?: string }[] }[] = [
  {
    title: "Product",
    links: [
      { label: "Games", to: "/games" },
      { label: "Download", href: RELEASES_URL },
      { label: "Changelog", href: CHANGELOG_URL },
    ],
  },
  {
    title: "Docs",
    links: [
      { label: "Getting started", to: "/docs/getting-started" },
      { label: "Playing offline", to: "/docs/offline" },
      { label: "Troubleshooting", to: "/docs/troubleshooting" },
      { label: "Architecture", to: "/docs/architecture" },
    ],
  },
  {
    title: "Community",
    links: [
      { label: "Source code", href: GITHUB_URL },
      { label: "Report an issue", href: ISSUES_URL },
      { label: "Discussions", href: DISCUSSIONS_URL },
      { label: "Contributing", href: CONTRIBUTING_URL },
    ],
  },
];

export default function Footer() {
  return (
    <footer className="relative mt-24 border-t border-ink-500/80 bg-ink-800/60">
      <div className="hairline absolute inset-x-0 top-0 h-px" aria-hidden />
      <div className="container-site grid gap-10 py-14 sm:grid-cols-3 md:grid-cols-[1.4fr_repeat(3,1fr)]">
        <div className="sm:col-span-3 md:col-span-1">
          <Link to="/" className="inline-flex items-center gap-3" aria-label="Git Gud Gym home">
            <LogoMark className="h-10 w-10" />
            <Wordmark />
          </Link>
          <p className="mt-4 max-w-xs text-sm text-muted">
            Endless boss practice for souls-likes. Free and open source, for Windows.
          </p>
        </div>
        {columns.map((col) => (
          <div key={col.title}>
            <h2 className="font-display text-sm font-semibold uppercase tracking-[0.2em] text-bone">{col.title}</h2>
            <ul className="mt-4 space-y-2.5 text-sm">
              {col.links.map((l) => (
                <li key={l.label}>
                  {l.to ? (
                    <Link to={l.to} className="text-muted transition-colors hover:text-gold-hover">
                      {l.label}
                    </Link>
                  ) : (
                    <a href={l.href} className="text-muted transition-colors hover:text-gold-hover">
                      {l.label}
                    </a>
                  )}
                </li>
              ))}
            </ul>
          </div>
        ))}
      </div>
      <div className="border-t border-ink-500/60">
        <div className="container-site flex flex-col gap-3 py-6 text-xs leading-relaxed text-muted sm:flex-row sm:items-start sm:justify-between">
          <p>
            Released under the{" "}
            <a href={LICENSE_URL} className="underline decoration-ink-400 underline-offset-2 hover:text-bone">
              MIT License
            </a>
            . Use offline, at your own risk.
          </p>
          <p className="max-w-xl sm:text-right">
            Git Gud Gym is a fan-made tool and is not affiliated with or endorsed by FromSoftware or Bandai Namco.
            Game names are trademarks of their respective owners.
          </p>
        </div>
      </div>
    </footer>
  );
}
