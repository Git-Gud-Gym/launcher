#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// The game library published with the website (catalog.json), generated from GitHub releases.
struct CatalogFile {
    std::string name, url, sha256;
    uint64_t size = 0;
};

struct CatalogModule {
    std::string id, name, status, tagline;  // status: "available" or "planned"
    std::string version, releaseUrl;         // empty when nothing is published yet
    std::vector<std::string> features;
    std::vector<CatalogFile> files;
};

struct Catalog {
    std::string repo;  // "owner/name" on GitHub
    std::string appVersion, appReleaseUrl;  // empty when the app has no release yet
    std::vector<CatalogModule> modules;
};

std::optional<Catalog> ParseCatalog(const std::string& json, std::string& error);
