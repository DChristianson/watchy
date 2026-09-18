#include "rss.h"
#include "pugixml.hpp"

#include <cstring>

namespace wpp = watchpanel;

namespace {

// Adds a JSON string member per direct child element's tag name -> text
// content (via pugixml's child_value(), which transparently unwraps CDATA).
// If a tag repeats, the last occurrence wins.
void AddFieldsFromChildren(pugi::xml_node parent, rapidjson::Value &target,
                            rapidjson::Document::AllocatorType &allocator) {
    for (pugi::xml_node child = parent.first_child(); child; child = child.next_sibling()) {
        const char *tag = child.name();
        if (tag == nullptr || tag[0] == 0) continue;
        const char *text = child.child_value();
        target.RemoveMember(tag);
        rapidjson::Value key(tag, allocator);
        rapidjson::Value value(text ? text : "", allocator);
        target.AddMember(key, value, allocator);
    }
}

}  // namespace

bool wpp::ParseRssFile(const std::string &path, rapidjson::Document &out) {
    pugi::xml_document doc;
    const pugi::xml_parse_result result = doc.load_file(path.c_str());
    if (!result) return false;

    pugi::xml_node channel = doc.child("rss").child("channel");
    if (!channel) return false;

    out.SetObject();
    rapidjson::Document::AllocatorType &allocator = out.GetAllocator();

    rapidjson::Value channelValue(rapidjson::kObjectType);
    rapidjson::Value items(rapidjson::kArrayType);

    for (pugi::xml_node child = channel.first_child(); child; child = child.next_sibling()) {
        const char *tag = child.name();
        if (tag != nullptr && std::strcmp(tag, "item") == 0) {
            rapidjson::Value itemValue(rapidjson::kObjectType);
            AddFieldsFromChildren(child, itemValue, allocator);
            items.PushBack(itemValue, allocator);
        } else if (tag != nullptr && tag[0] != 0) {
            const char *text = child.child_value();
            channelValue.RemoveMember(tag);
            rapidjson::Value key(tag, allocator);
            rapidjson::Value value(text ? text : "", allocator);
            channelValue.AddMember(key, value, allocator);
        }
    }

    channelValue.AddMember("items", items, allocator);
    out.AddMember("channel", channelValue, allocator);
    return true;
}
