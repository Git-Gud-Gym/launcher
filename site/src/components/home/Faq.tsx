import { useId, useState, type ReactNode } from "react";
import { AnimatePresence, motion } from "framer-motion";
import { Plus } from "lucide-react";
import { Link } from "react-router-dom";
import { availableModules, type Catalog } from "../../catalog";
import { Reveal, SectionHeading } from "../ui";

const baseFaqs: { q: string; a: ReactNode }[] = [
  {
    q: "Is it safe to use? Can I get banned?",
    a: (
      <>
        Git Gud Gym is for <strong className="text-bone">offline play only</strong>. The app refuses to attach while
        Easy Anti-Cheat is running, and it can launch the game offline for you. Modding a game online can get your
        account flagged, so don't try to work around that. As with any mod, use it at your own risk.{" "}
        <Link to="/docs/offline" className="text-gold underline-offset-2 hover:underline">
          Playing offline
        </Link>
      </>
    ),
  },
  {
    q: "Does the boss reset to phase 1 after a defeat?",
    a: (
      <>
        Not yet. When the boss hits zero HP it refills to full and the defeat is counted, but it stays in whatever
        phase it was in. Phase changes triggered by HP thresholds still happen. A true phase-1 reset is on the
        roadmap.
      </>
    ),
  },
  {
    q: "Some bosses get stuck at 1 HP. What's going on?",
    a: (
      <>
        A few bosses change phase through a scripted death (in Elden Ring, Rennala and Radagon into the Elden Beast).
        Press <span className="kbd text-xs">F7</span> before the final hit to let the boss die once so the next phase
        can start.
      </>
    ),
  },
  {
    q: "Why does it ask for administrator rights?",
    a: "The app needs admin rights to attach to the game and to launch it, including when the game itself runs as administrator.",
  },
  {
    q: "I can't see the notifications or the HUD.",
    a: "Switch the game to Borderless or Windowed display mode. Exclusive fullscreen draws over the overlay.",
  },
  {
    q: "Where are my stats saved?",
    a: (
      <>
        In a <code className="rounded bg-ink-600 px-1 py-0.5 text-[0.9em] text-bone">stats</code> folder next to{" "}
        <code className="rounded bg-ink-600 px-1 py-0.5 text-[0.9em] text-bone">GitGudGym.exe</code>, as{" "}
        <code className="rounded bg-ink-600 px-1 py-0.5 text-[0.9em] text-bone">attempts.csv</code> and{" "}
        <code className="rounded bg-ink-600 px-1 py-0.5 text-[0.9em] text-bone">sessions.csv</code>.{" "}
        <Link to="/docs/stats" className="text-gold underline-offset-2 hover:underline">
          What's in them
        </Link>
      </>
    ),
  },
  {
    q: "Does it cost anything?",
    a: "No. Git Gud Gym is free and open source under the MIT license.",
  },
];

function Item({ q, a, defaultOpen = false }: { q: string; a: ReactNode; defaultOpen?: boolean }) {
  const [open, setOpen] = useState(defaultOpen);
  const id = useId();
  return (
    <div className="border-b border-ink-500">
      <h3>
        <button
          type="button"
          aria-expanded={open}
          aria-controls={id}
          onClick={() => setOpen((o) => !o)}
          className="flex w-full items-center justify-between gap-6 py-5 text-left font-sans text-base font-semibold text-bone transition-colors hover:text-gold-hover sm:text-lg"
        >
          {q}
          <Plus
            className={`h-5 w-5 shrink-0 text-gold transition-transform duration-300 ${open ? "rotate-45" : ""}`}
            aria-hidden
          />
        </button>
      </h3>
      <AnimatePresence initial={false}>
        {open && (
          <motion.div
            id={id}
            initial={{ height: 0, opacity: 0 }}
            animate={{ height: "auto", opacity: 1 }}
            exit={{ height: 0, opacity: 0 }}
            transition={{ duration: 0.25, ease: "easeOut" }}
            className="overflow-hidden"
          >
            <div className="max-w-3xl pb-6 text-muted">{a}</div>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  );
}

function gamesAnswer(catalog: Catalog): ReactNode {
  const available = availableModules(catalog);
  const planned = catalog.modules.filter((m) => m.status === "planned");
  return (
    <>
      {available.length > 0 ? (
        <>
          Available now:{" "}
          {available.map((m, i) => (
            <span key={m.id}>
              {i > 0 && ", "}
              <strong className="text-bone">{m.name}</strong>
              {m.game?.testedPatches.length ? ` (tested on patch ${m.game.testedPatches.join(", ")})` : ""}
            </span>
          ))}
          . Other patches may work, and the app warns you when it sees one it hasn't been tested on.{" "}
        </>
      ) : (
        "No game modules have been released yet. "
      )}
      {planned.length > 0 && <>Planned: {planned.map((m) => m.name).join(", ")}. </>}
      See the{" "}
      <Link to="/games" className="text-gold underline-offset-2 hover:underline">
        game library
      </Link>
      .
    </>
  );
}

export default function Faq({ catalog }: { catalog: Catalog }) {
  const faqs = [...baseFaqs];
  faqs.splice(3, 0, { q: "Which games are supported?", a: gamesAnswer(catalog) });
  return (
    <section className="container-site py-24 sm:py-32" aria-labelledby="faq-title">
      <div className="grid gap-12 lg:grid-cols-[1fr_2fr]">
        <SectionHeading
          id="faq-title"
          eyebrow="FAQ"
          title="Questions"
          lede="Something else? Check the docs or ask on GitHub."
        />
        <Reveal className="border-t border-ink-500">
          {faqs.map((f, i) => (
            <Item key={f.q} q={f.q} a={f.a} defaultOpen={i === 0} />
          ))}
        </Reveal>
      </div>
    </section>
  );
}
