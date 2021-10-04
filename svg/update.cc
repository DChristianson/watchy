#include "update.h"

namespace wpp = watchpanel; 

wpp::Updateable::~Updateable() {}
        
wpp::UpdateText::UpdateText(const char *text, wpp::TextGraphic &graphic)
    : formatter(text), graphic(graphic) {
}

wpp::UpdateText::~UpdateText() {

}

void wpp::UpdateText::Update(const std::map<std::string, std::string> &vars) {
    std::string buffer;
    formatter.Format(vars, buffer);
    graphic.SetValue(buffer.c_str());
};