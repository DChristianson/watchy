#include "hamper.h"

#include "curl/curl.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/error/en.h"

#include <iostream>

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
    if (pagefile == NULL) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);

    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::cerr << "curl fetch failed: " << curl_easy_strerror(res) << std::endl;
        fclose(pagefile);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return -1;
    }

    fclose(pagefile);

    curl_easy_cleanup(curl);

    curl_global_cleanup();

    // parse phase
    
    pagefile = fopen(cache_file_name, "rb");
    if (pagefile == NULL) {
        return -1;
    }

    char readBuffer[65536];
    rapidjson::FileReadStream is(pagefile, readBuffer, sizeof(readBuffer));
    
    d.ParseStream(is);
    if (d.HasParseError()) {
        std::cerr << "json parse failed: "
                  << rapidjson::GetParseError_En(d.GetParseError()) << std::endl;
        fclose(pagefile);
        return -1;
    }
    
    fclose(pagefile);

    return 0;

}