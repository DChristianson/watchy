#include "data.h"

namespace wpp = watchpanel; 

wpp::DataImport::~DataImport() {}

void wpp::DataImport::Pull(std::map<std::string, std::string> &vars) {}

wpp::BuiltinData::BuiltinData() {}

wpp::BuiltinData::~BuiltinData() {}

void wpp::BuiltinData::Pull(std::map<std::string, std::string> &vars) {
    vars["hh"] = "10";
    vars["MM"] = "11";
    vars["city"] = "Seattle";
    vars["icon"] = "iii";
    vars["temp"] = "It's cold";
    vars["temp_hi"] = "30";
    vars["temp_low"] = "18";
}

wpp::RemoteFetchData::RemoteFetchData() {}

wpp::RemoteFetchData::~RemoteFetchData() {}

void wpp::RemoteFetchData::Pull(std::map<std::string, std::string> &vars) {}
