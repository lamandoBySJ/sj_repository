#include "CmdParser.h"

void CmdParser::onMessageCallback(const String& topic,const String& payload)
{
    platform_debug::TracePrinter::printTrace(topic);
    platform_debug::TracePrinter::printTrace(payload);
    return;
    _topics.clear();
    _StringHelper.split(_topics,topic.c_str(),"/");
    DynamicJsonDocument  doc(payload.length()+1024);
    if(_topics.size()==2){
        if(_topics[1]=="ServerTime"){
            if (doc.containsKey("unix_timestamp")) {
                long ts =   doc["unix_timestamp"].as<long>();
                if (ts > 28800) {
                  //  timeMachine.setEpoch((time_t)ts);
                }
            }
        }else if(_topics[1]=="ServerReq"){
           
            DeserializationError error = deserializeJson(doc,payload.c_str()); 
            if (!error){
                if(doc.containsKey("event_type") ){
                    String event_type=doc["event_type"].as<String>();
                    switch( str_map_type[event_type] ){
                        case RequestType::ALS_MEASURE:
                        {

                        }break;
                        case RequestType::MANUAL_REQUEST:
                        {
                              
                              
                        }break;
                        case RequestType::OTA_UPGRADE:
                        {
                              if (doc.containsKey("url")) {
                               // if (doc.containsKey("cert_pem")) { 
                               // }
                  
                                _threadCountdown=std::thread(&CmdParser::timeoutChecker,this);
                               // _OTAService.execute(doc["url"].as<String>());
                              }
                              
                        }break;
                        case RequestType::OTA_CANCEL:
                        {
                             
                              
                        }break;
                        case RequestType::FILE_DOWNLOAD:
                        {
                            
                              
                        }break;
                        case RequestType::FILE_DELETE:
                        {
                              
                              
                        }break;
                        default:break;
                    }
                }
            }    
        }
    }
}

