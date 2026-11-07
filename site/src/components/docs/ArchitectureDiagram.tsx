import { AppWindow, Cpu, FileJson, Gamepad2, Globe } from "lucide-react";
import type { ReactNode } from "react";

function Node({
  icon,
  title,
  subtitle,
  items,
  accent = false,
}: {
  icon: ReactNode;
  title: string;
  subtitle: string;
  items: string[];
  accent?: boolean;
}) {
  return (
    <div
      className={`relative rounded-2xl border p-4 ${
        accent ? "border-gold/50 bg-gold/[0.06]" : "border-ink-500 bg-ink-700"
      }`}
    >
      <div className="flex items-center gap-3">
        <span className="flex h-9 w-9 shrink-0 items-center justify-center rounded-lg border border-gold/40 bg-gold/10 text-gold">
          {icon}
        </span>
        <div className="min-w-0">
          <div className="font-display text-lg font-semibold uppercase leading-tight tracking-wide text-bone">{title}</div>
          <div className="truncate font-mono text-xs text-muted">{subtitle}</div>
        </div>
      </div>
      <ul className="mt-3 space-y-1 text-[13px] leading-snug text-muted">
        {items.map((i) => (
          <li key={i} className="flex gap-2">
            <span className="mt-[7px] h-1 w-1 shrink-0 rounded-full bg-gold/70" aria-hidden />
            {i}
          </li>
        ))}
      </ul>
    </div>
  );
}

function Link({ label, vertical = false }: { label: string; vertical?: boolean }) {
  return (
    <div
      className={`flex items-center justify-center ${vertical ? "flex-col py-1" : "flex-col py-2 md:flex-row md:px-1 md:py-0"}`}
      aria-hidden
    >
      <span className={`bg-gradient-to-b from-gold/0 via-gold/70 to-gold/0 ${vertical ? "h-6 w-px" : "h-6 w-px md:h-px md:w-6 md:bg-gradient-to-r"}`} />
      <span className="whitespace-nowrap rounded-full border border-gold/40 bg-ink px-2.5 py-0.5 font-mono text-[11px] text-gold">
        {label}
      </span>
      <span className={`bg-gradient-to-b from-gold/0 via-gold/70 to-gold/0 ${vertical ? "h-6 w-px" : "h-6 w-px md:h-px md:w-6 md:bg-gradient-to-r"}`} />
    </div>
  );
}

/** Hub app <-> shared memory <-> in-game module, with the catalog feeding the library. */
export default function ArchitectureDiagram() {
  return (
    <figure className="not-prose my-8">
      <div className="rounded-3xl border border-ink-500 bg-ink-800 p-4 sm:p-6">
        <div className="grid items-center md:grid-cols-[1fr_auto_1fr]">
          <Node
            accent
            icon={<AppWindow className="h-5 w-5" />}
            title="Hub app"
            subtitle="GitGudGym.exe"
            items={["Game library & installs", "Launch offline, attach", "Live UI, stats, overlay, sounds"]}
          />
          <Link label="shared memory" />
          <div className="rounded-2xl border border-dashed border-ink-400 p-2">
            <div className="mb-2 flex items-center gap-2 px-2 pt-1 text-xs uppercase tracking-wider text-muted">
              <Gamepad2 className="h-4 w-4" aria-hidden /> Game process
            </div>
            <Node
              icon={<Cpu className="h-5 w-5" />}
              title="Game module"
              subtitle="gitgudgym-<game>.dll"
              items={["Game adapter: HP, flags, position", "Immortality, falls, boss tracking", "Attempt stats & events"]}
            />
          </div>

          <Link label="library reads" vertical />
          <div className="hidden md:block" />
          <div className="hidden md:block" />

          <Node
            icon={<FileJson className="h-5 w-5" />}
            title="Catalog"
            subtitle="catalog.json"
            items={["Built from module.json + GitHub releases", "Versions, tested patches, downloads"]}
          />
          <Link label="also read by" />
          <Node
            icon={<Globe className="h-5 w-5" />}
            title="This website"
            subtitle="Games page"
            items={["Same catalog as the app", "Available & planned games"]}
          />
        </div>
      </div>
      <figcaption className="mt-3 text-center text-xs text-muted">
        The app and the module talk only through one shared block of memory.
      </figcaption>
    </figure>
  );
}
