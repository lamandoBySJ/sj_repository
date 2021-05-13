#include "RGBCollector.h"

template<class T>
RGBCollector<T>::RGBCollector(MQTTNetwork& MQTTnetwork,ColorSensor<T>& colorSensor):
    _MQTTnetwork(MQTTnetwork),_colorSensor(colorSensor),_rgb(),_rgbTemp(),_mail_box()
    ,_mtx(),_mtxCallback(),_cbWebSocketClientEvent(nullptr),_colorConverter()
{

}
template<class T>
RGBCollector<T>::~RGBCollector()
{

}

template<class T>
void RGBCollector<T>::startup()
{
   //_thread = std::thread(&RGBCollector<T>::run_collector,this); 
    //_thread.join();
}

template<class T>
void RGBCollector<T>::delegateMethodPostMail(MeasEventType measEventType,uint32_t id)
{
    std::unique_lock<std::mutex> lck(_mtx, std::defer_lock);
	lck.lock();
    collector::mail_t *mail = _mail_box.alloc();
    if(mail!=NULL){
        mail->id = id;
        mail->eventType = measEventType;
        _mail_box.put(mail) ;
     }
}

template<class T>
void RGBCollector<T>::run_collector()
{
    DynamicJsonDocument  docProgress(200);
    DynamicJsonDocument  doc(2048);
    int progress=0;
    while(true){
        osEvent evt= _mail_box.get();
        if (evt.status == osEventMail) {
            _rgb.R.i16bit=0;
            _rgb.G.i16bit=0;
            _rgb.B.i16bit=0;
            _rgb.IR.i16bit=0;
            _colorSensor.measurementModeActive();
            for(char i=0;i<5;++i){
                _colorSensor.getRGB(_rgbTemp);

                if(_rgb.R.i16bit < _rgbTemp.R.i16bit ||
                   _rgb.G.i16bit < _rgbTemp.G.i16bit ||
                   _rgb.B.i16bit < _rgbTemp.B.i16bit)
                {
                    _rgb.R.i16bit=_rgbTemp.R.i16bit;
                    _rgb.G.i16bit=_rgbTemp.G.i16bit;
                     _rgb.B.i16bit=_rgbTemp.B.i16bit;
                    _rgb.IR.i16bit=_rgbTemp.IR.i16bit;
                 }
                progress+=15;
                docProgress["status"]="measuring";
                docProgress["progress"]=progress;
                runCallbackWebSocketClientPostEvent(docProgress.as<String>(),"progress");
                std::this_thread::sleep_for(chrono::milliseconds(200));
                
                platform_debug::TracePrinter::printTrace(String(_rgb.R.i16bit,DEC)+": "+
                                                              String(_rgb.G.i16bit,DEC)+": "+
                                                              String(_rgb.B.i16bit,DEC)+": "+
                                                              String(_rgb.IR.i16bit,DEC) ); 
                                                                                        
            }
            _colorSensor.measurementModeInactive();
            //platform_debug::TracePrinter::printTrace("Returned :No Client connected!\n");
            doc.clear();
            JsonArray data = doc.createNestedArray("TowerColor");
            _colorConverter.color(_rgb,data);
            

            collector::mail_t *mail = (collector::mail_t *)evt.value.p;
            switch (mail->eventType)
            {
                case  MeasEventType::EventSystemMeasure:
                {
                    doc["r_reg"] = _rgb.R.i16bit;
                    doc["g_reg"] = _rgb.G.i16bit;
                    doc["b_reg"] = _rgb.B.i16bit;
                }
                break;
                case  MeasEventType::EventServerMeasure:
                {
                    doc["r_reg"] = _rgb.R.i16bit;
                    doc["g_reg"] = _rgb.G.i16bit;
                    doc["b_reg"] = _rgb.B.i16bit;
                }
                break;
                case  MeasEventType::EventSystemOffset:
                {
                    int32_t red_diff = (_rgb.R.i16bit-rgb_properties::r_offset);
                    int32_t green_diff = (_rgb.G.i16bit-rgb_properties::g_offset);
                    int32_t blue_diff = (_rgb.B.i16bit-rgb_properties::b_offset);
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);

                    rgb_properties::r_offset = _rgb.R.i16bit;
                    rgb_properties::g_offset = _rgb.G.i16bit;
                    rgb_properties::b_offset = _rgb.B.i16bit;

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
                    runCallbackWebSocketClientText(mail->id,doc.as<String>());
                }
                break;
                case  MeasEventType::EventWebAppMeasure:
                {
                    int32_t red_diff = (_rgb.R.i16bit-rgb_properties::r_offset);
                    int32_t green_diff = (_rgb.G.i16bit-rgb_properties::g_offset);
                    int32_t blue_diff = (_rgb.B.i16bit-rgb_properties::b_offset);
                    doc["r_reg"] = _rgb.R.i16bit;
                    doc["g_reg"] = _rgb.G.i16bit;
                    doc["b_reg"] = _rgb.B.i16bit;
                    doc["r_offset"] = rgb_properties::r_offset;
                    doc["g_offset"] = rgb_properties::g_offset;
                    doc["b_offset"] = rgb_properties::b_offset;
                    doc["r_diff"] =  abs(red_diff);
                    doc["g_diff"] =  abs(green_diff);
                    doc["b_diff"] =  abs(blue_diff);
                    doc["ws_evt_type"]="WS_EVT_DATA";
                    doc["msg"]="RgbMeasure";
                    runCallbackWebSocketClientText(mail->id,doc.as<String>());
                }
                break;
                default: break;
            }
             _mail_box.free(mail); 
            
            docProgress["status"]="done";
            docProgress["progress"]=100;
            runCallbackWebSocketClientPostEvent(docProgress.as<String>(),"progress");
        }
    } 
}

template class RGBCollector<BH1749NUC>;
