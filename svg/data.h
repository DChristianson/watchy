#ifndef WATCHPANEL_DATA_H_
#define WATCHPANEL_DATA_H_

#include "strings.h"

#include <map>

namespace watchpanel {

    class DataImport {
    public:

        virtual ~DataImport();
        
        virtual void Pull(std::map<std::string, std::string> &vars);

    };

    class BuiltinData : public DataImport {
    public:

        BuiltinData();
        ~BuiltinData();
    
        void Pull(std::map<std::string, std::string> &vars);

    };

    class RemoteFetchData : public DataImport {
    private:
        
        FormattedString src;

        void Clear();

    public:
    
        RemoteFetchData(const char *src);
        ~RemoteFetchData();
    
        void Pull(std::map<std::string, std::string> &vars);

    };

}

#endif // WATCHPANEL_DATA_H_