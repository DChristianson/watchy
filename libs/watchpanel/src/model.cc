#include "model.h"

#include <string>

#include "rapidjson/pointer.h"

namespace wpp = watchpanel;

wpp::Model::~Model() {}

bool wpp::Model::Find(const char *varName, std::string &result) const { return false; }

int wpp::Model::GetArraySize(const char *path) const { return 0; }

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

int wpp::DocumentModel::GetArraySize(const char *path) const {
    rapidjson::Pointer p(path);
    if (auto search = rapidjson::GetValueByPointer(doc, p)) {
        if (search->IsArray()) {
            return static_cast<int>(search->Size());
        }
    }
    return 0;
}

wpp::ScopedModel::ScopedModel(const Model &base, const std::string &basePath)
    : base(base), basePath(basePath) {}

wpp::ScopedModel::~ScopedModel() {}

namespace {

std::string ResolvePath(const std::string &basePath, const char *varName) {
    if (varName != nullptr && varName[0] == '/') {
        return std::string(varName);
    }
    return basePath + "/" + (varName != nullptr ? varName : "");
}

}  // namespace

bool wpp::ScopedModel::Find(const char *varName, std::string &result) const {
    if (varName != nullptr && varName[0] == '/') {
        return base.Find(varName, result);
    }
    const std::string fullPath = ResolvePath(basePath, varName);
    return base.Find(fullPath.c_str(), result);
}

int wpp::ScopedModel::GetArraySize(const char *path) const {
    if (path != nullptr && path[0] == '/') {
        return base.GetArraySize(path);
    }
    const std::string fullPath = ResolvePath(basePath, path);
    return base.GetArraySize(fullPath.c_str());
}
