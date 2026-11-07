import { copyFileSync } from "node:fs";
import { resolve } from "node:path";
import { defineConfig, type Plugin } from "vite";
import react from "@vitejs/plugin-react";

/** Normalise SITE_BASE so it always starts and ends with "/". */
function resolveBase(raw: string | undefined): string {
  const value = (raw ?? "/git-gud-gym/").trim() || "/";
  const withLead = value.startsWith("/") ? value : `/${value}`;
  return withLead.endsWith("/") ? withLead : `${withLead}/`;
}

/**
 * GitHub Pages has no SPA fallback. Serving a copy of index.html as 404.html makes
 * deep links like /docs/stats load the app, which then renders the right route.
 */
function spaFallback(): Plugin {
  let outDir = "dist";
  return {
    name: "spa-404-fallback",
    apply: "build",
    configResolved(config) {
      outDir = resolve(config.root, config.build.outDir);
    },
    closeBundle() {
      copyFileSync(resolve(outDir, "index.html"), resolve(outDir, "404.html"));
    },
  };
}

export default defineConfig({
  base: resolveBase(process.env.SITE_BASE),
  plugins: [react(), spaFallback()],
  build: {
    target: "es2020",
    rollupOptions: {
      output: {
        manualChunks: {
          react: ["react", "react-dom", "react-router-dom"],
          motion: ["framer-motion"],
        },
      },
    },
  },
});
