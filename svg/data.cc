#include "data.h"

#include "hamper.h"

#include <iostream>

namespace wpp = watchpanel; 

wpp::DataImport::~DataImport() {}

void wpp::DataImport::Pull(std::map<std::string, std::string> &vars) {}

wpp::BuiltinData::BuiltinData() {}

wpp::BuiltinData::~BuiltinData() {}

void wpp::BuiltinData::Pull(std::map<std::string, std::string> &vars) {
    vars["config.openweather.city"] = "Seattle";
    vars["config.openweather.state_code"] = "WA";
    vars["config.openweather.country_code"] = "US";
    vars["config.openweather.appid"] = "<add here>";
}

wpp::RemoteFetchData::RemoteFetchData(const char * src) : src(src) {}

wpp::RemoteFetchData::~RemoteFetchData() {
    Clear();
}

void wpp::RemoteFetchData::Clear() {}

void wpp::RemoteFetchData::Pull(std::map<std::string, std::string> &vars) {
    Clear();

    std::string url;
    src.Format(vars, url);
    
    std::cout << url << std::endl;

    // do fetch
    rapidjson::Document document;
    int res = hamper::fetch_url(url.c_str(), document, "tmp.cache.json");
    if (0 != res) {
        // TODO: error
    }
    
    for ( auto& m : document.GetObject() ) {
        std::cout << m.name.GetString() << std::endl;
    }
}
