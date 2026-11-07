import { useId } from "react";

/**
 * Our own abstract "cover art" for a game: typography plus gradients and a geometric motif.
 * Deliberately uses no official artwork, logos or imagery.
 */

type Motif = "rings" | "slashes" | "embers" | "grid";

interface Theme {
  from: string;
  to: string;
  accent: string;
  motif: Motif;
}

const THEMES: Record<string, Theme> = {
  eldenring: { from: "#2a2010", to: "#0e0d0c", accent: "#D4A84B", motif: "rings" },
  sekiro: { from: "#2b0f12", to: "#0d0b0e", accent: "#E5484D", motif: "slashes" },
  darksouls3: { from: "#2a1608", to: "#0c0c10", accent: "#E8823A", motif: "embers" },
};

function hash(s: string) {
  let h = 0;
  for (let i = 0; i < s.length; i++) h = (h * 31 + s.charCodeAt(i)) >>> 0;
  return h;
}

function themeFor(id: string): Theme {
  if (THEMES[id]) return THEMES[id];
  const hue = hash(id) % 360;
  return {
    from: `hsl(${hue} 35% 14%)`,
    to: "#0c0d12",
    accent: `hsl(${hue} 60% 62%)`,
    motif: "grid",
  };
}

function MotifLayer({ motif, accent, pid }: { motif: Motif; accent: string; pid: string }) {
  switch (motif) {
    case "rings":
      return (
        <g fill="none" stroke={accent}>
          {[38, 62, 86, 110, 134, 158].map((r, i) => (
            <circle key={r} cx="300" cy="40" r={r} strokeOpacity={0.32 - i * 0.04} strokeWidth={i === 2 ? 1.6 : 0.8} />
          ))}
          <circle cx="300" cy="40" r="210" strokeOpacity="0.08" strokeDasharray="2 6" />
        </g>
      );
    case "slashes":
      return (
        <g stroke={accent} strokeLinecap="round">
          <line x1="190" y1="-10" x2="390" y2="190" strokeOpacity="0.5" strokeWidth="2" />
          <line x1="230" y1="-20" x2="420" y2="170" strokeOpacity="0.22" strokeWidth="1" />
          <line x1="150" y1="10" x2="300" y2="160" strokeOpacity="0.14" strokeWidth="6" />
          <line x1="250" y1="0" x2="330" y2="200" strokeOpacity="0.12" strokeWidth="1" />
        </g>
      );
    case "embers":
      return (
        <g fill={accent}>
          {Array.from({ length: 26 }, (_, i) => {
            const x = 140 + ((i * 97) % 260);
            const y = 10 + ((i * 53) % 170);
            const r = 0.8 + ((i * 7) % 5) * 0.45;
            return <circle key={i} cx={x} cy={y} r={r} fillOpacity={0.15 + ((i * 13) % 10) / 22} />;
          })}
        </g>
      );
    default:
      return (
        <>
          <defs>
            <pattern id={pid} width="18" height="18" patternUnits="userSpaceOnUse">
              <path d="M18 0H0V18" fill="none" stroke={accent} strokeOpacity="0.16" strokeWidth="0.8" />
            </pattern>
          </defs>
          <rect width="400" height="200" fill={`url(#${pid})`} />
        </>
      );
  }
}

export default function GameArt({
  id,
  name,
  muted = false,
  className = "",
}: {
  id: string;
  name: string;
  muted?: boolean;
  className?: string;
}) {
  const t = themeFor(id);
  const uid = useId().replace(/:/g, "");
  const initials = name
    .replace(/[^A-Za-z0-9 ]/g, " ")
    .split(/\s+/)
    .filter((w) => w && !["of", "the", "and"].includes(w.toLowerCase()))
    .slice(0, 3)
    .map((w) => (/^[IVX]+$/.test(w) ? w : w[0]))
    .join("")
    .toUpperCase();

  return (
    <div
      className={`relative overflow-hidden ${muted ? "grayscale-[0.7]" : ""} ${className}`}
      style={{ background: `linear-gradient(135deg, ${t.from} 0%, ${t.to} 75%)` }}
      aria-hidden
    >
      <svg viewBox="0 0 400 200" preserveAspectRatio="xMidYMid slice" className="absolute inset-0 h-full w-full">
        <defs>
          <radialGradient id={`g-${uid}`} cx="78%" cy="18%" r="70%">
            <stop offset="0%" stopColor={t.accent} stopOpacity="0.38" />
            <stop offset="100%" stopColor={t.accent} stopOpacity="0" />
          </radialGradient>
        </defs>
        <rect width="400" height="200" fill={`url(#g-${uid})`} />
        <MotifLayer motif={t.motif} accent={t.accent} pid={`p-${uid}`} />
      </svg>
      <span
        className="pointer-events-none absolute -bottom-6 right-2 select-none font-display text-[7.5rem] font-bold leading-none tracking-tighter"
        style={{ color: t.accent, opacity: 0.09 }}
      >
        {initials}
      </span>
      <div className="absolute inset-0 bg-gradient-to-t from-ink/90 via-ink/10 to-transparent" />
      <div className="absolute inset-x-0 bottom-0 p-5">
        <span className="block h-0.5 w-8 rounded-full" style={{ background: t.accent }} />
        <span className="mt-2 block font-display text-[1.7rem] font-bold uppercase leading-[0.95] tracking-wide text-bone sm:text-3xl">
          {name}
        </span>
      </div>
    </div>
  );
}
