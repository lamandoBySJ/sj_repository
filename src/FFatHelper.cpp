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
bool FFatHelper::isDirectory(fs::FS &fs, const char * path){
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
 
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    File file = fs.open(path, FILE_WRITE);
    if(file.name()!=nullptr){
        if(file.write((const uint8_t*)text.c_str(),text.length()) == text.length()){
            file.close();
            return true;
        }
    }
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

bool FFatHelper::renameFile(fs::FS &fs, const char * path1, const char * path2){
   std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
      if (fs.rename(path1, path2)) {
            return true;
      }
  return false;
}

bool FFatHelper::deleteFile(fs::FS &fs, const char * path){
   std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    if(fs.remove(path)){
        return true;
    }
  return false;
}


bool FFatHelper::listDir(fs::FS &fs, const char * dirname, uint8_t levels,std::unordered_set<std::string>& dir_set,std::unordered_set<std::string>& file_set){
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();

    File root = fs.open(dirname);
    if(root.name()==nullptr){
        return false;
    }
    File file = root.openNextFile();
    while(file.name()){
        if(file.isDirectory()){
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
        }
        file = root.openNextFile();
    }
    file.close();

   return true;
}

bool FFatHelper::createDir(fs::FS &fs, const char * path){
    std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    if(fs.mkdir(path)){
        return true;
    }
   return false;
}

bool FFatHelper::removeDir(fs::FS &fs, const char * path){
   std::unique_lock<std::mutex> _mutex(_mtx, std::defer_lock);
    _mutex.lock();
    if(fs.rmdir(path)){
        return true;
    }
   return false;
}