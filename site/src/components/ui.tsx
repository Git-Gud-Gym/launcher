import type { ReactNode } from "react";
import { motion } from "framer-motion";
import type { ModuleStatus } from "../catalog";

/** Fades and lifts its children in once they scroll into view. */
export function Reveal({
  children,
  delay = 0,
  className = "",
  as = "div",
}: {
  children: ReactNode;
  delay?: number;
  className?: string;
  as?: "div" | "li" | "section" | "article";
}) {
  const Comp = motion[as];
  return (
    <Comp
      className={className}
      initial={{ opacity: 0, y: 18 }}
      whileInView={{ opacity: 1, y: 0 }}
      viewport={{ once: true, margin: "0px 0px -60px 0px" }}
      transition={{ duration: 0.6, delay, ease: [0.22, 1, 0.36, 1] }}
    >
      {children}
    </Comp>
  );
}

export function SectionHeading({
  eyebrow,
  title,
  lede,
  center = false,
  id,
}: {
  eyebrow: string;
  title: ReactNode;
  lede?: ReactNode;
  center?: boolean;
  id?: string;
}) {
  return (
    <Reveal className={center ? "mx-auto text-center" : ""}>
      <p className="eyebrow">
        <span className="h-px w-6 bg-gold/70" aria-hidden />
        {eyebrow}
      </p>
      <h2 id={id} className="section-title mt-3">
        {title}
      </h2>
      {lede && <p className={`section-lede ${center ? "mx-auto" : ""}`}>{lede}</p>}
    </Reveal>
  );
}

export function StatusBadge({ status }: { status: ModuleStatus }) {
  if (status === "available") {
    return (
      <span className="inline-flex items-center gap-1.5 rounded-full border border-moss/40 bg-moss/10 px-2.5 py-0.5 text-xs font-semibold uppercase tracking-wider text-[#7DD38B]">
        <span className="h-1.5 w-1.5 rounded-full bg-moss shadow-[0_0_8px_#46A758]" aria-hidden />
        Available
      </span>
    );
  }
  return (
    <span className="inline-flex items-center gap-1.5 rounded-full border border-ink-400 bg-ink-600 px-2.5 py-0.5 text-xs font-semibold uppercase tracking-wider text-muted">
      <span className="h-1.5 w-1.5 rounded-full border border-muted" aria-hidden />
      Planned
    </span>
  );
}

/** Ambient gold glow blob for section backgrounds. */
export function Glow({ className = "" }: { className?: string }) {
  return (
    <div
      aria-hidden
      className={`pointer-events-none absolute rounded-full bg-[radial-gradient(closest-side,rgba(212,168,75,0.22),transparent)] blur-2xl ${className}`}
    />
  );
}
