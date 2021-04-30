#ifndef F_FAT_HELPER_H
#define F_FAT_HELPER_H

#include "Arduino.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "time.h"
#include <iostream>
#include<iomanip>
#include<sstream>
#include <time.h>
#include "esp_pm.h"
#include <FS.h>
#include "FFat.h"
#include <unordered_set>
#include <mutex>
#include "platform_debug.h" 

using namespace std;

class FFatHelper {


public:
    static SemaphoreHandle_t _xSemaphore;
    
    static bool init();
    static bool exists(String path);
    static bool listDir(fs::FS &fs,  const String& , uint8_t levels);
    static bool deleteFiles(fs::FS &fs,  const String& , uint8_t levels);
   // static bool listDir(fs::FS &fs, const char * dirname, uint8_t levels,std::unordered_set<String>& dir_set,std::unordered_set<String>& file_set);
    static bool createDir(fs::FS &fs, const String& );

    static bool removeDir(fs::FS &fs,  const String&  path);
    static bool readFile(fs::FS &fs,const  String& path,String& text);
    static bool writeFile(fs::FS &fs,const  String& path,const String& message);
    static bool writeFile(fs::FS &fs,const String& path,const uint8_t * text,size_t len);
    static bool appendFile(fs::FS &fs, const String& path, const String& text);
   static bool appendFile(fs::FS &fs,const String& path,const uint8_t * text,size_t len);

    static bool renameFile(fs::FS &fs, const String&  path1, const String&  path2);
    static bool deleteFile(fs::FS &fs,  const String&  path);
    
     static bool isDirectory(fs::FS &fs,  const String&  path);
private:
   static std::mutex _mtx;
};


#endif
