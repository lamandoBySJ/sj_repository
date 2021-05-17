#include "RGBCollector.h"


void RGBCollector::delegateMethodPostMail(MeasEventType measEventType,AsyncWebSocketClient* client)
{   
    collector::mail_t *mail = _mail_box.alloc();
    if(mail!=NULL){
        mail->client = client;
        mail->eventType = measEventType;
        _mail_box.put(mail) ;
     }
     Serial.println("delegateMethodPostMail"+String((int)measEventType,DEC));
}


void RGBCollector::run_task_collection()
{
    colorSensor.init();
    
    DynamicJsonDocument  doc(1024);
    while(true){
       
        osEvent evt= _mail_box.get();
        if (evt.status == osEventMail) {
            collector::mail_t *mail = (collector::mail_t *)evt.value.p;
            Serial.println("E:..........................................."+String((int)mail->eventType,DEC));

            int progress=0;
            RGB rgb;
    
            std::array<RGB,5> a;
            //colorSensor.loopMeasure(_rgb,std::chrono::milliseconds(200),a);
           // runCallbackWebSocketClientPostEvent("{\"status\":\"measuring\",\"progress\":"+String(progress,DEC)+"}","progress");
             platform_debug::TracePrinter::printTrace(String(_rgb.R.u16bit,DEC)+": "+
                                                              String(_rgb.G.u16bit,DEC)+": "+
                                                              String(_rgb.B.u16bit,DEC)+": "+
                                                              String(_rgb.IR.u16bit,DEC) ); 
           
            //platform_debug::TracePrinter::printTrace("Returned :No Client connected!\n");
            doc.clear();
            JsonArray data = doc.createNestedArray("TowerColor");
            _colorConverter.color(_rgb,data);
          
          
            doc["box_mac_id"] = web_properties::ap_ssid;
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
                    doc["r_reg"] = _rgb.R.u16bit;
                    doc["g_reg"] = _rgb.G.u16bit;
                    doc["b_reg"] = _rgb.B.u16bit;
                }
                break;
                case  MeasEventType::EventServerMeasure:
                {
                    doc["r_reg"] = _rgb.R.u16bit;
                    doc["g_reg"] = _rgb.G.u16bit;
                    doc["b_reg"] = _rgb.B.u16bit;
                }
                break;
                case  MeasEventType::EventSystemOffset:
                {   
                    
                    int32_t red_diff = (_rgb.R.u16bit-rgb_properties::r_offset);
                    int32_t green_diff = (_rgb.G.u16bit-rgb_properties::g_offset);
                    int32_t blue_diff = (_rgb.B.u16bit-rgb_properties::b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);

                    rgb_properties::r_offset = _rgb.R.u16bit;
                    rgb_properties::g_offset = _rgb.G.u16bit;
                    rgb_properties::b_offset = _rgb.B.u16bit;

                    doc["r_offset"] = rgb_properties::r_offset;
                    doc["g_offset"] = rgb_properties::g_offset;
                    doc["b_offset"] = rgb_properties::b_offset;
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbOffset";
                     
                    String text=String("{\"r_offset\":")+String(rgb_properties::r_offset,DEC)
                                 +String(",\"g_offset\":")+String(rgb_properties::g_offset,DEC)
                                 +String(",\"b_offset\":")+String(rgb_properties::b_offset,DEC)+String("}");
                      
                    if(FFatHelper::writeFile(FFat,rgb_properties::path.c_str(),text)){
                        doc["file_write"]=true;
                    }else{
                        doc["file_write"]=false;
                    }
                    runCallbackWebSocketClientText(mail->client,doc.as<String>());
                }
                break;
                case  MeasEventType::EventWebAppMeasure:
                {
                    int32_t red_diff = (_rgb.R.u16bit-rgb_properties::r_offset);
                    int32_t green_diff = (_rgb.G.u16bit-rgb_properties::g_offset);
                    int32_t blue_diff = (_rgb.B.u16bit-rgb_properties::b_offset);
                    doc["r_reg"] = _rgb.R.u16bit;
                    doc["g_reg"] = _rgb.G.u16bit;
                    doc["b_reg"] = _rgb.B.u16bit;
                    doc["r_offset"] = rgb_properties::r_offset;
                    doc["g_offset"] = rgb_properties::g_offset;
                    doc["b_offset"] = rgb_properties::b_offset;
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbMeasure";
                    runCallbackWebSocketClientText(mail->client,doc.as<String>());
                }
                break;
                default: break;
            }
             _mail_box.free(mail); 

           // runCallbackWebSocketClientPostEvent("{\"status\":\"done\",\"progress\":100}","progress");
          
        }
    } 
}

void RGBCollector::startup()
{  
    _thread.start(callback(this,&RGBCollector::run_task_collection));
}
//template class RGBCollector<ColorSensor<BH1749NUC>>;
