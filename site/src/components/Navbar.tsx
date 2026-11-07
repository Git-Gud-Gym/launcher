import { useEffect, useState } from "react";
import { Link, NavLink, useLocation } from "react-router-dom";
import { AnimatePresence, motion } from "framer-motion";
import { Github, Menu, Star, X } from "lucide-react";
import { GITHUB_URL } from "../config";
import { compactNumber, showStars, useRepoStats } from "../lib/github";
import { LogoMark, Wordmark } from "./Logo";

const links = [
  { to: "/games", label: "Games" },
  { to: "/docs/getting-started", label: "Docs" },
  { to: "/docs/adding-a-game", label: "Contribute" },
];

function isActive(to: string, pathname: string) {
  if (to === "/docs/getting-started") return pathname.startsWith("/docs") && !pathname.startsWith("/docs/adding-a-game");
  return pathname === to || pathname.startsWith(`${to}/`);
}

export default function Navbar() {
  const [open, setOpen] = useState(false);
  const [scrolled, setScrolled] = useState(false);
  const stats = useRepoStats();
  const { pathname } = useLocation();

  useEffect(() => setOpen(false), [pathname]);

  useEffect(() => {
    const onScroll = () => setScrolled(window.scrollY > 8);
    onScroll();
    window.addEventListener("scroll", onScroll, { passive: true });
    return () => window.removeEventListener("scroll", onScroll);
  }, []);

  useEffect(() => {
    if (!open) return;
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") setOpen(false);
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [open]);

  return (
    <header
      className={`sticky top-0 z-50 transition-colors duration-300 ${
        scrolled || open ? "border-b border-ink-500/80 bg-ink/80 backdrop-blur-xl" : "border-b border-transparent"
      }`}
    >
      <nav className="container-site flex h-16 items-center justify-between gap-4" aria-label="Main">
        <Link to="/" className="group flex items-center gap-3" aria-label="Git Gud Gym home">
          <LogoMark className="h-9 w-9 transition-transform duration-500 group-hover:rotate-[20deg]" />
          <Wordmark />
        </Link>

        <div className="hidden items-center gap-1 md:flex">
          {links.map((l) => {
            const active = isActive(l.to, pathname);
            return (
              <NavLink
                key={l.to}
                to={l.to}
                aria-current={active ? "page" : undefined}
                className={`relative rounded-lg px-3 py-2 font-display text-[17px] font-medium uppercase tracking-wider transition-colors ${
                  active ? "text-gold" : "text-muted hover:text-bone"
                }`}
              >
                {l.label}
                {active && (
                  <motion.span
                    layoutId="nav-underline"
                    className="absolute inset-x-3 -bottom-px h-px bg-gold"
                    transition={{ type: "spring", stiffness: 400, damping: 34 }}
                  />
                )}
              </NavLink>
            );
          })}
          <a
            href={GITHUB_URL}
            className="ml-3 inline-flex items-center gap-2 rounded-lg border border-ink-500 bg-ink-700/70 px-3 py-1.5 text-sm font-medium text-bone transition-colors hover:border-gold/60"
          >
            <Github className="h-4 w-4" aria-hidden />
            GitHub
            {showStars(stats) && stats && (
              <span className="flex items-center gap-1 border-l border-ink-500 pl-2 text-muted">
                <Star className="h-3.5 w-3.5 text-gold" aria-hidden />
                <span className="sr-only">Stars:</span>
                {compactNumber(stats.stars)}
              </span>
            )}
          </a>
        </div>

        <button
          type="button"
          className="inline-flex h-10 w-10 items-center justify-center rounded-lg border border-ink-500 text-bone md:hidden"
          aria-expanded={open}
          aria-controls="mobile-nav"
          aria-label={open ? "Close menu" : "Open menu"}
          onClick={() => setOpen((o) => !o)}
        >
          {open ? <X className="h-5 w-5" /> : <Menu className="h-5 w-5" />}
        </button>
      </nav>

      <AnimatePresence initial={false}>
        {open && (
          <motion.div
            id="mobile-nav"
            initial={{ height: 0, opacity: 0 }}
            animate={{ height: "auto", opacity: 1 }}
            exit={{ height: 0, opacity: 0 }}
            transition={{ duration: 0.22, ease: "easeOut" }}
            className="overflow-hidden md:hidden"
          >
            <div className="container-site flex flex-col gap-1 pb-5 pt-1">
              {links.map((l) => (
                <Link
                  key={l.to}
                  to={l.to}
                  className="rounded-lg px-3 py-3 font-display text-xl font-medium uppercase tracking-wider text-bone hover:bg-ink-700"
                >
                  {l.label}
                </Link>
              ))}
              <a
                href={GITHUB_URL}
                className="flex items-center gap-2 rounded-lg px-3 py-3 font-display text-xl font-medium uppercase tracking-wider text-bone hover:bg-ink-700"
              >
                <Github className="h-5 w-5" aria-hidden /> GitHub
                {showStars(stats) && stats && (
                  <span className="ml-auto font-sans text-sm normal-case tracking-normal text-muted">
                    {compactNumber(stats.stars)} stars
                  </span>
                )}
              </a>
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </header>
  );
}
