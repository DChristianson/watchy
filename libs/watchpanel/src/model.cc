#include "model.h"

#include <string>

#include "rapidjson/pointer.h"

namespace wpp = watchpanel; 

wpp::Model::~Model() {}

bool wpp::Model::Find(const char *varName, std::string &result) const { return false; }

wpp::DocumentModel::DocumentModel(const rapidjson::Document &doc) : doc(doc) {}

wpp::DocumentModel::~DocumentModel() {}

bool wpp::DocumentModel::Find(const char *varName, std::string &result) const {
    // TODO: should cache pointers
    rapidjson::Pointer p(varName);
    // TODO: be more flexible about where data is
    if (auto search = rapidjson::GetValueByPointer(doc, p)) {
        if (search->IsString()) {
            result = search->GetString();
            return true;
        } else if (search->IsNumber()) {
            if (search->IsDouble()) {
                result = std::to_string(search->GetDouble());
            } else {
                result = std::to_string(search->GetInt());
            }
            return true;
        } else if (search->IsBool()) {
            result = search->GetBool() ? "true" : "false";
            return true;
        } else {
            // TODO: debug value not serialized
        }
    } else {
        // TODO: debug value not found
    }
    return false;
}