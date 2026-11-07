#include "catalog.h"

#include "json.hpp"

using nlohmann::json;

namespace {

std::string Str(const json& j, const char* key) {
    auto it = j.find(key);
    return it != j.end() && it->is_string() ? it->get<std::string>() : std::string();
}

}  // namespace

std::optional<Catalog> ParseCatalog(const std::string& text, std::string& error) {
    try {
        const json j = json::parse(text);
        if (j.value("schema", 0) != 1) {
            error = "Unsupported catalog version; update Git Gud Gym";
            return std::nullopt;
        }
        Catalog c;
        c.repo = Str(j, "repo");
        if (auto app = j.find("app"); app != j.end() && app->is_object()) {
            c.appVersion = Str(*app, "version");
            c.appReleaseUrl = Str(*app, "releaseUrl");
        }
        for (const json& m : j.at("modules")) {
            CatalogModule cm;
            cm.id = Str(m, "id");
            cm.name = Str(m, "name");
            cm.status = Str(m, "status");
            cm.tagline = Str(m, "tagline");
            cm.version = Str(m, "version");
            cm.releaseUrl = Str(m, "releaseUrl");
            if (auto f = m.find("features"); f != m.end() && f->is_array()) {
                for (const json& s : *f) {
                    if (s.is_string()) cm.features.push_back(s.get<std::string>());
                }
            }
            if (auto files = m.find("files"); files != m.end() && files->is_array()) {
                for (const json& f : *files) {
                    cm.files.push_back({Str(f, "name"), Str(f, "url"), Str(f, "sha256"), f.value("size", 0ull)});
                }
            }
            if (!cm.id.empty() && !cm.name.empty()) c.modules.push_back(std::move(cm));
        }
        return c;
    } catch (const std::exception& e) {
        error = std::string("Couldn't read the game library: ") + e.what();
        return std::nullopt;
    }
}
