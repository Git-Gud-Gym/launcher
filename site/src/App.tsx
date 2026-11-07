import { lazy, Suspense } from "react";
import { MotionConfig } from "framer-motion";
import { Route, Routes } from "react-router-dom";
import Layout from "./components/Layout";
import Home from "./pages/Home";
import Games from "./pages/Games";
import NotFound from "./pages/NotFound";

// Docs pull in the Markdown renderer, so they load on demand.
const Docs = lazy(() => import("./pages/Docs"));

export default function App() {
  return (
    <MotionConfig reducedMotion="user">
      <Routes>
        <Route element={<Layout />}>
          <Route index element={<Home />} />
          <Route path="games" element={<Games />} />
          <Route
            path="docs/:slug?"
            element={
              <Suspense fallback={<div className="container-site min-h-[60vh] py-32" aria-busy="true" />}>
                <Docs />
              </Suspense>
            }
          />
          <Route path="*" element={<NotFound />} />
        </Route>
      </Routes>
    </MotionConfig>
  );
}
