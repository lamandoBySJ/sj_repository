#include "ColorCollector.h"


extern FFatHelper<rtos::Mutex> FatHelper;
void ColorCollector::delegateMethodPostMail(MeasEventType measEventType,AsyncWebSocketClient *client)
{   
    collector::mail_ws_t *mail = _mail_box_collection.alloc();
    if(mail){
        mail->client = client;
        mail->eventType = measEventType;
        _mail_box_collection.put(mail);
    }
}
void  ColorCollector::startup()
{   
    osStatus  status = _thread.start(callback(this,&ColorCollector::run_task_collection));
    (status!=osOK ? throw os::thread_error((osStatus_t)status,_thread.get_name()):NULL);
}

void ColorCollector::run_task_collection()
{
   
    ColorSensor<BH1749NUC,rtos::Mutex>::getColorSensor()->init();
    ColorSensor<BH1749NUC,rtos::Mutex>::getColorSensor()->attachMeasurementHook(std::bind(&ColorCollector::runCallbackWebSocketClientPostEvent,this,
      std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    TracePrinter::printTrace("----------------- ColorCollector -----------------");
    while(true){
        // ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(30));
        osEvent evt =  _mail_box_collection.get();
        if (evt.status == osEventMail) {
            collector::mail_ws_t *mail = (collector::mail_ws_t *)evt.value.p;
        
            ColorSensor<BH1749NUC,rtos::Mutex>::getColorSensor()->loopMeasure(_rgb,Kernel::Clock::duration_milliseconds(200),arrRGB);
            _rgb.R.u16bit =  arrRGB[4].R.u16bit;
            _rgb.G.u16bit =  arrRGB[4].G.u16bit;
            _rgb.B.u16bit =  arrRGB[4].B.u16bit;
            TracePrinter::printTrace(String(_rgb.R.u16bit,DEC)+": "+
                                                              String(_rgb.G.u16bit,DEC)+": "+
                                                              String(_rgb.B.u16bit,DEC)+": "+
                                                              String(_rgb.IR.u16bit,DEC) ); 
           
            doc.clear();
            JsonArray data = doc.createNestedArray("TowerColor");
            ColorConverter::getColorConverter().color(_rgb,data);
          
            doc["box_mac_id"] = Platform::getWebProperties().ap_ssid;
            doc["r_reg"] = _rgb.R.u16bit;
            doc["g_reg"] = _rgb.G.u16bit;
            doc["b_reg"] = _rgb.B.u16bit;
            doc["r_on"] = 0;
            doc["g_on"] = 0;
            doc["b_on"] = 0;
            doc["r_off"] = 0;
            doc["g_off"] = 0;
            doc["b_off"] = 0;
            switch (mail->eventType)
            {
                case  MeasEventType::EventSystemMeasure:
                {
                    doc["r_offset"] = Platform::getRGBProperties().r_offset;
                    doc["g_offset"] = Platform::getRGBProperties().g_offset;
                    doc["b_offset"] = Platform::getRGBProperties().b_offset;
                   // MQTTNetwork::getNetworkClient()->publish("TowerColorMeasure/"+Platform::getWebProperties().ap_ssid,doc.as<String>());
                }
                break;
                case  MeasEventType::EventServerMeasure:
                {
                    doc["r_offset"] = Platform::getRGBProperties().r_offset;
                    doc["g_offset"] = Platform::getRGBProperties().g_offset;
                    doc["b_offset"] = Platform::getRGBProperties().b_offset;
                    //MQTTNetwork::getNetworkClient()->publish("TowerColor/"+Platform::getWebProperties().ap_ssid,doc.as<String>());
                }
                break;
                case  MeasEventType::EventWebAppOffset:
                {   
                    
                    int32_t red_diff = (_rgb.R.u16bit-Platform::getRGBProperties().r_offset);
                    int32_t green_diff = (_rgb.G.u16bit-Platform::getRGBProperties().g_offset);
                    int32_t blue_diff = (_rgb.B.u16bit-Platform::getRGBProperties().b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);

                    Platform::getRGBProperties().r_offset = _rgb.R.u16bit;
                    Platform::getRGBProperties().g_offset = _rgb.G.u16bit;
                    Platform::getRGBProperties().b_offset = _rgb.B.u16bit;

                    doc["r_offset"] = Platform::getRGBProperties().r_offset;
                    doc["g_offset"] = Platform::getRGBProperties().g_offset;
                    doc["b_offset"] = Platform::getRGBProperties().b_offset;
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbOffset";
                     
                    text=String("{\"r_offset\":")+String(Platform::getRGBProperties().r_offset,DEC)
                                 +String(",\"g_offset\":")+String(Platform::getRGBProperties().g_offset,DEC)
                                 +String(",\"b_offset\":")+String(Platform::getRGBProperties().b_offset,DEC)+String("}");
                    
                    if(FatHelper.writeFile(FFat,Platform::getRGBProperties().path.c_str(),text)){
                        doc["file_write"]=true;
                        runCallbackWebSocketClientPostEvent(100,"done","progress");
                        runCallbackWebSocketClientText(mail->client,doc.as<String>());
                    }else{
                        doc["file_write"]=false;
                        runCallbackWebSocketClientPostEvent(80,"fail","progress");
                    }
                }
                break;
                case  MeasEventType::EventWebAppMeasure:
                {
                    int32_t red_diff = (_rgb.R.u16bit-Platform::getRGBProperties().r_offset);
                    int32_t green_diff = (_rgb.G.u16bit-Platform::getRGBProperties().g_offset);
                    int32_t blue_diff = (_rgb.B.u16bit-Platform::getRGBProperties().b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);
                    doc["r_offset"] = Platform::getRGBProperties().r_offset;
                    doc["g_offset"] = Platform::getRGBProperties().g_offset;
                    doc["b_offset"] = Platform::getRGBProperties().b_offset;
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbMeasure";
                    runCallbackWebSocketClientText(mail->client,doc.as<String>());
                    runCallbackWebSocketClientPostEvent(100,"done","progress");
                }
                break;
                default: break;
            }
            _mail_box_collection.free(mail); 
        }

    } 
}

void ColorCollector::runCallbackWebSocketClientPostEvent(int progress,const String& status , const String& event){
         std::lock_guard<rtos::Mutex> lck(_mtx);
         if(this->_cbWebSocketClientEvent!=nullptr){
            this->_cbWebSocketClientEvent("{\"status\":\""+status+"\",\"progress\":"+String(progress,DEC)+"}",event,0,0);
         }
}

void ColorCollector::runCallbackWebSocketClientText(AsyncWebSocketClient *client,const String& text){
         std::lock_guard<rtos::Mutex> lck(_mtx);
         if(this->_cbWebSocketClientText!=nullptr){
           this->_cbWebSocketClientText(client,text); 
         }
}

