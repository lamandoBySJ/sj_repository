#include "FFatHelper.h"

std::mutex FFatHelper::_mtx;
bool FFatHelper::init(){
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    return FFat.begin(true);
}
bool FFatHelper::exists(String path){

    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    File file = FFat.open(path, "r");
    bool isExists=false;
    if(file.name()!=nullptr) {
        isExists=true;
    }
    file.close();
    return isExists;
}
<<<<<<< HEAD
bool FFatHelper::isDirectory(fs::FS &fs, const String& path){
=======
bool FFatHelper::isDirectory(fs::FS &fs, const char * path){
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
   std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
     bool isExists=false;
    File file = fs.open(path);
    if(file.isDirectory()){
          isExists=true;
    }
    file.close();
    return isExists;
}
bool FFatHelper::readFile(fs::FS &fs,const String& path,String& text){
<<<<<<< HEAD

    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    char c[2]={0};
    int max =500;
    bool success=false;
    File file = fs.open(path);
    if(file.name()!=nullptr ){
        if(!file.isDirectory()){
            text="";
            while(file.available() && --max>0){
                c[0]=file.read();
                text+=c;
                success=true;
            }
        }
        file.close();
        platform_debug::PlatformDebug::println(String("R:OK"));     
    }
           
    return success;
}

bool FFatHelper::writeFile(fs::FS &fs,const String& path,const String& text){
=======
    bool success=true;
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    char c[2]={0};
    int max =1000;
    File file = fs.open(path);
    if(file.name() || file.isDirectory()){
        success=false;
    }else{
        text="";
        while(file.available() && --max>0){
            c[0]=file.read();
            text+=c;
            //Serial.write(text);
         }
    }
    file.close();             
    return success;
}

bool FFatHelper::writeFile(fs::FS &fs,const String& path,const  String& text){
 
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    File file = fs.open(path, FILE_WRITE);
    if(file.name()!=nullptr){
        if(file.write((const uint8_t*)text.c_str(),text.length()) == text.length()){
            file.close();
<<<<<<< HEAD
            platform_debug::PlatformDebug::println(String("W:OK"));
            return true;
        }
    }
    platform_debug::PlatformDebug::println(String("W:ERROR"));
  return false;  
}

bool FFatHelper::writeFile(fs::FS &fs,const String& path,const uint8_t * text,size_t len){
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    File file = fs.open(path, FILE_WRITE);
    if(file.name()!=nullptr){
        if(file.write(text,len) == len){
            file.close();
            platform_debug::PlatformDebug::println(String("W:OK"));
            return true;
        }
    }
    platform_debug::PlatformDebug::println(String("W:ERROR"));
=======
            return true;
        }
    }
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
  return false;  
}

bool FFatHelper::appendFile(fs::FS &fs, const String& path, const String& text){
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    File file = fs.open(path, FILE_APPEND);
    if(file.name()!=nullptr){
        if(file.write((const uint8_t *)text.c_str(),text.length()) == text.length()){
            file.close();
            return true;
        }
    }
    return false;
}
<<<<<<< HEAD
bool FFatHelper::appendFile(fs::FS &fs,const String& path,const uint8_t * text,size_t len){
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    File file = fs.open(path, FILE_APPEND);
    if(file.name()!=nullptr){
        if(file.write(text,len) == len){
            file.close();
            return true;
        }
    }
    return false;
}
bool FFatHelper::renameFile(fs::FS &fs, const String& path1, const String& path2){
=======

bool FFatHelper::renameFile(fs::FS &fs, const char * path1, const char * path2){
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
   std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
      if (fs.rename(path1, path2)) {
            return true;
      }
  return false;
}

<<<<<<< HEAD
bool FFatHelper::deleteFile(fs::FS &fs, const String& path){
=======
bool FFatHelper::deleteFile(fs::FS &fs, const char * path){
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
   std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    if(fs.remove(path)){
        return true;
    }
  return false;
}


<<<<<<< HEAD
bool FFatHelper::listDir(fs::FS &fs, const String& dirname, uint8_t levels){
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();

    File root = fs.open(dirname);
    if(root.name()==nullptr){
        return false;
    }
    File file = root.openNextFile();
    while(file.name()){
        if(file.isDirectory()){
            platform_debug::PlatformDebug::println("-List DiR:"+String(file.name()));
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            platform_debug::PlatformDebug::println("--List FILE:"+String(file.name()));
        }
        file = root.openNextFile();
    }
    file.close();

   return true;
}

bool FFatHelper::deleteFiles(fs::FS &fs, const String& dirname, uint8_t levels){
=======
bool FFatHelper::listDir(fs::FS &fs, const char * dirname, uint8_t levels,std::unordered_set<std::string>& dir_set,std::unordered_set<std::string>& file_set){
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();

    File root = fs.open(dirname);
    if(root.name()==nullptr){
        return false;
    }
    File file = root.openNextFile();
    while(file.name()){
        if(file.isDirectory()){
<<<<<<< HEAD
            platform_debug::PlatformDebug::println("-List DiR:"+String(file.name()));
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            platform_debug::PlatformDebug::println("--List FILE:"+String(file.name()));
            _mutex.unlock();
            FFatHelper::deleteFile(fs,file.name());
=======
            std::string s(file.name());
            dir_set.insert(s);
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            if(levels){
                listDir(fs, file.name(), levels -1,dir_set,file_set);
            }
        } else {
            std::string s(file.name());
            file_set.insert(s);
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
        }
        file = root.openNextFile();
    }
    file.close();

   return true;
}

<<<<<<< HEAD
bool FFatHelper::createDir(fs::FS &fs, const String&  path){
=======
bool FFatHelper::createDir(fs::FS &fs, const char * path){
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    if(fs.mkdir(path)){
        return true;
    }
   return false;
}

<<<<<<< HEAD
bool FFatHelper::removeDir(fs::FS &fs,  const String&  path){
=======
bool FFatHelper::removeDir(fs::FS &fs, const char * path){
>>>>>>> 4c551e600b8717be79f4d54f68f8a14634d658f8
   std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    if(fs.rmdir(path)){
        return true;
    }
   return false;
}