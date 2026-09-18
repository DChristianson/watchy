#include "hamper.h"

#include "curl/curl.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"

#include <sys/stat.h>
#include <sys/time.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <mutex>

namespace hamper {
namespace {

std::once_flag curlInitFlag;

void EnsureCurlInitialized() {
    // curl_global_init is documented as not safe to call repeatedly/from
    // multiple threads; the original code called it on every single
    // fetch, which was both wasteful and against libcurl's own guidance.
    std::call_once(curlInitFlag, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

size_t WriteData(void *ptr, size_t size, size_t nmemb, void *stream) {
    return fwrite(ptr, size, nmemb, static_cast<FILE *>(stream));
}

// FNV-1a -- just needs to turn an arbitrary URL into a filesystem-safe,
// deterministic cache key; not used for anything security-sensitive.
std::string CacheKeyForUrl(const char *url) {
    uint64_t hash = 1469598103934665603ULL;  // FNV offset basis
    for (const char *p = url; *p; ++p) {
        hash ^= static_cast<unsigned char>(*p);
        hash *= 1099511628211ULL;  // FNV prime
    }
    char buf[17];
    std::snprintf(buf, sizeof(buf), "%016llx", static_cast<unsigned long long>(hash));
    return std::string(buf);
}

bool EnsureDirExists(const std::string &dir) {
    if (mkdir(dir.c_str(), 0755) == 0) return true;
    return errno == EEXIST;
}

// Age of a file in seconds relative to `now`, or -1 if it doesn't exist /
// can't be stat'd. `now` is caller-supplied so this is deterministic.
long FileAgeSeconds(const std::string &path, long now) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return -1;
    return now - static_cast<long>(st.st_mtime);
}

// Stamps a file's mtime with `timestamp` instead of leaving it to whatever
// the OS's real clock assigned on write -- this is what makes FileAgeSeconds
// fully deterministic under a fixed sequence of caller-supplied `now`
// values, with no dependency on real wall-clock time at all.
void SetFileTimestamp(const std::string &path, long timestamp) {
    struct timeval times[2];
    times[0].tv_sec = timestamp;
    times[0].tv_usec = 0;
    times[1] = times[0];
    utimes(path.c_str(), times);
}

bool FetchUrlToFile(const char *url, const std::string &destPath) {
    EnsureCurlInitialized();

    CURL *curl = curl_easy_init();
    if (!curl) return false;

    // Write to a temp file first, then atomically rename over the real
    // cache path -- a crash or interrupted fetch mid-download must never
    // leave a torn file in the spot a "last known good" read expects.
    const std::string tmpPath = destPath + ".tmp";
    FILE *file = fopen(tmpPath.c_str(), "wb");
    if (!file) {
        curl_easy_cleanup(curl);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    // Without this, a server that redirects (e.g. http:// -> https://,
    // common for real feeds) yields an empty body instead of the actual
    // content -- curl doesn't follow redirects unless told to.
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    const CURLcode res = curl_easy_perform(curl);
    fclose(file);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "hamper: fetch failed for " << url << ": " << curl_easy_strerror(res) << std::endl;
        std::remove(tmpPath.c_str());
        return false;
    }

    if (std::rename(tmpPath.c_str(), destPath.c_str()) != 0) {
        std::cerr << "hamper: failed to install cache file " << destPath << std::endl;
        std::remove(tmpPath.c_str());
        return false;
    }
    return true;
}

}  // namespace

std::string FetchToCache(const char *url, long now, long maxAgeSeconds, const char *cacheDir) {
    const std::string dir(cacheDir);
    if (!EnsureDirExists(dir)) {
        std::cerr << "hamper: could not create cache directory " << dir << std::endl;
        return "";
    }

    const std::string path = dir + "/" + CacheKeyForUrl(url) + ".cache";
    const long age = FileAgeSeconds(path, now);

    if (age >= 0 && age < maxAgeSeconds) {
        return path;  // fresh enough -- no network hit
    }

    if (FetchUrlToFile(url, path)) {
        SetFileTimestamp(path, now);
        return path;
    }

    // Fetch failed: fall back to whatever was last successfully fetched,
    // no matter how stale, rather than treating this as a hard failure.
    if (age >= 0) {
        std::cerr << "hamper: fetch failed, serving stale cache (age " << age << "s) for " << url
                   << std::endl;
        return path;
    }

    return "";
}

int fetch_url(const char *url, rapidjson::Document &d, long now, long maxAgeSeconds, const char *cacheDir) {
    const std::string path = FetchToCache(url, now, maxAgeSeconds, cacheDir);
    if (path.empty()) {
        d.SetObject();
        return -1;
    }

    FILE *pagefile = fopen(path.c_str(), "rb");
    if (pagefile == NULL) {
        d.SetObject();
        return -1;
    }

    char readBuffer[65536];
    rapidjson::FileReadStream is(pagefile, readBuffer, sizeof(readBuffer));
    d.ParseStream(is);
    fclose(pagefile);

    if (d.HasParseError()) {
        std::cerr << "hamper: json parse failed: " << rapidjson::GetParseError_En(d.GetParseError())
                   << std::endl;
        d.SetObject();
        return -1;
    }

    return 0;
}

std::string fetch_image(const char *url, long now, long maxAgeSeconds, const char *cacheDir) {
    return FetchToCache(url, now, maxAgeSeconds, cacheDir);
}

}  // namespace hamper
