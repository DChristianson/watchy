#ifndef WATCHPANEL_DATA_H_
#define WATCHPANEL_DATA_H_

#include "strings.h"
#include "update.h"
#include "model.h"
#include "rapidjson/document.h"

namespace watchpanel {

    class DataImport {
    private:
    
        std::string name;
        std::vector<Updateable *> updateList;

    public:

        DataImport(const char *name);
        virtual ~DataImport();
        
        virtual void Update(const Model &model);

        virtual void Pull(const Model &model, rapidjson::Document &out);

        void AddUpdate(Updateable *update);

        const char *GetName() { return name.c_str(); }

    };

    class ConfigData : public DataImport {
    private:

        std::string path;
    
    public:

        ConfigData(const char *packageName, const char *path);
        ~ConfigData();
    
        void Pull(const Model &model, rapidjson::Document &out);

    };

    class FeedData : public DataImport {
    private:
        
        std::string href;

    public:
    
        FeedData(const char *name, const char *href);
        ~FeedData();
    
        void SetHRef(const char *href);
        
        void Pull(const Model &model, rapidjson::Document &out);

    };

}

#endif // WATCHPANEL_DATA_H_