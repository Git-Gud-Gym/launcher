import { useRef, useState, type KeyboardEvent } from "react";
import { AnimatePresence, motion } from "framer-motion";
import { asset } from "../../config";
import { Reveal, SectionHeading } from "../ui";

/** A region of docs/images/app.png (1200 x 880), in source pixels. */
interface Region {
  x: number;
  y: number;
  w: number;
  h: number;
}

const SRC_W = 1200;

function Crop({ r, alt }: { r: Region; alt: string }) {
  return (
    <div className="relative w-full overflow-hidden" style={{ aspectRatio: `${r.w} / ${r.h}` }}>
      <img
        src={asset("app.webp")}
        alt={alt}
        loading="lazy"
        className="absolute max-w-none"
        style={{
          width: `${(SRC_W / r.w) * 100}%`,
          left: `${(-r.x / r.w) * 100}%`,
          top: `${(-r.y / r.h) * 100}%`,
        }}
      />
    </div>
  );
}

const tabs: { id: string; label: string; title: string; body: string; region: Region; alt: string }[] = [
  {
    id: "glance",
    label: "At a glance",
    title: "The numbers that matter",
    body: "Deaths (and how many were falls), boss defeats with your fastest time, attempts with the average length, and the lowest HP you've taken the boss to.",
    region: { x: 294, y: 72, w: 882, h: 96 },
    alt: "Counters: 23 deaths, 2 from falls; 2 boss defeats, fastest 3:12; 26 attempts, average 1:41; best boss HP 0%.",
  },
  {
    id: "live",
    label: "Live fight",
    title: "Watch the fight as it happens",
    body: "Live HP bars for you and the auto-detected boss, plus the current attempt's timer, damage dealt and taken, hits and the boss's lowest HP.",
    region: { x: 294, y: 189, w: 479, h: 267 },
    alt: "Live fight panel with the boss HP bar at 37%, the player HP bar, and attempt 27 stats.",
  },
  {
    id: "mods",
    label: "Training mods",
    title: "Toggle what you need",
    body: "Player can't die, boss can't die, falls count as deaths (with an adjustable fall distance) and let the boss die once for scripted phase changes. Each has a hotkey.",
    region: { x: 793, y: 189, w: 383, h: 353 },
    alt: "Training mods panel with switches for player can't die, boss can't die, falling counts as death with a fall distance slider, and let boss die once.",
  },
  {
    id: "session",
    label: "Session",
    title: "See the whole session",
    body: "Totals across every attempt: damage, hits, healing, falls, fastest defeat and average attempt. Start a new session any time with F9.",
    region: { x: 294, y: 479, w: 479, h: 203 },
    alt: "Session panel with total damage dealt and taken, hits landed and taken, healing, falls, fastest defeat and average attempt.",
  },
];

export default function Showcase() {
  const [active, setActive] = useState(0);
  const refs = useRef<(HTMLButtonElement | null)[]>([]);
  const tab = tabs[active];

  const onKeyDown = (e: KeyboardEvent<HTMLDivElement>) => {
    const dir = e.key === "ArrowRight" || e.key === "ArrowDown" ? 1 : e.key === "ArrowLeft" || e.key === "ArrowUp" ? -1 : 0;
    if (!dir) return;
    e.preventDefault();
    const next = (active + dir + tabs.length) % tabs.length;
    setActive(next);
    refs.current[next]?.focus();
  };

  return (
    <section className="relative border-y border-ink-500/70 bg-ink-800/50 py-24 sm:py-32" aria-labelledby="showcase-title">
      <div className="container-site">
        <SectionHeading
          id="showcase-title"
          eyebrow="The app"
          title="Your training partner, on a second screen"
          lede="The desktop app attaches to the game for you, shows the fight live, and detaches when you close it."
        />

        <div className="mt-12 grid items-start gap-8 lg:grid-cols-[minmax(0,20rem)_minmax(0,1fr)] lg:gap-12">
          <Reveal>
            <div role="tablist" aria-label="App panels" aria-orientation="vertical" className="flex gap-2 overflow-x-auto pb-1 lg:flex-col lg:overflow-visible" onKeyDown={onKeyDown}>
              {tabs.map((t, i) => {
                const selected = i === active;
                return (
                  <button
                    key={t.id}
                    ref={(el) => (refs.current[i] = el)}
                    role="tab"
                    id={`tab-${t.id}`}
                    aria-selected={selected}
                    aria-controls={`panel-${t.id}`}
                    tabIndex={selected ? 0 : -1}
                    onClick={() => setActive(i)}
                    className={`relative shrink-0 rounded-xl border px-4 py-3 text-left transition-colors lg:px-5 lg:py-4 ${
                      selected ? "border-gold/50 bg-ink-600" : "border-transparent hover:bg-ink-700"
                    }`}
                  >
                    {selected && (
                      <motion.span
                        layoutId="showcase-bar"
                        className="absolute inset-y-3 left-0 hidden w-0.5 rounded-full bg-gold lg:block"
                        aria-hidden
                      />
                    )}
                    <span className={`font-display text-lg font-semibold uppercase tracking-wider ${selected ? "text-gold" : "text-bone"}`}>
                      {t.label}
                    </span>
                    <span className="mt-1 hidden text-sm leading-snug text-muted lg:block">{t.title}</span>
                  </button>
                );
              })}
            </div>
          </Reveal>

          <Reveal delay={0.1}>
            <div
              role="tabpanel"
              id={`panel-${tab.id}`}
              aria-labelledby={`tab-${tab.id}`}
              className="rounded-2xl bg-gradient-to-b from-gold/40 via-ink-500 to-ink-500/40 p-px shadow-[0_30px_100px_-30px_rgba(212,168,75,0.35)]"
            >
              <div className="overflow-hidden rounded-2xl bg-ink-700">
                <AnimatePresence mode="wait" initial={false}>
                  <motion.div
                    key={tab.id}
                    initial={{ opacity: 0, scale: 0.98 }}
                    animate={{ opacity: 1, scale: 1 }}
                    exit={{ opacity: 0, scale: 0.98 }}
                    transition={{ duration: 0.25 }}
                  >
                    <div className="flex min-h-[12rem] items-center justify-center bg-[#0f1016] p-3 sm:p-6">
                      <div className="w-full overflow-hidden rounded-lg" style={{ maxWidth: tab.region.w }}>
                        <Crop r={tab.region} alt={tab.alt} />
                      </div>
                    </div>
                    <div className="border-t border-ink-500 p-5 sm:p-6">
                      <h3 className="font-display text-2xl font-semibold uppercase tracking-wide text-bone">{tab.title}</h3>
                      <p className="mt-1.5 text-muted">{tab.body}</p>
                    </div>
                  </motion.div>
                </AnimatePresence>
              </div>
            </div>
            <p className="mt-3 text-xs text-muted">Screens show sample data.</p>
          </Reveal>
        </div>
      </div>
    </section>
  );
}
