#ifndef HAMPER_H_
#define HAMPER_H_

#include "rapidjson/document.h"

#include <string>

namespace hamper {

    // Fetches `url`'s content into a local cache file under `cacheDir`,
    // reusing an existing entry if it's younger than `maxAgeSeconds`
    // instead of hitting the network again.
    //
    // If a fresh fetch is attempted and fails, an existing cache entry
    // (no matter how stale) is still returned rather than treated as a
    // failure: data is only ever replaced by a successful fetch, never
    // discarded just for being old. Returns an empty string only when
    // there's no cache entry to fall back to and the fetch itself failed.
    std::string FetchToCache(const char *url, long maxAgeSeconds, const char *cacheDir);

    // Fetches a JSON document, backed by FetchToCache. Default max age:
    // 15 minutes. Returns 0 on success (served from cache or freshly
    // fetched), non-zero if there was nothing usable at all.
    int fetch_url(const char *url, rapidjson::Document &document,
                  long maxAgeSeconds = 15 * 60, const char *cacheDir = "cache");

    // Fetches an image (any format watchpanel::DecodeImage understands),
    // backed by the same cache. Default max age: 24 hours. Returns the
    // local file path for the caller to decode, or an empty string on
    // total failure.
    std::string fetch_image(const char *url,
                             long maxAgeSeconds = 24 * 60 * 60,
                             const char *cacheDir = "cache");

}

#endif // HAMPER_H_
