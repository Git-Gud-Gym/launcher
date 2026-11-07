import { useRef } from "react";
import { Link } from "react-router-dom";
import { motion, useReducedMotion, useScroll, useTransform } from "framer-motion";
import { ArrowRight } from "lucide-react";
import { availableModules, type Catalog } from "../../catalog";
import { asset } from "../../config";
import DownloadButton from "../DownloadButton";
import { LogoMark } from "../Logo";

const ease = [0.22, 1, 0.36, 1] as const;

function Screenshot() {
  const ref = useRef<HTMLDivElement>(null);
  const reduce = useReducedMotion();
  const { scrollYProgress } = useScroll({ target: ref, offset: ["start end", "center center"] });
  const rotateX = useTransform(scrollYProgress, [0, 1], [reduce ? 0 : 16, 0]);
  const scale = useTransform(scrollYProgress, [0, 1], [reduce ? 1 : 0.92, 1]);

  return (
    <div ref={ref} className="relative mx-auto mt-16 max-w-5xl [perspective:1600px] sm:mt-20">
      <div
        aria-hidden
        className="absolute -inset-x-10 -top-10 bottom-0 rounded-[3rem] bg-[radial-gradient(60%_50%_at_50%_30%,rgba(212,168,75,0.28),transparent_70%)] blur-2xl"
      />
      <motion.figure
        style={{ rotateX, scale, transformOrigin: "50% 0%" }}
        initial={{ opacity: 0, y: 40 }}
        animate={{ opacity: 1, y: 0 }}
        transition={{ duration: 0.9, delay: 0.35, ease }}
        className="relative"
      >
        <div className="rounded-2xl bg-gradient-to-b from-gold/50 via-ink-500 to-ink-500/30 p-px shadow-[0_40px_120px_-30px_rgba(0,0,0,0.9)]">
          <div className="overflow-hidden rounded-2xl bg-ink-700">
            <img
              src={asset("app.webp")}
              width={1200}
              height={880}
              alt="The Git Gud Gym desktop app mid-fight: deaths, boss defeats and attempts counters, live boss and player HP bars, session stats, training toggles and an activity feed."
              className="block h-auto w-full"
            />
          </div>
        </div>
        <figcaption className="mt-4 text-center text-xs text-muted">The desktop app mid-fight (sample data).</figcaption>
      </motion.figure>
    </div>
  );
}

export default function Hero({ catalog }: { catalog: Catalog }) {
  const games = availableModules(catalog).map((m) => m.name);
  const hasPlanned = catalog.modules.some((m) => m.status === "planned");

  return (
    <section className="relative overflow-hidden pb-10 pt-10 sm:pt-16" aria-labelledby="hero-title">
      {/* Backdrop: grid fading out, warm glow, vignette */}
      <div aria-hidden className="pointer-events-none absolute inset-0">
        <div className="grid-bg absolute inset-0 [mask-image:radial-gradient(ellipse_70%_55%_at_50%_0%,black,transparent_75%)]" />
        <div className="absolute left-1/2 top-[-18rem] h-[38rem] w-[60rem] -translate-x-1/2 rounded-full bg-[radial-gradient(closest-side,rgba(212,168,75,0.20),transparent)]" />
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_center,transparent_55%,rgba(0,0,0,0.5))] [mask-image:linear-gradient(to_bottom,black_55%,transparent)]" />
      </div>

      <div className="container-site relative text-center">
        <motion.div
          initial={{ opacity: 0, scale: 0.85 }}
          animate={{ opacity: 1, scale: 1 }}
          transition={{ duration: 0.8, ease }}
          className="relative mx-auto h-28 w-28 sm:h-32 sm:w-32"
        >
          <svg viewBox="0 0 100 100" className="absolute -inset-4 h-[calc(100%+2rem)] w-[calc(100%+2rem)] animate-spin-slow" aria-hidden>
            <circle cx="50" cy="50" r="48" fill="none" stroke="#D4A84B" strokeOpacity="0.35" strokeWidth="0.4" strokeDasharray="1 3" />
          </svg>
          <div className="absolute inset-0 rounded-full shadow-[0_0_80px_-10px_rgba(212,168,75,0.55)]" aria-hidden />
          <LogoMark className="relative h-full w-full" />
        </motion.div>

        <motion.div
          initial={{ opacity: 0, y: 14 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.7, delay: 0.1, ease }}
          className="mt-8 flex flex-wrap items-center justify-center gap-2"
        >
          <span className="chip">Free &amp; open source</span>
          <span className="chip">Windows 10 / 11</span>
          <span className="chip">Offline only</span>
        </motion.div>

        <motion.h1
          id="hero-title"
          initial={{ opacity: 0, y: 18 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.8, delay: 0.15, ease }}
          className="mx-auto mt-6 max-w-4xl font-display text-[3.2rem] font-bold uppercase leading-[0.9] tracking-tight text-bone sm:text-7xl lg:text-[5.75rem]"
        >
          Endless boss practice <span className="gold-text animate-shimmer">for souls-likes</span>
        </motion.h1>

        <motion.p
          initial={{ opacity: 0, y: 14 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.8, delay: 0.25, ease }}
          className="mx-auto mt-6 max-w-2xl text-lg text-muted sm:text-xl"
        >
          Neither you nor the boss can die. Keep fighting, learn the moveset, and watch every attempt get logged
          so you can see yourself improve.
        </motion.p>

        <motion.div
          initial={{ opacity: 0, y: 14 }}
          animate={{ opacity: 1, y: 0 }}
          transition={{ duration: 0.8, delay: 0.32, ease }}
          className="mt-9 flex flex-col items-stretch justify-center gap-3 sm:flex-row sm:items-center"
        >
          <DownloadButton catalog={catalog} />
          <Link to="/games" className="btn-ghost group">
            Browse games
            <ArrowRight className="h-5 w-5 transition-transform group-hover:translate-x-0.5" aria-hidden />
          </Link>
        </motion.div>

        {games.length > 0 && (
          <motion.p
            initial={{ opacity: 0 }}
            animate={{ opacity: 1 }}
            transition={{ duration: 0.8, delay: 0.45 }}
            className="mt-5 text-sm text-muted"
          >
            Available now for <span className="text-bone">{games.join(", ")}</span>.
            {hasPlanned && " More games are planned."}
          </motion.p>
        )}

        <Screenshot />
      </div>
    </section>
  );
}
