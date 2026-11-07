import { Link } from "react-router-dom";
import { useDocumentTitle } from "../lib/useDocumentTitle";

export default function NotFound() {
  useDocumentTitle("Not found");
  return (
    <div className="container-site flex min-h-[60vh] flex-col items-center justify-center py-24 text-center">
      <p className="font-display text-sm font-semibold uppercase tracking-[0.3em] text-blood">404</p>
      <h1 className="mt-4 font-display text-6xl font-bold uppercase tracking-tight text-bone sm:text-8xl">You died</h1>
      <p className="mt-4 max-w-md text-muted">
        This page doesn't exist. Fortunately, nothing here is permanent. Your HP has been refilled.
      </p>
      <div className="mt-8 flex flex-col gap-3 sm:flex-row">
        <Link to="/" className="btn-gold">
          Back to the bonfire
        </Link>
        <Link to="/docs/getting-started" className="btn-ghost">
          Docs
        </Link>
      </div>
    </div>
  );
}
