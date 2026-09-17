// Test for hamper's local cache: fresh-fetch, cache-hit avoiding a
// re-fetch, TTL expiry triggering a refresh, and -- the important part --
// a failed refresh falling back to the last successfully fetched copy
// instead of failing outright. All of this runs against file:// URLs in a
// throwaway scratch directory, so no network and no shared state with any
// other test or the real cache/ directory.
//
// Run via: ctest --test-dir build

#include "hamper.h"

#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

int failures = 0;

void Check(bool condition, const char *what) {
    if (!condition) {
        std::cerr << "FAILED: " << what << std::endl;
        ++failures;
    }
}

std::string ReadFile(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

void WriteFile(const std::string &path, const std::string &content) {
    std::ofstream out(path, std::ios::binary);
    out << content;
}

std::string ResolveAbsolute(const std::string &path) {
    char resolved[4096];
    if (realpath(path.c_str(), resolved) == nullptr) return "";
    return std::string(resolved);
}

}  // namespace

int main() {
    // Everything lives under one throwaway directory so cleanup is simple
    // and nothing collides with other tests or the real cache/.
    char scratchTemplate[] = "/tmp/watchy_hamper_test_XXXXXX";
    const char *scratchDir = mkdtemp(scratchTemplate);
    Check(scratchDir != nullptr, "created scratch directory");
    if (!scratchDir) {
        std::cerr << failures << " check(s) failed" << std::endl;
        return 1;
    }

    const std::string sourcePath = std::string(scratchDir) + "/source.json";
    const std::string cacheDir = std::string(scratchDir) + "/cache";

    WriteFile(sourcePath, "source-v1");
    const std::string sourceUrl = "file://" + ResolveAbsolute(sourcePath);

    // 1. No cache entry yet: must fetch, regardless of maxAge.
    const std::string path1 = hamper::FetchToCache(sourceUrl.c_str(), 0, cacheDir.c_str());
    Check(!path1.empty(), "first fetch (no cache yet) succeeds");
    Check(ReadFile(path1) == "source-v1", "first fetch returns the real source content");

    // 2. Source changes, but a large maxAge means the still-fresh cache
    // entry from step 1 should be served without re-fetching.
    WriteFile(sourcePath, "source-v2");
    const std::string path2 = hamper::FetchToCache(sourceUrl.c_str(), 9999, cacheDir.c_str());
    Check(path2 == path1, "cache hit reuses the same cache file");
    Check(ReadFile(path2) == "source-v1", "cache hit serves the old content, doesn't re-fetch");

    // 3. maxAge=0 forces the cache to be considered stale: this should
    // actually re-fetch and pick up the new content.
    const std::string path3 = hamper::FetchToCache(sourceUrl.c_str(), 0, cacheDir.c_str());
    Check(ReadFile(path3) == "source-v2", "expired cache triggers a real refresh");

    // 4. THE IMPORTANT PART: point at a URL that will fail (nonexistent
    // local file), forcing maxAge=0 so a refresh is attempted. A cache
    // entry already exists (from step 3) -- it must be returned as-is
    // rather than treated as a failure, no matter how the fetch went.
    const std::string missingUrl = "file://" + std::string(scratchDir) + "/does_not_exist.json";
    // Reuse the same cache slot by fetching missingUrl into path3's cache
    // dir isn't meaningful (different URL = different cache key), so
    // instead simulate "the same URL now fails" by removing the source
    // file out from under a fetch of the URL we've already cached.
    std::remove(sourcePath.c_str());
    const std::string path4 = hamper::FetchToCache(sourceUrl.c_str(), 0, cacheDir.c_str());
    Check(!path4.empty(), "failed refresh still returns a path (falls back to cache)");
    Check(ReadFile(path4) == "source-v2", "failed refresh serves the last known good content");

    // 5. A URL that has NEVER been fetched successfully, with nothing to
    // fall back to, must fail outright (empty string).
    const std::string path5 = hamper::FetchToCache(missingUrl.c_str(), 0, cacheDir.c_str());
    Check(path5.empty(), "no cache entry and a failed fetch returns empty, not a fallback");

    // Cleanup.
    std::remove(path1.c_str());
    std::remove((path1 + ".tmp").c_str());
    rmdir(cacheDir.c_str());
    std::remove(sourcePath.c_str());  // already removed above; harmless if missing
    rmdir(scratchDir);

    if (failures == 0) {
        std::cout << "OK (hamper cache checks passed)" << std::endl;
        return 0;
    }
    std::cerr << failures << " check(s) failed" << std::endl;
    return 1;
}
