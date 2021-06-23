#include "FFatHelper.h"
rtos::Mutex _mtx;
FFatHelper FatHelper(_mtx);


bool FFatHelper::init(){
    std::lock_guard<rtos::Mutex> lck(_mtx);
    return FFat.begin(true);
}

bool FFatHelper::exists(String path){
    std::lock_guard<rtos::Mutex> lck(_mtx);
    File file = FFat.open(path, "r");
    bool isExists=false;
    if(file.name()!=nullptr) {
        isExists=true;
    }
    file.close();
    return isExists;
}

bool FFatHelper::isDirectory(fs::FS &fs, const String& path){
    std::lock_guard<rtos::Mutex> lck(_mtx);
    bool isExists=false;
    File file = fs.open(path);
    if(file.isDirectory()){
          isExists=true;
    }
    file.close();
    return isExists;
}

bool FFatHelper::readFile(fs::FS &fs,const String& path,String& text){

   std::lock_guard<rtos::Mutex> lck(_mtx);
    char c[2]={0};
    int max =5000;
    bool success=false;
    File file = fs.open(path);
    if(file.name()!=nullptr && strlen(file.name())>0){
     
        if(!file.isDirectory()){
            text="";
            while(file.available() && --max>0){
                c[0]=file.read();
                text+=c;
                success=true;
            }
        }
        file.close();
        PlatformDebug::println(String("R:OK"));     
    }
           
    return success;
}

bool FFatHelper::writeFile(fs::FS &fs,const String& path,const String& text){
   std::lock_guard<rtos::Mutex> lck(_mtx);
   File file = fs.open(path, FILE_WRITE);
   PlatformDebug::println("file.name:"+String(file.name()));
    if(file.name()!=nullptr){
        if(file.write((const uint8_t*)text.c_str(),text.length()) == text.length()){
            file.close();
            PlatformDebug::println(String("W:OK"));
            return true;
        }
    }
    PlatformDebug::println(String("W:ERROR"));
  return false;  
}

bool FFatHelper::writeFile(fs::FS &fs,const String& path,const uint8_t * text,size_t len){
   std::lock_guard<rtos::Mutex> lck(_mtx);
    File file = fs.open(path, FILE_WRITE);
    if(file.name()!=nullptr){
        if(file.write(text,len) == len){
            file.close();
            PlatformDebug::println(String("W:OK"));
            return true;
        }
    }
    PlatformDebug::println(String("W:ERROR"));
  return false;  
}

bool FFatHelper::appendFile(fs::FS &fs, const String& path, const String& text){
   std::lock_guard<rtos::Mutex> lck(_mtx);
    File file = fs.open(path, FILE_APPEND);
    if(file.name()!=nullptr){
        if(file.write((const uint8_t *)text.c_str(),text.length()) == text.length()){
            file.close();
            return true;
        }
    }
    return false;
}

bool FFatHelper::appendFile(fs::FS &fs,const String& path,const uint8_t * text,size_t len){
   std::lock_guard<rtos::Mutex> lck(_mtx);
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
   std::lock_guard<rtos::Mutex> lck(_mtx);
      if (fs.rename(path1, path2)) {
            return true;
      }
  return false;
}

bool FFatHelper::deleteFile(fs::FS &fs, const String& path){
   std::lock_guard<rtos::Mutex> lck(_mtx);
    if(fs.remove(path)){
        return true;
    }
  return false;
}


bool FFatHelper::listDir(fs::FS &fs, const String& dirname, uint8_t levels){
    std::lock_guard<rtos::Mutex> lck(_mtx);

    File root = fs.open(dirname);
    if(root.name()==nullptr){
        return false;
    }
    File file = root.openNextFile();
    while(file.name()){
        if(file.isDirectory()){
            PlatformDebug::println("-List DiR:"+String(file.name()));
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            PlatformDebug::println("--List FILE:"+String(file.name()));
        }
        file = root.openNextFile();
    }
    file.close();

   return true;
}

bool FFatHelper::deleteFiles(fs::FS &fs, const String& dirname, uint8_t levels){
   

    File root = fs.open(dirname);
    if(root.name()==nullptr){
        return false;
    }
    File file = root.openNextFile();
    while(file.name()){
        if(file.isDirectory()){
            PlatformDebug::println("-List DiR:"+String(file.name()));
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            PlatformDebug::println("--List FILE:"+String(file.name()));
            deleteFile(fs,file.name());
        }
        file = root.openNextFile();
    }
    file.close();

   return true;
}

bool FFatHelper::createDir(fs::FS &fs, const String&  path){
   std::lock_guard<rtos::Mutex> lck(_mtx);
    if(fs.mkdir(path)){
        return true;
    }
   return false;
}

bool FFatHelper::removeDir(fs::FS &fs,  const String&  path){
   std::lock_guard<rtos::Mutex> lck(_mtx);
    if(fs.rmdir(path)){
        return true;
    }
   return false;
}
