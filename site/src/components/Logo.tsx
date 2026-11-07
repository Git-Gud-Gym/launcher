import { asset } from "../config";

export function LogoMark({ className = "h-9 w-9" }: { className?: string }) {
  return (
    <img
      src={asset("logo-mark.webp")}
      alt=""
      width={320}
      height={320}
      className={`${className} select-none rounded-full`}
      draggable={false}
    />
  );
}

export function Wordmark({ className = "" }: { className?: string }) {
  return (
    <span className={`font-display text-xl font-semibold uppercase tracking-[0.12em] text-gold ${className}`}>
      Git Gud Gym
    </span>
  );
}
