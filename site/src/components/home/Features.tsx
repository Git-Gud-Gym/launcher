import type { ReactNode } from "react";
import { motion } from "framer-motion";
import { BellRing, Crosshair, FileSpreadsheet, Footprints, HeartPulse, Keyboard } from "lucide-react";
import { Reveal, SectionHeading } from "../ui";

function Card({
  icon,
  title,
  children,
  className = "",
  delay = 0,
  visual,
}: {
  icon: ReactNode;
  title: string;
  children: ReactNode;
  className?: string;
  delay?: number;
  visual?: ReactNode;
}) {
  return (
    <Reveal
      as="li"
      delay={delay}
      className={`group relative overflow-hidden rounded-2xl border border-ink-500 bg-gradient-to-b from-ink-600/70 to-ink-700/70 p-6 transition-colors duration-300 hover:border-gold/40 sm:p-7 ${className}`}
    >
      <div
        aria-hidden
        className="pointer-events-none absolute -right-16 -top-16 h-40 w-40 rounded-full bg-gold/0 blur-3xl transition-colors duration-500 group-hover:bg-gold/15"
      />
      <div className="flex h-11 w-11 items-center justify-center rounded-xl border border-gold/30 bg-gold/10 text-gold">
        {icon}
      </div>
      <h3 className="mt-5 font-display text-2xl font-semibold uppercase tracking-wide text-bone">{title}</h3>
      <div className="mt-2 text-[15px] leading-relaxed text-muted">{children}</div>
      {visual}
    </Reveal>
  );
}

/** A looping mini-demo: the boss bar drains to zero and refills, the player bar dips and refills. */
function ImmortalDemo() {
  const loop = { duration: 6, repeat: Infinity, ease: "easeInOut" as const };
  return (
    <div className="mt-6 space-y-4 rounded-xl border border-ink-500 bg-ink/60 p-4" aria-hidden>
      <div>
        <div className="mb-1.5 flex justify-between text-xs uppercase tracking-wider text-muted">
          <span>Boss</span>
          <span className="text-gold">refills at 0 HP · counts a defeat</span>
        </div>
        <div className="h-3 overflow-hidden rounded-full bg-ink-600">
          <motion.div
            className="h-full rounded-full bg-gradient-to-r from-gold-dim to-gold"
            initial={{ width: "100%" }}
            animate={{ width: ["100%", "62%", "34%", "8%", "0%", "100%", "100%"] }}
            transition={{ ...loop, times: [0, 0.18, 0.36, 0.52, 0.6, 0.66, 1] }}
          />
        </div>
      </div>
      <div>
        <div className="mb-1.5 flex justify-between text-xs uppercase tracking-wider text-muted">
          <span>You</span>
          <span className="text-blood">lethal hit · HP refilled · death counted</span>
        </div>
        <div className="h-3 overflow-hidden rounded-full bg-ink-600">
          <motion.div
            className="h-full rounded-full bg-gradient-to-r from-[#2f7a3d] to-moss"
            initial={{ width: "100%" }}
            animate={{ width: ["100%", "100%", "55%", "55%", "20%", "20%", "0%", "100%"] }}
            transition={{ ...loop, times: [0, 0.1, 0.16, 0.4, 0.46, 0.8, 0.86, 0.92] }}
          />
        </div>
      </div>
    </div>
  );
}

const hotkeys: [string, string][] = [
  ["F5", "Immortality on / off"],
  ["F6", "Next boss"],
  ["F7", "Let boss die once"],
  ["F8", "Probe"],
  ["F9", "New session"],
  ["F10", "Unload"],
];

export default function Features() {
  return (
    <section className="container-site relative py-24 sm:py-32" aria-labelledby="features-title">
      <SectionHeading
        id="features-title"
        eyebrow="Features"
        title={
          <>
            Fight the boss.
            <br className="hidden sm:block" /> <span className="text-gold">Not the runback.</span>
          </>
        }
        lede="Everything that stands between you and another attempt is removed, and everything that tells you whether you're getting better is recorded."
      />

      <ul className="mt-14 grid gap-4 md:grid-cols-2 lg:grid-cols-3">
        <Card
          className="md:col-span-2"
          icon={<HeartPulse className="h-5 w-5" />}
          title="Nobody dies. Everybody learns."
          visual={<ImmortalDemo />}
        >
          Lethal hits refill your HP and count as a death. At zero HP the boss refills to full and it counts as a
          defeat. The boss stays in its current phase, and phase changes triggered by HP thresholds still happen.
        </Card>
        <Card icon={<Footprints className="h-5 w-5" />} title="Falls don't strand you" delay={0.05}>
          Fall off a ledge and you're put back on the last solid ground you stood on, so you're never stuck at the
          bottom of a cliff. The fall counts as a death, and the distance is adjustable.
        </Card>
        <Card icon={<FileSpreadsheet className="h-5 w-5" />} title="Every attempt logged">
          Damage dealt and taken, hits landed and taken, healing, duration and the lowest boss HP you reached. Saved
          to CSV after every attempt, ready for Excel or Google Sheets.
        </Card>
        <Card icon={<BellRing className="h-5 w-5" />} title="Toasts, HUD & sounds" delay={0.05}>
          "YOU DIED" and "BOSS DEFEATED" pop-ups and a mini HUD drawn over the game, with optional sounds. Works in
          borderless or windowed mode.
        </Card>
        <Card icon={<Crosshair className="h-5 w-5" />} title="Finds the boss" delay={0.1}>
          The boss is picked automatically as you walk into the arena and stays locked during a fight. Not the one
          you wanted? Press <span className="kbd text-xs">F6</span> for the next candidate.
        </Card>
        <Reveal
          as="li"
          className="rounded-2xl border border-ink-500 bg-ink-700/60 p-6 sm:p-7 md:col-span-2 lg:col-span-3"
        >
          <div className="flex flex-col gap-6 lg:flex-row lg:items-center lg:justify-between">
            <div className="flex items-center gap-4">
              <div className="flex h-11 w-11 shrink-0 items-center justify-center rounded-xl border border-gold/30 bg-gold/10 text-gold">
                <Keyboard className="h-5 w-5" />
              </div>
              <div>
                <h3 className="font-display text-2xl font-semibold uppercase tracking-wide text-bone">
                  Never leave the game
                </h3>
                <p className="text-[15px] text-muted">Everything important is a hotkey away.</p>
              </div>
            </div>
            <dl className="grid grid-cols-2 gap-x-6 gap-y-3 sm:grid-cols-3 lg:flex lg:gap-6">
              {hotkeys.map(([k, label]) => (
                <div key={k} className="flex items-center gap-2.5">
                  <dt className="kbd">{k}</dt>
                  <dd className="text-sm text-muted">{label}</dd>
                </div>
              ))}
            </dl>
          </div>
        </Reveal>
      </ul>
    </section>
  );
}
