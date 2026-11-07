# Security policy

Git Gud Gym runs with administrator rights and loads code into game processes, so we take its
supply chain seriously.

## How downloads are protected

- **HTTPS only:** the app only downloads over HTTPS, from this repository's GitHub releases.
- **Checksums:** every module file has a SHA-256 in `catalog.json`. The app refuses any file whose
  checksum doesn't match, and nothing is installed until every file has been verified.
- **Built in CI:** releases are built by GitHub Actions from tagged commits, not uploaded by hand.

## Reporting a vulnerability

Please **don't** open a public issue. Use GitHub's
[private vulnerability reporting](../../security/advisories/new) instead, with steps to reproduce.
We'll acknowledge it within a few days and coordinate a fix and disclosure with you.

## Supported versions

Only the latest `app-v*` release and the latest release of each game module get fixes.
