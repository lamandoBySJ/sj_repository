#include "AsyncMqttClientService.h"

void AsyncMqttClientService::init(const char* host,uint16_t port)
{
      std::lock_guard<rtos::Mutex> lck(_mtx);
      _host=host;
      _port=port;

      _client.setWill("STLB_WILL",0,false,_clientId,strlen(_clientId));
      _client.setCleanSession(true);
      _client.setKeepAlive(300);
      _client.setClientId(_clientId);
    

      _client.onConnect(mbed::callback(this,&AsyncMqttClientService::onMqttConnect));
      _client.onDisconnect(mbed::callback(this,&AsyncMqttClientService::onMqttDisconnect));
      _client.onSubscribe(mbed::callback(this,&AsyncMqttClientService::onMqttSubscribe));
      _client.onUnsubscribe(mbed::callback(this,&AsyncMqttClientService::onMqttUnsubscribe));
      _client.onMessage(mbed::callback(this,&AsyncMqttClientService::onMqttMessage));
      _client.onPublish(mbed::callback(this,&AsyncMqttClientService::onMqttPublish));

      _client.setServer(_host,_port); 

      osStatus status =  _threadTx.start(mbed::callback(this,&AsyncMqttClientService::eventTxLooper));
      (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadTx.get_name()):NULL); 

      status =  _threadRx.start(mbed::callback(this,&AsyncMqttClientService::eventRxLooper));
      (status!=osOK ? throw os::thread_error((osStatus_t)status,_threadRx.get_name()):NULL); 

}

bool AsyncMqttClientService::postMailBoxTx(MqttEventType type,const char* topic, uint8_t qos, bool retain, const char* payload, size_t length, bool dup, uint16_t message_id){
        
        while(!_connected.load()){
            ThisThread::sleep_for(random(500,1000));
        }
        std::lock_guard<rtos::Mutex> lck(_mtx);
        mqtt_event_t *mail = _mail_box_tx.alloc();
        if(mail){
            mail->type=type;
            switch(type){
                case MqttEventType::MQTT_PUBLISH:
                {
                    mail->properties.qos = qos;
                    mail->properties.dup = dup;
                    mail->properties.retain = retain;
                    snprintf((char*)mail->topic,sizeof(mail->topic),"%s",topic);
                    mail->payload = mail->copy("%.*s",length,payload);
                }
                break;
                case MqttEventType::MQTT_SUBSCRIBE:
                {
                    mail->properties.qos = qos;
                    snprintf((char*)mail->topic,sizeof(mail->topic),"%s",topic);
                }
                break;
                case MqttEventType::MQTT_UNSUBSCRIBE:
                {
                    snprintf((char*)mail->topic,sizeof(mail->topic),"%s",topic);
                }
                break;
                default:break;
            }
            _mail_box_tx.put(mail) ;
        }
        return true;
}

void AsyncMqttClientService::eventRxLooper()
{
    while(true){
        osEvent evt= _mail_box_rx.get();
        if (evt.status == osEventMail) {
            mqtt_event_t *mail = (mqtt_event_t *)evt.value.p;
            if(!mail){
                continue;
            }
            for(auto& v : _MqttEventListener){
                        switch(mail->type){
                            case MqttEventType::ON_MQTT_CONNECT:v->onMqttConnect(mail->sessionPresent);break;
                            case MqttEventType::ON_MQTT_DISCONNECT:
                            {
                                if(mail->reason == AsyncMqttClientDisconnectReason::TCP_DISCONNECTED){
                                    v->onMqttConnectionLost();
                                }else{
                                    v->onMqttDisconnect(mail->reason);
                                }
                            }
                            break;
                            case MqttEventType::ON_MQTT_SUBSCRIBE:v->onMqttSubscribe(mail->packetId,mail->properties.qos) ;break;
                            case MqttEventType::ON_MQTT_UNSUBSCRIBE:v->onMqttUnsubscribe(mail->packetId);break;
                            case MqttEventType::ON_MQTT_MESSAGE:
                            {
                                v->onMqttMessage(mail->topic,mail->payload,mail->properties,mail->len,mail->index,mail->total);
                            }
                            break;
                            case MqttEventType::ON_MQTT_PUBLISH:v->onMqttPublish(mail->packetId) ;break;
                            default:break;   
                        }
            }
            _mail_box_rx.free(mail); 
        }
    }
}

void AsyncMqttClientService::eventTxLooper()
{
        while(true){
            osEvent evt= _mail_box_tx.get();
            if (evt.status == osEventMail) {
                mqtt_event_t *mail = (mqtt_event_t *)evt.value.p;
                if(!mail){
                    continue;
                }
                #ifdef TRACE_PRINTER_NDEBUG
                TracePrinter::printTrace("[OK]:mail arrived."+String((int)mail->type,DEC));
                #endif
           
                uint16_t res=1;
                     switch(mail->type){
                        case MqttEventType::MQTT_CONNECT: 
                        {
                            if(!_client.connected()){
                                _client.connect();
                            }
                               
                        }break;
                        case MqttEventType::MQTT_DISCONNECT:
                        {
                            _client.disconnect();
                        }
                        break;
                        case MqttEventType::MQTT_PUBLISH:
                        {
                            res  =_client.publish(mail->topic,mail->properties.qos,mail->properties.retain,mail->payload,mail->len);

                            if(res==0){
                                for(auto& v : _MqttEventListener){
                                     v->onMqttPublishError(mail->topic,mail->properties.qos,mail->properties.retain,mail->payload,mail->len,mail->properties.dup,mail->total);
                                }
                            }
                        }
                        break;
                        case MqttEventType::MQTT_SUBSCRIBE:
                        {
                            res =_client.subscribe(mail->topic,mail->properties.qos);
                            if(res==0){
                                for(auto& v : _MqttEventListener){
                                    v->onMqttSubscribeError(mail->topic);
                                }
                            }
                        }
                        break;
                        case MqttEventType::MQTT_UNSUBSCRIBE:
                        {
                            _client.unsubscribe(mail->topic);
                            if(res==0){
                                for(auto& v : _MqttEventListener){
                                    v->onMqttUnsubscribeError(mail->topic);
                                }
                            }
                        }
                        break;
                        default:break;
                    }

                _mail_box_tx.free(mail); 
            }
        }
    }