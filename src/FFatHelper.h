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

template<typename OSMutex>
class FFatHelper {
public:
   FFatHelper()=delete;
   FFatHelper(OSMutex &mutex):_mtx(mutex)
   {

   }
   ~FFatHelper()=default;
   bool init();
   bool exists(String path);
   bool listDir(fs::FS &fs,  const String& , uint8_t levels);
   bool deleteFiles(fs::FS &fs,  const String& , uint8_t levels);
   //bool listDir(fs::FS &fs, const char * dirname, uint8_t levels,std::unordered_set<String>& dir_set,std::unordered_set<String>& file_set);
   bool createDir(fs::FS &fs, const String& );

   bool removeDir(fs::FS &fs,  const String&  path);
   bool readFile(fs::FS &fs,const  String& path,String& text);
   bool writeFile(fs::FS &fs,const  String& path,const String& message);
   bool writeFile(fs::FS &fs,const String& path,const uint8_t * text,size_t len);
   bool appendFile(fs::FS &fs, const String& path, const String& text);
   bool appendFile(fs::FS &fs,const String& path,const uint8_t * text,size_t len);

   bool renameFile(fs::FS &fs, const String&  path1, const String&  path2);
   bool deleteFile(fs::FS &fs,  const String&  path);
    
   bool isDirectory(fs::FS &fs,  const String&  path);
   

private:
   OSMutex &_mtx;
};

extern FFatHelper<rtos::Mutex> FatHelper;

#endif
