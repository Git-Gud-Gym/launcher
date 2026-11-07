import { Link } from "react-router-dom";
import { ArrowRight, BookOpen, Github, Library, MonitorDown, ShieldCheck, Star, Swords } from "lucide-react";
import { useCatalog } from "../catalog";
import { CONTRIBUTING_URL, GITHUB_URL } from "../config";
import { compactNumber, showStars, useRepoStats } from "../lib/github";
import DownloadButton from "../components/DownloadButton";
import GameCard, { AddGameCard } from "../components/GameCard";
import { Glow, Reveal, SectionHeading } from "../components/ui";
import Hero from "../components/home/Hero";
import Features from "../components/home/Features";
import Showcase from "../components/home/Showcase";
import Faq from "../components/home/Faq";
import { useDocumentTitle } from "../lib/useDocumentTitle";

const steps = [
  {
    icon: MonitorDown,
    title: "Install Git Gud Gym",
    body: "Download the app, extract it anywhere and run GitGudGym.exe. It asks for administrator rights so it can launch and attach to your game.",
  },
  {
    icon: Library,
    title: "Add a game",
    body: "Pick the games you play from the library. Git Gud Gym finds your install by itself, or you point it at the game's exe.",
  },
  {
    icon: Swords,
    title: "Fight forever",
    body: "Launch the game offline from the app, walk into a boss arena and fight. Every death, defeat and attempt is tracked.",
  },
];

function HowItWorks() {
  return (
    <section className="container-site pb-24 pt-8 sm:pb-32 sm:pt-12" aria-labelledby="how-title">
      <SectionHeading
        id="how-title"
        eyebrow="How it works"
        title="Three steps to the arena"
        lede="Install Git Gud Gym once, then add modules for the games you play."
        center
      />
      <div className="relative mt-16">
        <div
          aria-hidden
          className="absolute left-[16.5%] right-[16.5%] top-8 hidden h-px bg-gradient-to-r from-gold/0 via-gold/50 to-gold/0 md:block"
        />
        <ol className="relative grid gap-10 md:grid-cols-3 md:gap-6">
        {steps.map((s, i) => (
          <Reveal as="li" key={s.title} delay={i * 0.1} className="relative text-center">
            <div className="relative mx-auto flex h-16 w-16 items-center justify-center rounded-2xl border border-gold/40 bg-ink-700 text-gold shadow-[0_0_40px_-8px_rgba(212,168,75,0.5)]">
              <s.icon className="h-7 w-7" aria-hidden />
              <span className="absolute -right-2 -top-2 flex h-6 w-6 items-center justify-center rounded-full bg-gold font-display text-sm font-bold text-ink">
                {i + 1}
              </span>
            </div>
            <h3 className="mt-6 font-display text-2xl font-semibold uppercase tracking-wide text-bone">{s.title}</h3>
            <p className="mx-auto mt-2 max-w-xs text-muted">{s.body}</p>
          </Reveal>
        ))}
        </ol>
      </div>
    </section>
  );
}

function LibraryPreview({ catalog }: { catalog: ReturnType<typeof useCatalog>["catalog"] }) {
  const modules = [...catalog.modules]
    .sort((a, b) => (a.status === b.status ? 0 : a.status === "available" ? -1 : 1))
    .slice(0, 5);
  return (
    <section className="container-site py-24 sm:py-32" aria-labelledby="library-title">
      <div className="flex flex-col justify-between gap-6 md:flex-row md:items-end">
        <SectionHeading
          id="library-title"
          eyebrow="Game library"
          title="One app. Every game you play."
          lede="Each game is a separate module, so you only install what you play."
        />
        <Reveal>
          <Link to="/games" className="btn-ghost group shrink-0">
            All games
            <ArrowRight className="h-5 w-5 transition-transform group-hover:translate-x-0.5" aria-hidden />
          </Link>
        </Reveal>
      </div>
      <ul className="mt-12 grid gap-5 sm:grid-cols-2 lg:grid-cols-3">
        {modules.map((m, i) => (
          <Reveal as="li" key={m.id} delay={i * 0.06}>
            <GameCard m={m} />
          </Reveal>
        ))}
        <Reveal as="li" delay={modules.length * 0.06}>
          <AddGameCard />
        </Reveal>
      </ul>
    </section>
  );
}

function Safety() {
  const points = [
    ["Refuses to attach while Easy Anti-Cheat is running", "No accidental online sessions with mods loaded."],
    ["Launches the game offline for you", "On Steam, it asks before creating steam_appid.txt so the game starts without anti-cheat."],
    ["Cleans up after itself", "Unloading the mod clears every flag it set. Closing the app unloads the mod."],
    ["Careful with memory", "Reads are fault-tolerant, so a bad pointer is skipped instead of crashing the game."],
  ];
  return (
    <section className="container-site py-12" aria-labelledby="safety-title">
      <Reveal className="relative overflow-hidden rounded-3xl border border-gold/25 bg-gradient-to-br from-[#1a1710] via-ink-700 to-ink-700 p-8 sm:p-12">
        <Glow className="-left-24 -top-24 h-72 w-72" />
        <div className="relative grid gap-10 lg:grid-cols-[1fr_1.3fr] lg:items-center">
          <div>
            <div className="flex h-12 w-12 items-center justify-center rounded-xl border border-gold/40 bg-gold/10 text-gold">
              <ShieldCheck className="h-6 w-6" aria-hidden />
            </div>
            <h2 id="safety-title" className="section-title mt-6">
              Offline only. <span className="text-gold">On purpose.</span>
            </h2>
            <p className="mt-4 text-muted">
              Git Gud Gym is a practice tool, not a cheat. It's built to be used offline, where nobody else is
              affected. Modding a game online can get your account flagged. Use it at your own risk.
            </p>
            <Link to="/docs/offline" className="mt-6 inline-flex items-center gap-2 font-semibold text-gold hover:text-gold-hover">
              How to play offline <ArrowRight className="h-4 w-4" aria-hidden />
            </Link>
          </div>
          <ul className="grid gap-4 sm:grid-cols-2">
            {points.map(([title, body]) => (
              <li key={title} className="rounded-2xl border border-ink-500 bg-ink/50 p-5">
                <h3 className="font-sans text-base font-semibold tracking-normal text-bone">{title}</h3>
                <p className="mt-1.5 text-sm text-muted">{body}</p>
              </li>
            ))}
          </ul>
        </div>
      </Reveal>
    </section>
  );
}

function Contribute() {
  const stats = useRepoStats();
  const pieces = [
    ["Game adapter", "A DLL that knows one game's memory: where HP lives, how to keep someone alive."],
    ["module.json", "Describes the module: which game, which exe, which patches it was tested on."],
    ["The library", "Release it and it shows up in the catalog, on this site and in the app."],
  ];
  return (
    <section className="container-site py-24 sm:py-32" aria-labelledby="contribute-title">
      <div className="grid gap-12 lg:grid-cols-2 lg:items-center">
        <div>
          <SectionHeading
            id="contribute-title"
            eyebrow="Open source"
            title={
              <>
                Built in the open.
                <br />
                <span className="text-gold">Bring your game.</span>
              </>
            }
            lede="Git Gud Gym is MIT licensed and written in C++20 with Dear ImGui. Bug reports, offsets for new patches and whole new game modules are all welcome."
          />
          <Reveal className="mt-8 flex flex-col gap-3 sm:flex-row" delay={0.1}>
            <a href={GITHUB_URL} className="btn-gold">
              <Github className="h-5 w-5" aria-hidden />
              View on GitHub
              {showStars(stats) && stats && (
                <span className="ml-1 inline-flex items-center gap-1 rounded-md bg-ink/15 px-1.5 py-0.5 font-sans text-xs font-semibold normal-case tracking-normal">
                  <Star className="h-3 w-3" aria-hidden />
                  {compactNumber(stats.stars)}
                </span>
              )}
            </a>
            <a href={CONTRIBUTING_URL} className="btn-ghost">
              <BookOpen className="h-5 w-5" aria-hidden />
              Contributing guide
            </a>
          </Reveal>
        </div>

        <Reveal delay={0.1} className="relative">
          <Glow className="-right-10 top-10 h-64 w-64" />
          <div className="relative rounded-3xl border border-ink-500 bg-ink-700/80 p-6 sm:p-8">
            <p className="eyebrow">Adding a game</p>
            <ol className="mt-6 space-y-5">
              {pieces.map(([title, body], i) => (
                <li key={title} className="flex gap-4">
                  <span className="flex h-8 w-8 shrink-0 items-center justify-center rounded-lg border border-gold/40 bg-gold/10 font-display text-base font-bold text-gold">
                    {i + 1}
                  </span>
                  <div>
                    <h3 className="font-mono text-[15px] font-semibold tracking-normal text-bone">{title}</h3>
                    <p className="mt-1 text-sm text-muted">{body}</p>
                  </div>
                </li>
              ))}
            </ol>
            <Link
              to="/docs/adding-a-game"
              className="mt-8 inline-flex items-center gap-2 font-semibold text-gold hover:text-gold-hover"
            >
              Read the overview <ArrowRight className="h-4 w-4" aria-hidden />
            </Link>
          </div>
        </Reveal>
      </div>
    </section>
  );
}

function FinalCta({ catalog }: { catalog: ReturnType<typeof useCatalog>["catalog"] }) {
  return (
    <section className="container-site pt-8" aria-labelledby="cta-title">
      <Reveal className="relative overflow-hidden rounded-3xl border border-ink-500 bg-ink-700 px-6 py-16 text-center sm:px-12">
        <div aria-hidden className="grid-bg absolute inset-0 opacity-60 [mask-image:radial-gradient(ellipse_at_center,black,transparent_70%)]" />
        <Glow className="left-1/2 top-1/2 h-80 w-[36rem] -translate-x-1/2 -translate-y-1/2" />
        <div className="relative">
          <h2 id="cta-title" className="font-display text-5xl font-bold uppercase leading-none tracking-tight sm:text-6xl">
            Time to <span className="gold-text">git gud</span>.
          </h2>
          <p className="mx-auto mt-4 max-w-md text-muted">Free, open source, and ready for your next thousand attempts.</p>
          <div className="mt-8 flex flex-col items-stretch justify-center gap-3 sm:flex-row sm:items-center">
            <DownloadButton catalog={catalog} />
            <Link to="/docs/getting-started" className="btn-ghost">
              Read the docs
            </Link>
          </div>
        </div>
      </Reveal>
    </section>
  );
}

export default function Home() {
  const { catalog } = useCatalog();
  useDocumentTitle(null);
  return (
    <>
      <Hero catalog={catalog} />
      <Features />
      <HowItWorks />
      <Showcase />
      <LibraryPreview catalog={catalog} />
      <Safety />
      <Contribute />
      <Faq catalog={catalog} />
      <FinalCta catalog={catalog} />
    </>
  );
}
