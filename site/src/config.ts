/**
 * The single place that knows where the project lives on GitHub.
 * When the repo moves (e.g. to an organization), change REPO here and nowhere else.
 */
export const REPO = "ryyanrashid01/git-gud-gym";

export const GITHUB_URL = `https://github.com/${REPO}`;
export const RELEASES_URL = `${GITHUB_URL}/releases`;
export const LATEST_RELEASE_URL = `${RELEASES_URL}/latest`;
export const ISSUES_URL = `${GITHUB_URL}/issues`;
export const NEW_ISSUE_URL = `${ISSUES_URL}/new`;
export const DISCUSSIONS_URL = `${GITHUB_URL}/discussions`;
export const CONTRIBUTING_URL = `${GITHUB_URL}/blob/main/CONTRIBUTING.md`;
export const CHANGELOG_URL = `${GITHUB_URL}/blob/main/CHANGELOG.md`;
export const LICENSE_URL = `${GITHUB_URL}/blob/main/LICENSE`;
export const README_URL = `${GITHUB_URL}#readme`;
export const API_REPO_URL = `https://api.github.com/repos/${REPO}`;

/** URL of a file served from the site's public/ folder, respecting the deploy base path. */
export function asset(path: string): string {
  return `${import.meta.env.BASE_URL}${path.replace(/^\//, "")}`;
}

/** Values that Markdown docs can reference as {{NAME}}. */
export const DOC_VARS: Record<string, string> = {
  REPO,
  GITHUB_URL,
  RELEASES_URL,
  LATEST_RELEASE_URL,
  ISSUES_URL,
  NEW_ISSUE_URL,
  DISCUSSIONS_URL,
  CONTRIBUTING_URL,
  CHANGELOG_URL,
  LICENSE_URL,
};
