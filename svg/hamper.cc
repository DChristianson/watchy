#include "hamper.h"

#include "curl/curl.h"
#include "rapidjson/filereadstream.h"

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

int hamper::fetch_url(const char *url, rapidjson::Document &d, const char *cache_file_name) {

    curl_global_init(CURL_GLOBAL_DEFAULT);
 
    CURL *curl = curl_easy_init();
    if ( NULL == curl) {
        // TODO: push error
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

    FILE * pagefile = fopen(cache_file_name, "wb");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);

    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        // TODO: push error
        // errors.push_back(curl_easy_strerror(res));
    }

    fclose(pagefile);

    curl_easy_cleanup(curl);

    curl_global_cleanup();

    // parse phase
    
    pagefile = fopen(cache_file_name, "rb");

    char readBuffer[65536];
    rapidjson::FileReadStream is(pagefile, readBuffer, sizeof(readBuffer));
    
    d.ParseStream(is);
    
    fclose(pagefile);

    return 0;

}