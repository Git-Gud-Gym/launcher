import type { Config } from "tailwindcss";
import typography from "@tailwindcss/typography";

export default {
  content: ["./index.html", "./src/**/*.{ts,tsx}"],
  theme: {
    extend: {
      colors: {
        ink: {
          DEFAULT: "#0C0D12",
          900: "#0C0D12",
          800: "#111219",
          700: "#15171E",
          600: "#1C1E27",
          500: "#272A35",
          400: "#353946",
        },
        gold: {
          DEFAULT: "#D4A84B",
          hover: "#E6BF65",
          dim: "#9C7A34",
        },
        blood: "#E5484D",
        moss: "#46A758",
        bone: "#ECEAE6",
        muted: "#8A909C",
      },
      fontFamily: {
        display: ['"Barlow Condensed"', "Bahnschrift", '"Arial Narrow"', "sans-serif"],
        sans: ['"Inter Variable"', "Inter", "system-ui", "Segoe UI", "sans-serif"],
        mono: ['ui-monospace', '"Cascadia Code"', '"SF Mono"', "Consolas", "monospace"],
      },
      maxWidth: {
        site: "76rem",
      },
      keyframes: {
        "spin-slow": { to: { transform: "rotate(360deg)" } },
        shimmer: {
          "0%": { backgroundPosition: "-200% 0" },
          "100%": { backgroundPosition: "200% 0" },
        },
      },
      animation: {
        "spin-slow": "spin-slow 60s linear infinite",
        shimmer: "shimmer 6s linear infinite",
      },
    },
  },
  plugins: [typography],
} satisfies Config;
