#include "HTTPDownload.h"

extern FFatHelper<rtos::Mutex> FatHelper;

bool HTTPDownload::execute(const String& url,const String& file)
    {
      _path =  String("/")+file; 
      _httpClient.begin(url);
      int httpCode = _httpClient.GET();
       
      if((httpCode != HTTP_CODE_OK)){
        return false;
      }
             
      bool success=true;
      if(httpCode > 0 && httpCode == HTTP_CODE_OK) {
          int len = _httpClient.getSize();
          int buff_len = sizeof(buff);
          // get tcp stream
          WiFiClient* stream = _httpClient.getStreamPtr();
          FatHelper.writeFile(FFat,_path,"");   
          // read all data from server
          while( _httpClient.connected() && (len > 0 || len == -1)) {
              // get available data size
              size_t size = stream->available();
              TracePrinter::printf("[HTTP] size : %d, len: %d\n",size,len);
              if(size>0) {
                // read up to 128 byte
                if(size <= buff_len){
                    buff_len=size;
                }else{
                    buff_len=sizeof(buff);
                }
                int c = stream->readBytes(buff, buff_len);
                if(len > 0) {
                    len -= c;
                }
                if(c == buff_len){
                    FatHelper.appendFile(FFat,_path,buff,c); 
                }else{
                    success=false;
                break;
              }
          }
        }
      } 
      if(success==true){
         vfileDownloaded.push_back(file);
      }
      _httpClient.end();
      return success;
    }