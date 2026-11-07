import { useEffect, useMemo, useState } from "react";
import { Link, Navigate, NavLink, useParams } from "react-router-dom";
import { ArrowLeft, ArrowRight, ChevronDown, Pencil } from "lucide-react";
import { DEFAULT_DOC, DOC_GROUPS, DOC_PAGES } from "../docs";
import Markdown, { headingsOf } from "../components/docs/Markdown";
import { GITHUB_URL } from "../config";
import NotFound from "./NotFound";
import { useDocumentTitle } from "../lib/useDocumentTitle";

function Sidebar({ current }: { current: string }) {
  return (
    <nav aria-label="Documentation" className="space-y-8">
      {DOC_GROUPS.map((g) => (
        <div key={g.title}>
          <h2 className="px-3 font-display text-xs font-semibold uppercase tracking-[0.22em] text-muted">{g.title}</h2>
          <ul className="mt-3 space-y-0.5 border-l border-ink-500">
            {g.pages.map((p) => (
              <li key={p.slug}>
                <NavLink
                  to={`/docs/${p.slug}`}
                  aria-current={p.slug === current ? "page" : undefined}
                  className={`-ml-px block border-l px-3 py-1.5 text-sm transition-colors ${
                    p.slug === current
                      ? "border-gold font-medium text-gold"
                      : "border-transparent text-muted hover:border-ink-400 hover:text-bone"
                  }`}
                >
                  {p.nav}
                </NavLink>
              </li>
            ))}
          </ul>
        </div>
      ))}
    </nav>
  );
}

/** Highlights the section currently in view in the "On this page" list. */
function useActiveHeading(ids: string[]) {
  const [active, setActive] = useState<string | null>(null);
  useEffect(() => {
    if (!ids.length || !("IntersectionObserver" in window)) return;
    const els = ids.map((id) => document.getElementById(id)).filter((e): e is HTMLElement => !!e);
    const obs = new IntersectionObserver(
      (entries) => {
        const visible = entries.filter((e) => e.isIntersecting).sort((a, b) => a.boundingClientRect.top - b.boundingClientRect.top);
        if (visible[0]) setActive(visible[0].target.id);
      },
      { rootMargin: "-80px 0px -65% 0px" },
    );
    els.forEach((e) => obs.observe(e));
    return () => obs.disconnect();
  }, [ids]);
  return active;
}

export default function Docs() {
  const { slug } = useParams();
  const page = DOC_PAGES.find((p) => p.slug === slug);
  const [menuOpen, setMenuOpen] = useState(false);
  const headings = useMemo(() => (page ? headingsOf(page.body) : []), [page]);
  const ids = useMemo(() => headings.map((h) => h.id), [headings]);
  const active = useActiveHeading(ids);
  useDocumentTitle(page ? page.title : "Docs");

  useEffect(() => setMenuOpen(false), [slug]);

  if (!slug) return <Navigate to={`/docs/${DEFAULT_DOC}`} replace />;
  if (!page) return <NotFound />;

  const index = DOC_PAGES.indexOf(page);
  const prev = DOC_PAGES[index - 1];
  const next = DOC_PAGES[index + 1];
  const group = DOC_GROUPS.find((g) => g.pages.includes(page));

  return (
    <div className="container-site grid gap-10 pb-10 pt-10 lg:grid-cols-[15rem_minmax(0,1fr)] lg:pt-14 xl:grid-cols-[15rem_minmax(0,1fr)_12rem]">
      {/* Mobile menu */}
      <div className="lg:hidden">
        <button
          type="button"
          onClick={() => setMenuOpen((o) => !o)}
          aria-expanded={menuOpen}
          aria-controls="docs-menu"
          className="flex w-full items-center justify-between rounded-xl border border-ink-500 bg-ink-700 px-4 py-3 text-left"
        >
          <span>
            <span className="block text-xs uppercase tracking-wider text-muted">Docs</span>
            <span className="font-medium text-bone">{page.nav}</span>
          </span>
          <ChevronDown className={`h-5 w-5 text-muted transition-transform ${menuOpen ? "rotate-180" : ""}`} aria-hidden />
        </button>
        {menuOpen && (
          <div id="docs-menu" className="mt-2 rounded-xl border border-ink-500 bg-ink-700 p-4">
            <Sidebar current={page.slug} />
          </div>
        )}
      </div>

      {/* Desktop sidebar */}
      <aside className="hidden lg:block">
        <div className="sticky top-24 max-h-[calc(100vh-7rem)] overflow-y-auto pb-8">
          <Sidebar current={page.slug} />
        </div>
      </aside>

      <article className="min-w-0">
        {group && <p className="eyebrow mb-3">{group.title}</p>}
        <div
          className="prose prose-invert max-w-none prose-headings:font-display prose-headings:font-semibold prose-headings:tracking-tight
            prose-h1:mb-6 prose-h1:text-5xl prose-h1:uppercase prose-h1:leading-none sm:prose-h1:text-6xl
            prose-h2:mt-12 prose-h2:border-t prose-h2:border-ink-500 prose-h2:pt-8 prose-h2:text-3xl prose-h2:uppercase
            prose-h3:text-xl prose-h3:font-sans prose-h3:font-semibold prose-h3:tracking-normal
            prose-p:text-[#c9c7c2] prose-li:text-[#c9c7c2] prose-strong:text-bone
            prose-a:font-medium prose-a:text-gold prose-a:no-underline hover:prose-a:text-gold-hover hover:prose-a:underline
            prose-code:rounded prose-code:bg-ink-600 prose-code:px-1.5 prose-code:py-0.5 prose-code:font-normal prose-code:text-bone prose-code:before:content-none prose-code:after:content-none
            prose-pre:border prose-pre:border-ink-500 prose-pre:bg-ink-800
            prose-th:bg-ink-700 prose-th:px-4 prose-th:py-2.5 prose-th:text-left prose-th:font-display prose-th:text-sm prose-th:uppercase prose-th:tracking-wider prose-th:text-muted
            prose-td:px-4 prose-td:py-2.5 prose-td:align-top prose-tr:border-ink-500 prose-thead:border-ink-500
            prose-blockquote:border-gold/50 prose-blockquote:text-muted
            prose-hr:border-ink-500 marker:text-gold/70"
        >
          <Markdown source={page.body} />
        </div>

        <div className="mt-12 flex flex-wrap items-center justify-between gap-4 border-t border-ink-500 pt-6 text-sm">
          <a
            href={`${GITHUB_URL}/edit/main/site/src/docs/${page.slug}.md`}
            className="inline-flex items-center gap-2 text-muted hover:text-gold-hover"
          >
            <Pencil className="h-4 w-4" aria-hidden /> Edit this page on GitHub
          </a>
        </div>

        <nav aria-label="Previous and next" className="mt-6 grid gap-4 sm:grid-cols-2">
          {prev ? (
            <Link
              to={`/docs/${prev.slug}`}
              className="group rounded-xl border border-ink-500 p-4 transition-colors hover:border-gold/50"
            >
              <span className="flex items-center gap-1.5 text-xs uppercase tracking-wider text-muted">
                <ArrowLeft className="h-3.5 w-3.5" aria-hidden /> Previous
              </span>
              <span className="mt-1 block font-display text-xl font-semibold uppercase text-bone group-hover:text-gold">
                {prev.title}
              </span>
            </Link>
          ) : (
            <span />
          )}
          {next && (
            <Link
              to={`/docs/${next.slug}`}
              className="group rounded-xl border border-ink-500 p-4 text-right transition-colors hover:border-gold/50"
            >
              <span className="flex items-center justify-end gap-1.5 text-xs uppercase tracking-wider text-muted">
                Next <ArrowRight className="h-3.5 w-3.5" aria-hidden />
              </span>
              <span className="mt-1 block font-display text-xl font-semibold uppercase text-bone group-hover:text-gold">
                {next.title}
              </span>
            </Link>
          )}
        </nav>
      </article>

      {headings.length > 1 && (
        <aside className="hidden xl:block" aria-label="On this page">
          <div className="sticky top-24">
            <p className="font-display text-xs font-semibold uppercase tracking-[0.22em] text-muted">On this page</p>
            <ul className="mt-3 space-y-2 text-sm">
              {headings.map((h) => (
                <li key={h.id}>
                  <a
                    href={`#${h.id}`}
                    className={`block leading-snug transition-colors ${active === h.id ? "text-gold" : "text-muted hover:text-bone"}`}
                  >
                    {h.text}
                  </a>
                </li>
              ))}
            </ul>
          </div>
        </aside>
      )}
    </div>
  );
}
