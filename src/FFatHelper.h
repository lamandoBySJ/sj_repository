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

using namespace std;

class FFatHelper {


public:
    static SemaphoreHandle_t _xSemaphore;
    
    static bool init();
    static bool exists(String path);
    static bool listDir(fs::FS &fs, const char * dirname, uint8_t levels,std::unordered_set<std::string>& dir_set,std::unordered_set<std::string>& file_set);
   // static bool listDir(fs::FS &fs, const char * dirname, uint8_t levels,std::unordered_set<String>& dir_set,std::unordered_set<String>& file_set);
    static bool createDir(fs::FS &fs, const char * path);

    static bool removeDir(fs::FS &fs, const char * path);
    static bool readFile(fs::FS &fs,const  String& path,String& text);
    static bool writeFile(fs::FS &fs,const  String& path,const String& message);
    static bool appendFile(fs::FS &fs, const String& path, const String& text);
   
    static bool renameFile(fs::FS &fs, const char * path1, const char * path2);
    static bool deleteFile(fs::FS &fs, const char * path);
    
     static bool isDirectory(fs::FS &fs, const char * path);
private:
   static std::mutex _mtx;
};


#endif
