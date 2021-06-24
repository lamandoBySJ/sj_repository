#include "ColorCollector.h"
#include "Logger.h"
#include "ColorConverter.h"
#include "ArduinoJson.h"

extern FFatHelper<rtos::Mutex> FatHelper;
void ColorCollector::post_mail_measure(MeasEventType measEventType,AsyncWebSocketClient *client)
{   
    os::mail_ws_t *mail = _mail_box_collection.alloc();
    if(mail){
        mail->client = client;
        mail->eventType = measEventType;
        _mail_box_collection.put(mail);
    }
}
void  ColorCollector::startup()
{   
    osStatus  status = _thread.start(mbed::callback(this,&ColorCollector::task_collection));
    (status!=osOK ? throw os::thread_error((osStatus_t)status,_thread.get_name()):NULL);
}

void ColorCollector::task_collection()
{

    _colorSensor.attachMeasurementHook(std::bind(&ColorCollector::invokeCallbackWebSocketClientPostEvent,this,
      std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    String rtc_datetime,system_clock_datetime;
    time_t rtc_epoch,system_clock_epoch;
    String lastColor;
    std::array<RGB,10> arrRGB;
    RGB rgb;
    uint16_t max;
    String log;
    while(true){

        osEvent evt =  _mail_box_collection.get();
        if (evt.status == osEventMail) {
            os::mail_ws_t *mail = (os::mail_ws_t *)evt.value.p;

            _colorSensor.loopMeasure(_rgb_reg,Kernel::Clock::duration_milliseconds(200),arrRGB);

            max=_rgb_reg.R.u16bit;
            if(max>_rgb_reg.G.u16bit){
                std::sort(arrRGB.begin(),arrRGB.end(),[](const RGB& left,const RGB& right)->bool{
                    return left.R.u16bit < right.R.u16bit;
                });
            }else{
                max=_rgb_reg.G.u16bit;
                std::sort(arrRGB.begin(),arrRGB.end(),[](const RGB& left,const RGB& right)->bool{
                    return left.G.u16bit < right.G.u16bit;
                });
            }
            
            if(max < _rgb_reg.B.u16bit){
                max=_rgb_reg.B.u16bit;
                std::sort(arrRGB.begin(),arrRGB.end(),[](const RGB& left,const RGB& right)->bool{
                    return left.B.u16bit < right.G.u16bit;
                });
            }
            
            _rgb_reg.R.u16bit =  arrRGB[arrRGB.size()-1].R.u16bit;
            _rgb_reg.G.u16bit =  arrRGB[arrRGB.size()-1].G.u16bit;
            _rgb_reg.B.u16bit =  arrRGB[arrRGB.size()-1].B.u16bit;

            rgb.R.u16bit   = abs(product_api::get_rgb_properties().r_offset-_rgb_reg.R.u16bit);
            rgb.G.u16bit   = abs(product_api::get_rgb_properties().g_offset-_rgb_reg.G.u16bit);
            rgb.B.u16bit   = abs(product_api::get_rgb_properties().b_offset-_rgb_reg.B.u16bit);

            doc.clear();
            doc["version"] = platformio_api::get_version();

            JsonArray data = doc.createNestedArray("TowerColor");
            if(_rgb_reg.R.u16bit<10&&_rgb_reg.G.u16bit<10&&_rgb_reg.B.u16bit<10){
                data.add("Black");
            }else if(rgb.R.u16bit<10&&rgb.G.u16bit<10&&rgb.B.u16bit<10){
                 data.add("Black");
            }else{
                ColorConverter::getColorConverter().color(rgb,data);
            }   
   
            system_clock_epoch = SystemClock::LocalDateTime::now(system_clock_datetime);
            rtc_epoch = RTC::LocalDateTime::now(rtc_datetime);

            if( abs(system_clock_epoch - rtc_epoch) > 43200){
                Logger::error(String(__PRETTY_FUNCTION__)+String(":"),system_clock_epoch);
                Logger::error(String(__PRETTY_FUNCTION__)+String(":"),rtc_epoch);
            }
            if(Logger::getInstance().get_error_count() > 0){
                Logger::getInstance().error_functions_get(log);
                invokeCallbackMqttPublish("error_functions/"+platformio_api::get_device_info().BoardID,log);
            }
            doc["unix_timestamp"]= rtc_epoch;
            doc["datetime"]= rtc_datetime;  

            doc["system_clock_epoch"]= system_clock_epoch;
            doc["system_clock_datetime"]= system_clock_datetime;  
            //JsonArray error_logs = doc.createNestedArray("error_functions");
            //Logger::getInstance().error_log_get(error_logs); 
            doc["error_functions"] = Logger::getInstance().get_error_count();
            doc["r_reg"] = _rgb_reg.R.u16bit;
            doc["g_reg"] = _rgb_reg.G.u16bit;
            doc["b_reg"] = _rgb_reg.B.u16bit;
            doc["r_on"] = 0;
            doc["g_on"] = 0;
            doc["b_on"] = 0;
            doc["r_off"] = 0;
            doc["g_off"] = 0;
            doc["b_off"] = 0;
            TracePrinter::printTrace(doc["TowerColor"].as<String>());
            switch (mail->eventType)
            {
                case  MeasEventType::EventSystemMeasure:
                {
                    doc["SentFrom"] = platformio_api::get_device_info().BoardID;
                    doc["r_offset"] = product_api::get_rgb_properties().r_offset;
                    doc["g_offset"] = product_api::get_rgb_properties().g_offset;
                    doc["b_offset"] = product_api::get_rgb_properties().b_offset;
                    doc["type"] = "system_measure";
                    if(lastColor!=doc["TowerColor"].as<String>()){
                        lastColor = doc["TowerColor"].as<String>();
                        invokeCallbackMqttPublish("TowerColor/"+platformio_api::get_device_info().BoardID,doc.as<String>());
                    }
                }
                break;
                case MeasEventType::EventTimeoutMeasure:
                {
                    doc["SentFrom"] = platformio_api::get_device_info().BoardID;
                    doc["r_offset"] = product_api::get_rgb_properties().r_offset;
                    doc["g_offset"] = product_api::get_rgb_properties().g_offset;
                    doc["b_offset"] = product_api::get_rgb_properties().b_offset;
                    doc["type"] = "timeout_measure";
                    invokeCallbackMqttPublish("TowerColor/"+platformio_api::get_device_info().BoardID,doc.as<String>());
                }
                break;
                case  MeasEventType::EventServerMeasure:
                {
                    doc["SentFrom"] = platformio_api::get_device_info().BoardID;
                    doc["r_offset"] = product_api::get_rgb_properties().r_offset;
                    doc["g_offset"] = product_api::get_rgb_properties().g_offset;
                    doc["b_offset"] = product_api::get_rgb_properties().b_offset;
                    doc["type"] = "server_measure";
                    invokeCallbackMqttPublish("TowerColorMeasure/"+platformio_api::get_device_info().BoardID,doc.as<String>());
                }
                break;
                case  MeasEventType::EventManulRequest:
                {
                    doc["SentFrom"] = platformio_api::get_device_info().BoardID;
                    doc["r_offset"] = product_api::get_rgb_properties().r_offset;
                    doc["g_offset"] = product_api::get_rgb_properties().g_offset;
                    doc["b_offset"] = product_api::get_rgb_properties().b_offset;
                    doc["type"] = "manual_request";
                    if(lastColor!=doc["TowerColor"].as<String>()){
                        lastColor = doc["TowerColor"].as<String>();
                        doc["data_override"] = false;
                    }else{
                        doc["data_override"] = true;
                    }
                    invokeCallbackMqttPublish("TowerColor/"+platformio_api::get_device_info().BoardID,doc.as<String>());
                }
                break;
                
                case  MeasEventType::EventWebAppOffset:
                {   
                    doc["box_mac_id"] = platformio_api::get_web_properties().ap_ssid;
                    int32_t red_diff = (_rgb_reg.R.u16bit-product_api::get_rgb_properties().r_offset);
                    int32_t green_diff = (_rgb_reg.G.u16bit-product_api::get_rgb_properties().g_offset);
                    int32_t blue_diff = (_rgb_reg.B.u16bit-product_api::get_rgb_properties().b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);

                    product_api::get_rgb_properties().r_offset = _rgb_reg.R.u16bit;
                    product_api::get_rgb_properties().g_offset = _rgb_reg.G.u16bit;
                    product_api::get_rgb_properties().b_offset = _rgb_reg.B.u16bit;

                    doc["r_offset"] = product_api::get_rgb_properties().r_offset;
                    doc["g_offset"] = product_api::get_rgb_properties().g_offset;
                    doc["b_offset"] = product_api::get_rgb_properties().b_offset;
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbOffset";
                     
                    text=String("{\"r_offset\":")+String(product_api::get_rgb_properties().r_offset,DEC)
                                 +String(",\"g_offset\":")+String(product_api::get_rgb_properties().g_offset,DEC)
                                 +String(",\"b_offset\":")+String(product_api::get_rgb_properties().b_offset,DEC)+String("}");
                    
                    if(FatHelper.writeFile(FFat,product_api::get_rgb_properties().path.c_str(),text)){
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
                    doc["box_mac_id"] = platformio_api::get_web_properties().ap_ssid;
                    int32_t red_diff = (_rgb_reg.R.u16bit-product_api::get_rgb_properties().r_offset);
                    int32_t green_diff = (_rgb_reg.G.u16bit-product_api::get_rgb_properties().g_offset);
                    int32_t blue_diff = (_rgb_reg.B.u16bit-product_api::get_rgb_properties().b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);
                    doc["r_offset"] = product_api::get_rgb_properties().r_offset;
                    doc["g_offset"] = product_api::get_rgb_properties().g_offset;
                    doc["b_offset"] = product_api::get_rgb_properties().b_offset;
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

void ColorCollector::delegateMethodOnWsEvent(AsyncWebSocket * server, AsyncWebSocketClient *client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT) {
        TracePrinter::printf("ws[%s][%u] connect\n", server->url(), client->id());
        DynamicJsonDocument  doc(200);
        doc["box_mac_id"] = platformio_api::get_web_properties().ap_ssid;
        doc["ws_evt_type"] = "WS_EVT_CONNECT";
        doc["r_offset"] = product_api::get_rgb_properties().r_offset;
        doc["g_offset"] = product_api::get_rgb_properties().g_offset;
        doc["b_offset"] = product_api::get_rgb_properties().b_offset;
        client->text(doc.as<String>().c_str());
        client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
        TracePrinter::printf("ws[%s][%u] disconnect\n", server->url(), client);
    } else if (type == WS_EVT_ERROR) {
        TracePrinter::printf("ws[%s][%u] error(%u): %s\n", server->url(), client, *((uint16_t*)arg), (char*)data);
    } else if (type == WS_EVT_PONG) {
        TracePrinter::printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client, len, (len) ? (char*)data : "");
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len) {
        //the whole message is in a single frame and we got all of it's data
            TracePrinter::printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

            if (info->opcode == WS_TEXT) {
                for (size_t i = 0; i < info->len; i++) {
                    msg += (char) data[i];
                }
            } else {
                char buff[3];
                for (size_t i = 0; i < info->len; i++) {
                    sprintf(buff, "%02x ", (uint8_t) data[i]);
                    msg += buff ;
                }
            }
            DynamicJsonDocument  doc(500);
            DeserializationError error = deserializeJson(doc, msg.c_str());

            if (!error)
            {
                if (doc["msg"].as<String>() == String("rgb_offset")) {
                    post_mail_measure(MeasEventType::EventWebAppOffset,client);
                } else if (doc["msg"].as<String>() == String("rgb_measure")) {
                    post_mail_measure(MeasEventType::EventWebAppMeasure,client);
                }
            }
        }
    }
}
