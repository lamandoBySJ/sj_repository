#include "ColorCollector.h"
#include "app/TimeMachine/TimeMachine.h"

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
    ColorSensor<BH1749NUC,rtos::Mutex>::getColorSensor()->attachMeasurementHook(std::bind(&ColorCollector::invokeCallbackWebSocketClientPostEvent,this,
      std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    String timepoint;
    String lastColor;
    std::array<RGB,10> arrRGB;
    RGB rgb;
    while(true){
        // ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(30));
        osEvent evt =  _mail_box_collection.get();
        if (evt.status == osEventMail) {
            collector::mail_ws_t *mail = (collector::mail_ws_t *)evt.value.p;

            ColorSensor<BH1749NUC,rtos::Mutex>::getColorSensor()->loopMeasure(_rgb_reg,Kernel::Clock::duration_milliseconds(200),arrRGB);
            _rgb_reg.R.u16bit =  arrRGB[arrRGB.size()-1].R.u16bit;
            _rgb_reg.G.u16bit =  arrRGB[arrRGB.size()-1].G.u16bit;
            _rgb_reg.B.u16bit =  arrRGB[arrRGB.size()-1].B.u16bit;
            TracePrinter::printTrace(String(_rgb_reg.R.u16bit,DEC)+": "+ String(_rgb_reg.G.u16bit,DEC)+": "+
                                     String(_rgb_reg.B.u16bit,DEC)+": "+ String(_rgb_reg.IR.u16bit,DEC) ); 

            rgb.R.u16bit   = abs(Platform::get_rgb_properties().r_offset-_rgb_reg.R.u16bit);
            rgb.G.u16bit   = abs(Platform::get_rgb_properties().g_offset-_rgb_reg.G.u16bit);
            rgb.B.u16bit   = abs(Platform::get_rgb_properties().b_offset-_rgb_reg.B.u16bit);

            doc.clear();
            JsonArray data = doc.createNestedArray("TowerColor");
            if(_rgb_reg.R.u16bit<10&&_rgb_reg.G.u16bit<10&&_rgb_reg.B.u16bit<10){
                data.add("Black");
            }else if(rgb.R.u16bit<10&&rgb.G.u16bit<10&&rgb.B.u16bit<10){
                 data.add("Black");
            }else{
                ColorConverter::getColorConverter().color(rgb,data);
            }   
         
            
            doc["unix_timestamp"]= TimeMachine<DS1307,rtos::Mutex>::getTimeMachine()->getEpoch();
            TimeMachine<DS1307,rtos::Mutex>::getTimeMachine()->getDateTime(timepoint);
            doc["datetime"]= timepoint;    
            doc["r_reg"] = _rgb_reg.R.u16bit;
            doc["g_reg"] = _rgb_reg.G.u16bit;
            doc["b_reg"] = _rgb_reg.B.u16bit;
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
                    doc["SentFrom"] = Platform::get_device_Info().BoardID;
                    doc["r_offset"] = Platform::get_rgb_properties().r_offset;
                    doc["g_offset"] = Platform::get_rgb_properties().g_offset;
                    doc["b_offset"] = Platform::get_rgb_properties().b_offset;
                    TracePrinter::printTrace(doc["TowerColor"].as<String>());
                    if(lastColor!=doc["TowerColor"].as<String>()){
                        lastColor = doc["TowerColor"].as<String>();
                        invokeCallbackMqttPublish("TowerColor/"+Platform::get_device_Info().BoardID,doc.as<String>());
                    }
                }
                break;
                case  MeasEventType::EventServerMeasure:
                {
                    doc["SentFrom"] = Platform::get_device_Info().BoardID;
                    doc["r_offset"] = Platform::get_rgb_properties().r_offset;
                    doc["g_offset"] = Platform::get_rgb_properties().g_offset;
                    doc["b_offset"] = Platform::get_rgb_properties().b_offset;
                    invokeCallbackMqttPublish("TowerColorMeasure/"+Platform::get_device_Info().BoardID,doc.as<String>());
                }
                break;
                case  MeasEventType::EventWebAppOffset:
                {   
                    doc["box_mac_id"] = Platform::get_web_properties().ap_ssid;
                    int32_t red_diff = (_rgb_reg.R.u16bit-Platform::get_rgb_properties().r_offset);
                    int32_t green_diff = (_rgb_reg.G.u16bit-Platform::get_rgb_properties().g_offset);
                    int32_t blue_diff = (_rgb_reg.B.u16bit-Platform::get_rgb_properties().b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);

                    Platform::get_rgb_properties().r_offset = _rgb_reg.R.u16bit;
                    Platform::get_rgb_properties().g_offset = _rgb_reg.G.u16bit;
                    Platform::get_rgb_properties().b_offset = _rgb_reg.B.u16bit;

                    doc["r_offset"] = Platform::get_rgb_properties().r_offset;
                    doc["g_offset"] = Platform::get_rgb_properties().g_offset;
                    doc["b_offset"] = Platform::get_rgb_properties().b_offset;
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbOffset";
                     
                    text=String("{\"r_offset\":")+String(Platform::get_rgb_properties().r_offset,DEC)
                                 +String(",\"g_offset\":")+String(Platform::get_rgb_properties().g_offset,DEC)
                                 +String(",\"b_offset\":")+String(Platform::get_rgb_properties().b_offset,DEC)+String("}");
                    
                    if(FatHelper.writeFile(FFat,Platform::get_rgb_properties().path.c_str(),text)){
                        doc["file_write"]=true;
                    }else{
                        doc["file_write"]=false;
                    }
                    if(mail->client!=nullptr){
                        invokeCallbackWebSocketClientPostEvent(100,doc["file_write"].as<bool>()?"done":"fail","progress");
                        invokeCallbackWebSocketClientText(mail->client,doc.as<String>());
                    }
                }
                break;
                case  MeasEventType::EventWebAppMeasure:
                {
                    doc["box_mac_id"] = Platform::get_web_properties().ap_ssid;
                    int32_t red_diff = (_rgb_reg.R.u16bit-Platform::get_rgb_properties().r_offset);
                    int32_t green_diff = (_rgb_reg.G.u16bit-Platform::get_rgb_properties().g_offset);
                    int32_t blue_diff = (_rgb_reg.B.u16bit-Platform::get_rgb_properties().b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);
                    doc["r_offset"] = Platform::get_rgb_properties().r_offset;
                    doc["g_offset"] = Platform::get_rgb_properties().g_offset;
                    doc["b_offset"] = Platform::get_rgb_properties().b_offset;
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbMeasure";
                    if(mail->client!=nullptr){
                        invokeCallbackWebSocketClientText(mail->client,doc.as<String>());
                        invokeCallbackWebSocketClientPostEvent(100,"done","progress");
                    }
                }
                break;
                default: break;
            }
            _mail_box_collection.free(mail); 
        }

    } 
}

void ColorCollector::invokeCallbackWebSocketClientPostEvent(int progress,const String& status , const String& event){
         std::lock_guard<rtos::Mutex> lck(_mtx);
         if(this->_cbWebSocketClientEvent!=nullptr){
            this->_cbWebSocketClientEvent("{\"status\":\""+status+"\",\"progress\":"+String(progress,DEC)+"}",event,0,0);
         }
}

void ColorCollector::invokeCallbackWebSocketClientText(AsyncWebSocketClient *client,const String& text){
         std::lock_guard<rtos::Mutex> lck(_mtx);
         if(this->_cbWebSocketClientText!=nullptr){
           this->_cbWebSocketClientText(client,text); 
         }
}

