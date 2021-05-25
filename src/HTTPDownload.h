#ifndef HTTP_DOWNLOAD_H
#define HTTP_DOWNLOAD_H

#include "platform_debug.h"
#include "HTTPClient.h"
#include "FS.h"
#include "FFat.h"
#include "FFatHelper.h"
#include <vector>

class HTTPDownload
{
public:
    HTTPDownload(){

    }
    void init(){
      vfilelist.clear();
      vfileDownloaded.clear();
    }
    ~HTTPDownload()=default;
    bool execute(const String& url,const String& file);
private:
    uint8_t buff[1024] = { 0 };
    HTTPClient _httpClient;
    std::vector<String> vfilelist;
    std::vector<String> vfileDownloaded;
    String splitString;
    String _path;
};

#endif