#ifndef WATCHPANEL_RSS_H_
#define WATCHPANEL_RSS_H_

#include "rapidjson/document.h"

#include <string>

namespace watchpanel {

    // Parses an RSS 2.0 file (<rss><channel>...<item>...) at `path` into a
    // JSON document shaped like:
    //   { "channel": { "title": "...", ..., "items": [ {"title": "...", ...}, ... ] } }
    // Every child element of <channel> (other than <item>, which becomes
    // an entry in "items") and of each <item> is carried over generically
    // by tag name -> text content, so this works for any RSS 2.0 feed's
    // fields (title/description/pubDate/link/etc.), not just a fixed set.
    // Returns false (leaving `out` untouched) if the file can't be read or
    // isn't a well-formed <rss><channel> document.
    bool ParseRssFile(const std::string &path, rapidjson::Document &out);

}

#endif // WATCHPANEL_RSS_H_
