import { Download } from "lucide-react";
import { appDownload, formatBytes, type Catalog } from "../catalog";

/** Primary CTA. Points at the hub app's zip when the catalog has one, else the releases page. */
export default function DownloadButton({ catalog, className = "" }: { catalog: Catalog; className?: string }) {
  const dl = appDownload(catalog);
  const meta = catalog.app
    ? [`v${catalog.app.version}`, catalog.app.download?.size ? formatBytes(catalog.app.download.size) : ""]
        .filter(Boolean)
        .join(" · ")
    : null;

  return (
    <a href={dl.href} className={`btn-gold group ${className}`} {...(dl.direct ? { download: "" } : {})}>
      <Download className="h-5 w-5 transition-transform duration-300 group-hover:translate-y-0.5" aria-hidden />
      <span>Download for Windows</span>
      {meta && (
        <span className="ml-1 rounded-md bg-ink/15 px-1.5 py-0.5 font-sans text-xs font-semibold normal-case tracking-normal">
          {meta}
        </span>
      )}
    </a>
  );
}
