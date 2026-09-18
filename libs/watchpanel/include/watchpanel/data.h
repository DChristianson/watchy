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

        virtual void Update(const Model &model, long now, long deltaSeconds);

        virtual void Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds);

        void AddUpdate(Updateable *update);

        const char *GetName() { return name.c_str(); }

    };

    class ConfigData : public DataImport {
    private:

        std::string path;

    public:

        ConfigData(const char *packageName, const char *path);
        ~ConfigData();

        void Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds);

    };

    class JsonFileData : public DataImport {
    private:

        std::string path;

    public:

        JsonFileData(const char *name, const char *path);
        ~JsonFileData();

        void Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds);

    };

    class FeedData : public DataImport {
    private:

        std::string href;
        long maxAgeSeconds;
        std::string cacheDir;
        std::string format;

    public:

        FeedData(const char *name, const char *href,
                 long maxAgeSeconds = 15 * 60, const char *cacheDir = "cache",
                 const char *format = "json");
        ~FeedData();

        void SetHRef(const char *href);

        void Pull(const Model &model, rapidjson::Document &out, long now, long deltaSeconds);

    };

}

#endif // WATCHPANEL_DATA_H_
