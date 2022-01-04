#include "update.h"
#include <iostream>

namespace wpp = watchpanel; 

wpp::Updateable::~Updateable() {}
        
wpp::UpdateFormattedString::UpdateFormattedString(const char *text, std::function<void(const char *)> setter)
    : formatter(text), setter(setter) {
}

wpp::UpdateFormattedString::~UpdateFormattedString() {}

void wpp::UpdateFormattedString::Update(const Model &lookup) {
    std::string buffer;
    formatter.Format(lookup, buffer);
    std::cout << "new value: " << buffer << std::endl;
    setter(buffer.c_str());
}