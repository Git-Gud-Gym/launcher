import { Link } from "react-router-dom";
import { ArrowUpRight, Plus } from "lucide-react";
import type { CatalogModule } from "../catalog";
import GameArt from "./GameArt";
import { StatusBadge } from "./ui";

/** Compact card used in the home page library preview. */
export default function GameCard({ m }: { m: CatalogModule }) {
  const planned = m.status === "planned";
  return (
    <Link
      to={`/games#${m.id}`}
      className="group flex h-full flex-col overflow-hidden rounded-2xl border border-ink-500 bg-ink-700/70 transition-[border-color,transform,box-shadow] duration-300 hover:-translate-y-1 hover:border-gold/50 hover:shadow-[0_24px_60px_-24px_rgba(212,168,75,0.35)]"
    >
      <GameArt id={m.id} name={m.name} muted={planned} className="aspect-[16/9]" />
      <div className="flex flex-1 flex-col gap-3 p-5">
        <div className="flex items-center justify-between gap-3">
          <StatusBadge status={m.status} />
          {m.version && <span className="font-mono text-xs text-muted">v{m.version}</span>}
        </div>
        {m.tagline && <p className="text-sm text-muted">{m.tagline}</p>}
        <div className="mt-auto flex items-center justify-between pt-2 text-sm">
          <span className="text-muted">
            {m.game?.testedPatches.length ? `Tested on ${m.game.testedPatches.join(", ")}` : ""}
          </span>
          <ArrowUpRight
            className="h-4 w-4 text-muted transition-colors group-hover:text-gold"
            aria-hidden
          />
        </div>
      </div>
    </Link>
  );
}

export function AddGameCard() {
  return (
    <Link
      to="/docs/adding-a-game"
      className="group flex h-full min-h-[16rem] flex-col items-center justify-center gap-4 rounded-2xl border border-dashed border-ink-400 bg-transparent p-8 text-center transition-colors hover:border-gold/60 hover:bg-gold/[0.03]"
    >
      <span className="flex h-12 w-12 items-center justify-center rounded-full border border-ink-400 text-muted transition-colors group-hover:border-gold/60 group-hover:text-gold">
        <Plus className="h-6 w-6" aria-hidden />
      </span>
      <span className="font-display text-2xl font-semibold uppercase tracking-wide text-bone">Your game here</span>
      <span className="max-w-[16rem] text-sm text-muted">
        Each game is a module. Know your way around a souls-like's memory? Help add the next one.
      </span>
    </Link>
  );
}
