#include "data.h"

#include "hamper.h"
#include "rapidjson/pointer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"

#include <iostream>
#include <ctime>

namespace wpp = watchpanel; 

wpp::DataImport::DataImport(const char *name) : name(name) {}

wpp::DataImport::~DataImport() {
    for (auto u : updateList)
    {
        delete u;
    }
    updateList.clear();
}

void wpp::DataImport::AddUpdate(Updateable *update) {
    updateList.push_back(update);
}

void wpp::DataImport::Update(const Model &model, long now, long deltaSeconds) {
    for (auto u : updateList)
    {
        u->Update(model, now, deltaSeconds);
    }
}

void wpp::DataImport::Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds) {}

wpp::ConfigData::ConfigData(const char *name, const char *path) : DataImport(name), path(path) {}

wpp::ConfigData::~ConfigData() {}

void wpp::ConfigData::Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds) {
    // TODO: cache everything
    std::cout << "Loading " << path.c_str() << std::endl;
    auto pagefile = fopen(path.c_str(), "rb");
    if (pagefile == NULL) {
        out.SetObject();
        std::cerr << "Warning: failed to open " << path.c_str() << std::endl;
        return;
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(pagefile, readBuffer, sizeof(readBuffer));
    out.ParseStream(is);
    fclose(pagefile);
    if (out.HasParseError()) {
        std::cerr << "Warning: failed to parse " << path.c_str() << ": "
                  << rapidjson::GetParseError_En(out.GetParseError()) << std::endl;
        out.SetObject();
    }
}

wpp::JsonFileData::JsonFileData(const char *name, const char *path) : DataImport(name), path(path) {}

wpp::JsonFileData::~JsonFileData() {}

void wpp::JsonFileData::Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds) {
    std::cout << "Loading " << path.c_str() << std::endl;
    auto pagefile = fopen(path.c_str(), "rb");
    if (pagefile == NULL) {
        out.SetObject();
        std::cerr << "Warning: failed to open " << path.c_str() << std::endl;
        return;
    }
    char readBuffer[65536];
    rapidjson::FileReadStream is(pagefile, readBuffer, sizeof(readBuffer));
    out.ParseStream(is);
    fclose(pagefile);
    if (out.HasParseError()) {
        std::cerr << "Warning: failed to parse " << path.c_str() << ": "
                  << rapidjson::GetParseError_En(out.GetParseError()) << std::endl;
        out.SetObject();
    }
}

wpp::FeedData::FeedData(const char *name, const char *href, long maxAgeSeconds, const char *cacheDir)
    : DataImport(name), href(href), maxAgeSeconds(maxAgeSeconds), cacheDir(cacheDir) {}

wpp::FeedData::~FeedData() {}

void wpp::FeedData::SetHRef(const char *value) {
    href = value;
}

void wpp::FeedData::Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds) {
    (void)deltaSeconds;
    std::cout << "Fetching " << href.c_str() << std::endl;
    int res = hamper::fetch_url(href.c_str(), out, now, maxAgeSeconds, cacheDir.c_str());
    if (0 != res) {
        std::cerr << "Warning: fetch failed for " << href.c_str() << " with no cached data to fall back to" << std::endl;
        out.SetObject();
    }
}
