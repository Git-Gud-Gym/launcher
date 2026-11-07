import gettingStarted from "./getting-started.md?raw";
import hotkeys from "./hotkeys.md?raw";
import offline from "./offline.md?raw";
import findingYourGame from "./finding-your-game.md?raw";
import stats from "./stats.md?raw";
import troubleshooting from "./troubleshooting.md?raw";
import addingAGame from "./adding-a-game.md?raw";
import architecture from "./architecture.md?raw";

export interface DocPage {
  slug: string;
  title: string;
  /** Short label for the sidebar. */
  nav: string;
  body: string;
}

export interface DocGroup {
  title: string;
  pages: DocPage[];
}

export const DOC_GROUPS: DocGroup[] = [
  {
    title: "Using Git Gud Gym",
    pages: [
      { slug: "getting-started", title: "Getting started", nav: "Getting started", body: gettingStarted },
      { slug: "hotkeys", title: "Hotkeys", nav: "Hotkeys", body: hotkeys },
      { slug: "offline", title: "Playing offline", nav: "Playing offline / Easy Anti-Cheat", body: offline },
      { slug: "finding-your-game", title: "Finding your game", nav: "Finding your game", body: findingYourGame },
      { slug: "stats", title: "Your stats", nav: "Your stats", body: stats },
      { slug: "troubleshooting", title: "Troubleshooting", nav: "Troubleshooting", body: troubleshooting },
    ],
  },
  {
    title: "Contributing",
    pages: [
      { slug: "adding-a-game", title: "Adding a game", nav: "Adding a game", body: addingAGame },
      { slug: "architecture", title: "Architecture", nav: "Architecture", body: architecture },
    ],
  },
];

export const DOC_PAGES: DocPage[] = DOC_GROUPS.flatMap((g) => g.pages);
export const DEFAULT_DOC = DOC_PAGES[0].slug;
