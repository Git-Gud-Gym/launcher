import { Children, isValidElement, type ReactElement, type ReactNode } from "react";
import ReactMarkdown, { type Components } from "react-markdown";
import remarkGfm from "remark-gfm";
import { Link } from "react-router-dom";
import { AlertTriangle, Info, Lightbulb, OctagonAlert } from "lucide-react";
import { DOC_VARS } from "../../config";
import ArchitectureDiagram from "./ArchitectureDiagram";

/* ---------- helpers ---------- */

export function slugify(text: string): string {
  return text
    .toLowerCase()
    .replace(/[`*_~[\]()<>]/g, "")
    .replace(/[^a-z0-9\s-]/g, "")
    .trim()
    .replace(/\s+/g, "-");
}

function textOf(node: ReactNode): string {
  if (typeof node === "string" || typeof node === "number") return String(node);
  if (Array.isArray(node)) return node.map(textOf).join("");
  if (isValidElement(node)) return textOf((node.props as { children?: ReactNode }).children);
  return "";
}

/** Replace {{NAME}} placeholders with values from config, so docs never hardcode the repo. */
export function fillVars(md: string): string {
  const vars: Record<string, string> = {
    ...DOC_VARS,
    CATALOG_URL: `${window.location.origin}${import.meta.env.BASE_URL}catalog.json`,
  };
  return md.replace(/\{\{([A-Z_]+)\}\}/g, (m, key: string) => vars[key] ?? m);
}

/** Level-2 headings of a Markdown document, for the "On this page" list. */
export function headingsOf(md: string): { id: string; text: string }[] {
  const out: { id: string; text: string }[] = [];
  let fenced = false;
  for (const line of md.split(/\r?\n/)) {
    if (/^```/.test(line)) fenced = !fenced;
    if (fenced) continue;
    const m = /^##\s+(.+?)\s*$/.exec(line);
    if (m) {
      const text = m[1].replace(/[`*_]/g, "");
      out.push({ id: slugify(text), text });
    }
  }
  return out;
}

/* ---------- GitHub-style callouts: "> [!NOTE]" ---------- */

interface HastNode {
  type: string;
  tagName?: string;
  value?: string;
  properties?: Record<string, unknown>;
  children?: HastNode[];
}

const CALLOUT_RE = /^\s*\[!(NOTE|TIP|IMPORTANT|WARNING|CAUTION)\]\s*/;

function rehypeCallouts() {
  const visit = (node: HastNode) => {
    if (node.type === "element" && node.tagName === "blockquote") {
      const p = node.children?.find((c) => c.type === "element" && c.tagName === "p");
      const first = p?.children?.[0];
      if (first?.type === "text" && first.value) {
        const m = CALLOUT_RE.exec(first.value);
        if (m) {
          first.value = first.value.slice(m[0].length);
          node.properties = { ...node.properties, dataCallout: m[1].toLowerCase() };
        }
      }
    }
    node.children?.forEach(visit);
  };
  return (tree: HastNode) => visit(tree);
}

const calloutStyle: Record<string, { label: string; icon: ReactNode; cls: string }> = {
  note: { label: "Note", icon: <Info className="h-4 w-4" />, cls: "border-sky-400/40 bg-sky-400/[0.06] text-sky-300" },
  tip: { label: "Tip", icon: <Lightbulb className="h-4 w-4" />, cls: "border-moss/50 bg-moss/[0.07] text-[#7DD38B]" },
  important: { label: "Important", icon: <OctagonAlert className="h-4 w-4" />, cls: "border-gold/50 bg-gold/[0.07] text-gold" },
  warning: { label: "Warning", icon: <AlertTriangle className="h-4 w-4" />, cls: "border-blood/50 bg-blood/[0.07] text-[#F2777B]" },
  caution: { label: "Caution", icon: <AlertTriangle className="h-4 w-4" />, cls: "border-blood/50 bg-blood/[0.07] text-[#F2777B]" },
};

/* ---------- renderers ---------- */

function Heading({ level, children }: { level: 2 | 3; children?: ReactNode }) {
  const id = slugify(textOf(children));
  const Tag = level === 2 ? "h2" : "h3";
  return (
    <Tag id={id} className="group">
      {children}
      <a href={`#${id}`} className="ml-2 no-underline opacity-0 transition-opacity focus:opacity-100 group-hover:opacity-70" aria-label="Link to this section">
        #
      </a>
    </Tag>
  );
}

const components: Components = {
  h2: ({ children }) => <Heading level={2}>{children}</Heading>,
  h3: ({ children }) => <Heading level={3}>{children}</Heading>,
  a: ({ href = "", children }) => {
    const internal = href.startsWith("/") && !/\.[a-z0-9]+(#.*)?$/i.test(href);
    if (internal) return <Link to={href}>{children}</Link>;
    if (href.startsWith("#")) return <a href={href}>{children}</a>;
    return (
      <a href={href} target="_blank" rel="noreferrer noopener">
        {children}
      </a>
    );
  },
  code: ({ className, children }) => {
    const text = textOf(children);
    if (!className && /^F\d{1,2}$/.test(text)) return <kbd className="kbd not-prose">{text}</kbd>;
    return <code className={className}>{children}</code>;
  },
  pre: ({ children }) => {
    const child = Children.toArray(children)[0];
    if (isValidElement(child)) {
      const cls = (child as ReactElement<{ className?: string }>).props.className ?? "";
      if (cls.includes("language-diagram")) return <ArchitectureDiagram />;
    }
    return <pre>{children}</pre>;
  },
  table: ({ children }) => (
    <div className="my-6 overflow-x-auto rounded-xl border border-ink-500">
      <table className="my-0 w-full">{children}</table>
    </div>
  ),
  blockquote: ({ node, children }) => {
    const kind = (node?.properties as Record<string, unknown> | undefined)?.dataCallout as string | undefined;
    const style = kind ? calloutStyle[kind] : undefined;
    if (!style) return <blockquote>{children}</blockquote>;
    return (
      <aside className={`not-italic my-6 rounded-xl border-l-2 border-y-0 border-r-0 px-5 py-4 ${style.cls}`} role="note">
        <div className="mb-1 flex items-center gap-2 font-display text-sm font-semibold uppercase tracking-[0.15em]">
          {style.icon}
          {style.label}
        </div>
        <div className="text-bone/90 [&>p]:my-1">{children}</div>
      </aside>
    );
  },
};

export default function Markdown({ source }: { source: string }) {
  return (
    <ReactMarkdown remarkPlugins={[remarkGfm]} rehypePlugins={[rehypeCallouts]} components={components}>
      {fillVars(source)}
    </ReactMarkdown>
  );
}
