#include "ColorCollector.h"

RGBProperties ColorCollector::rgb_properties;


void ColorCollector::run_task_collection()
{
    DynamicJsonDocument  doc(1024);
    rtos::Mutex std_mtx;
    ColorSensor<BH1749NUC,rtos::Mutex>  colorSensor(Wire1,4,15,std_mtx,2);
    colorSensor.init();
    colorSensor.attachMeasurementHook(std::bind(&ColorCollector::runCallbackWebSocketClientPostEvent,this,
        std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
     std::array<RGB,5> arrRGB;
    while(true){
        osEvent evt= _mail_box_post.get();
        if (evt.status == osEventMail) {
            collector::mail_t *mail = (collector::mail_t *)evt.value.p;

            colorSensor.loopMeasure(_rgb,Kernel::Clock::duration_milliseconds(200),arrRGB);

            _rgb.R.u16bit =  arrRGB[4].R.u16bit;
            _rgb.G.u16bit =  arrRGB[4].G.u16bit;
            _rgb.B.u16bit =  arrRGB[4].B.u16bit;
            platform_debug::TracePrinter::printTrace(String(_rgb.R.u16bit,DEC)+": "+
                                                              String(_rgb.G.u16bit,DEC)+": "+
                                                              String(_rgb.B.u16bit,DEC)+": "+
                                                              String(_rgb.IR.u16bit,DEC) ); 
           
            doc.clear();
            JsonArray data = doc.createNestedArray("TowerColor");
            _colorConverter.color(_rgb,data);
          
          
            doc["box_mac_id"] = Platform::getWebProperties()->ap_ssid;
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
                    doc["r_offset"] = ColorCollector::rgb_properties.r_offset;
                    doc["g_offset"] = ColorCollector::rgb_properties.g_offset;
                    doc["b_offset"] = ColorCollector::rgb_properties.b_offset;
                    MQTTNetwork::getNetworkClient()->publish("TowerColorMeasure/"+Platform::getWebProperties()->ap_ssid,doc.as<String>());
                }
                break;
                case  MeasEventType::EventServerMeasure:
                {
                    doc["r_offset"] = ColorCollector::rgb_properties.r_offset;
                    doc["g_offset"] = ColorCollector::rgb_properties.g_offset;
                    doc["b_offset"] = ColorCollector::rgb_properties.b_offset;
                    MQTTNetwork::getNetworkClient()->publish("TowerColor/"+Platform::getWebProperties()->ap_ssid,doc.as<String>());
                }
                break;
                case  MeasEventType::EventWebAppOffset:
                {   
                    
                    int32_t red_diff = (_rgb.R.u16bit-ColorCollector::rgb_properties.r_offset);
                    int32_t green_diff = (_rgb.G.u16bit-ColorCollector::rgb_properties.g_offset);
                    int32_t blue_diff = (_rgb.B.u16bit-ColorCollector::rgb_properties.b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);

                    ColorCollector::rgb_properties.r_offset = _rgb.R.u16bit;
                    ColorCollector::rgb_properties.g_offset = _rgb.G.u16bit;
                    ColorCollector::rgb_properties.b_offset = _rgb.B.u16bit;

                    doc["r_offset"] = ColorCollector::rgb_properties.r_offset;
                    doc["g_offset"] = ColorCollector::rgb_properties.g_offset;
                    doc["b_offset"] = ColorCollector::rgb_properties.b_offset;
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbOffset";
                     
                    String text=String("{\"r_offset\":")+String(ColorCollector::rgb_properties.r_offset,DEC)
                                 +String(",\"g_offset\":")+String(ColorCollector::rgb_properties.g_offset,DEC)
                                 +String(",\"b_offset\":")+String(ColorCollector::rgb_properties.b_offset,DEC)+String("}");
                      
                    if(FFatHelper::writeFile(FFat,ColorCollector::rgb_properties.path.c_str(),text)){
                        doc["file_write"]=true;
                    }else{
                        doc["file_write"]=false;
                    }
                    runCallbackWebSocketClientText(mail->client,doc.as<String>());
                    runCallbackWebSocketClientPostEvent(100,"done","progress");
                }
                break;
                case  MeasEventType::EventWebAppMeasure:
                {
                    int32_t red_diff = (_rgb.R.u16bit-ColorCollector::rgb_properties.r_offset);
                    int32_t green_diff = (_rgb.G.u16bit-ColorCollector::rgb_properties.g_offset);
                    int32_t blue_diff = (_rgb.B.u16bit-ColorCollector::rgb_properties.b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);
                    doc["r_offset"] = ColorCollector::rgb_properties.r_offset;
                    doc["g_offset"] = ColorCollector::rgb_properties.g_offset;
                    doc["b_offset"] = ColorCollector::rgb_properties.b_offset;
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbMeasure";
                    runCallbackWebSocketClientText(mail->client,doc.as<String>());
                    runCallbackWebSocketClientPostEvent(100,"done","progress");
                }
                break;
                default: break;
            }
             _mail_box_post.free(mail); 
            
        }
    } 
}


