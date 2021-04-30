#include "ESPWebServer.h"

ESPWebServer::ESPWebServer():_server(80),_events("/events"),_wss("/ws"),running(false),
                            _mtxMailBox(),_mtxEventSource(),_mtxCallback(),_mail_box(),
                            _callback(nullptr){
       
}
ESPWebServer::~ESPWebServer(){

}

const char* PARAM_MESSAGE = "message";
void ESPWebServer::startup(){
  
        running =true;
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP(web_properties::ap_ssid.c_str(), web_properties::ap_pass.c_str());
     
        platform_debug::TracePrinter::printTrace(web_properties::ap_ssid);
        platform_debug::TracePrinter::printTrace(web_properties::ap_pass);
      
       // MDNS.addService("http","tcp",80);
        _server.addHandler(new SPIFFSEditor(FFat, web_properties::http_user,web_properties::http_pass));
        _server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(200, "text/plain", String(ESP.getFreeHeap()));
        });
        _server.serveStatic("/", FFat, "/data").setDefaultFile("index.html");
        _server.onNotFound(std::bind(&ESPWebServer::notFound,this,std::placeholders::_1));

        /*_server.onNotFound([](AsyncWebServerRequest *request){
            
        });*/

        _server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
            if(request->method() == HTTP_GET)
                platform_debug::TracePrinter::printTrace("GET");
            else if(request->method() == HTTP_POST)
                platform_debug::TracePrinter::printTrace("POST");
            else if(request->method() == HTTP_DELETE)
                platform_debug::TracePrinter::printTrace("DELETE");
            else if(request->method() == HTTP_PUT)
                platform_debug::TracePrinter::printTrace("PUT");
            else if(request->method() == HTTP_PATCH)
                platform_debug::TracePrinter::printTrace("PATCH");
            else if(request->method() == HTTP_HEAD)
                platform_debug::TracePrinter::printTrace("HEAD");
            else if(request->method() == HTTP_OPTIONS)
                platform_debug::TracePrinter::printTrace("OPTIONS");
            else
                platform_debug::TracePrinter::printTrace("UNKNOWN");
                platform_debug::TracePrinter::printfTrace(" http://%s%s\n", request->host().c_str(), request->url().c_str());

            if(request->contentLength()){
                platform_debug::TracePrinter::printfTrace("_CONTENT_TYPE: %s\n", request->contentType().c_str());
                platform_debug::TracePrinter::printfTrace("_CONTENT_LENGTH: %u\n", request->contentLength());
            }

            int headers = request->headers();
            int i;
            for(i=0;i<headers;i++){
            AsyncWebHeader* h = request->getHeader(i);
                platform_debug::TracePrinter::printfTrace("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
            }

            int params = request->params();
            for(i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isFile()){
                platform_debug::TracePrinter::printfTrace("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if(p->isPost()){
                platform_debug::TracePrinter::printfTrace("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                platform_debug::TracePrinter::printfTrace("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
            }
            request->send(200, "text/plain", "OK");
        },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
                String path = String("/data/")+filename;
                if(index==0){
                    platform_debug::TracePrinter::printfTrace("Upload: %s\n", filename.c_str());
                    FFatHelper::writeFile(FFat,path,data,len);
                }else{
                    FFatHelper::appendFile(FFat,path,data,len); 
                }
                
        },[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            if(!index)
                platform_debug::TracePrinter::printfTrace("BodyStart: %u\n", total);
                platform_debug::TracePrinter::printfTrace("%s", (const char*)data);
            if(index + len == total)
                platform_debug::TracePrinter::printfTrace("BodyEnd: %u\n", total);
        });

        _server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(200, "text/html",web_properties::server_upload_uri.c_str());
        });
        
        _server.on("/getSTLB", HTTP_GET, [&](AsyncWebServerRequest *request) {
             DynamicJsonDocument  _docucment(600);
            _docucment["box_mac_id"] = web_properties::ap_ssid;
            _docucment["ssid"] = mqtt::user_properties::ssid;
            _docucment["pass"] = mqtt::user_properties::pass;
            _docucment["host"] = mqtt::user_properties::host;
            _docucment["port"] = mqtt::user_properties::port;
            request->send(200, "application/json", _docucment.as<String>());
        });   
     
        _server.on("/reboot", HTTP_POST, [] (AsyncWebServerRequest *request) {
            if (request->hasParam("reboot")) {
                platform_debug::TracePrinter::printTrace(request->getParam("reboot")->value());
            }
            request->send(200, "text/plain", "OK:Reboot Now!" );
            std::this_thread::sleep_for(chrono::seconds(3));
            esp_restart();
        });
    
        _server.on("/als", HTTP_GET, [] (AsyncWebServerRequest *request) {
        request->send(FFat, "/data/als.html", "text/html");
        });
        
        _server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
        });

        _server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
            bool result = Update.hasError();
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", result ? "OK" : "FAIL");
            response->addHeader("Connection", "close");
            request->send(response);
        }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
            platform_debug::TracePrinter::printfTrace("Update Start: %s\n", filename.c_str());
            platform_debug::TracePrinter::printfTrace("ESP.getFreeSketchSpace(): %x\n", ESP.getFreeSketchSpace());
            //Update.runAsync();
            if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
                Update.printError(Serial);
            }
            }
            if (!Update.hasError()) {
                if (Update.write(data, len) != len) {
                    Update.printError(Serial);
                }
            }
            if (final) {
                if (Update.end(true)) {
                    platform_debug::TracePrinter::printfTrace("Update Success: %uB\n", index + len);
                } else {
                    Update.printError(Serial);
                }
            }
        });
        AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/postSTLB", [](AsyncWebServerRequest * request, JsonVariant & json) {
            JsonObject jsonObj = json.as<JsonObject>();
            platform_debug::TracePrinter::printfTrace( "check jsonObj:%s\n", json.as<String>().c_str());
        
            String text=json.as<String>();
            
            if(FFatHelper::writeFile(FFat,user_properties::path.c_str(),text) ){
                user_properties::ssid =  jsonObj["ssid"].as<String>();
                user_properties::pass =  jsonObj["pass"].as<String>();
                user_properties::host =  jsonObj["host"].as<String>(); 
                user_properties::port =  jsonObj["port"].as<int>();
            }else{
                DynamicJsonDocument  doc(200);
                doc["ssid"] = "n/a";
                doc["pass"] = "n/a";
                doc["host"] = "n/a";
                doc["port"] = 0;
                text=doc.as<String>();
            }
            platform_debug::TracePrinter::printTrace(text);
            request->send(200, "application/json",text.c_str());
        });
        
        _server.addHandler(handler);
        _wss.onEvent(std::bind(&ESPWebServer::onWsEvent,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6) );
        _server.addHandler(&_wss);
       
        _events.onConnect([](AsyncEventSourceClient* client) {
            if (client->lastId()) {
                platform_debug::TracePrinter::printfTrace("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
            }
            //send event with message "hello!", id current millis
            // and set reconnect delay to 1 second
            if(client!=nullptr){
                // client->send("Welcome...", NULL, millis(), 1000);
            }else{
                 platform_debug::TracePrinter::printTrace("Returned :No Client connected!\n");
            }
        });
     
        _events.setAuthentication(web_properties::http_user.c_str(), web_properties::http_pass.c_str());
        _server.addHandler(&_events);

        _server.begin();

        _thread = std::thread(&ESPWebServer::run_web_server,this);    
}


void ESPWebServer::run_web_server(){
        while(true){
            osEvent evt= _mail_box.get();
            if (evt.status == osEventMail) {
                web_server::mail_t *mail = (web_server::mail_t *)evt.value.p;
                _mail_box.free(mail); 
            }
           // _wss.cleanupClients();
            platform_debug::TracePrinter::printTrace("Thread:ws_wss.cleanupClients()\n");
        } 
}

void ESPWebServer::post_mail(int message){
   std::unique_lock<std::mutex> _lock(_mtxMailBox, std::defer_lock);
   _lock.lock();
    web_server::mail_t *mail = _mail_box.alloc();
    if(mail!=NULL){
        mail->message = message;
        _mail_box.put(mail) ;
     }
}

void ESPWebServer::delegateMethodWebSocketClientPostEvent(const String& message, const String& event, uint32_t id, uint32_t reconnect){
   std::unique_lock<std::mutex> _lock(_mtxEventSource, std::defer_lock);
   _lock.lock();
    _events.send(message.c_str(),event.c_str(),id, reconnect);
}

void ESPWebServer::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient *client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT) {
        platform_debug::TracePrinter::printfTrace("ws[%s][%u] connect\n", server->url(), client->id());

        DynamicJsonDocument  doc(200);
        doc["box_mac_id"] = web_properties::ap_ssid;
        doc["ws_evt_type"] = "WS_EVT_CONNECT";
        doc["r_offset"] = rgb_properties::r_offset;
        doc["g_offset"] = rgb_properties::g_offset;
        doc["b_offset"] = rgb_properties::b_offset;
    
        client->text(doc.as<String>().c_str());
        client->ping();
        delay(200);
    } else if (type == WS_EVT_DISCONNECT) {
        platform_debug::TracePrinter::printfTrace("ws[%s][%u] disconnect\n", server->url(), client->id());
        post_mail(0);
    } else if (type == WS_EVT_ERROR) {
        platform_debug::TracePrinter::printfTrace("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } else if (type == WS_EVT_PONG) {
        platform_debug::TracePrinter::printfTrace("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len) {
        //the whole message is in a single frame and we got all of it's data
            platform_debug::TracePrinter::printfTrace("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

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
            platform_debug::TracePrinter::printTrace("==============================="+String(client->id(),DEC));
            platform_debug::TracePrinter::printTrace(msg);
            if (!error) //检查反序列化是否成功
            {
                if (doc["msg"].as<String>() == String("rgb_offset")) {
                    runCallbackPostMailToCollector(MeasEventType::EventSystemOffset,client->id());
                } else if (doc["msg"].as<String>() == String("rgb_measure")) {
                    runCallbackPostMailToCollector(MeasEventType::EventWebAppMeasure,client->id());
                }
            }
        }
    }
}

void ESPWebServer::notFound(AsyncWebServerRequest *request) {
    platform_debug::TracePrinter::printTrace("NOT_FOUND: ");
            if(request->method() == HTTP_GET)
                platform_debug::TracePrinter::printTrace("GET");
            else if(request->method() == HTTP_POST)
                platform_debug::TracePrinter::printTrace("POST");
            else if(request->method() == HTTP_DELETE)
                platform_debug::TracePrinter::printTrace("DELETE");
            else if(request->method() == HTTP_PUT)
                platform_debug::TracePrinter::printTrace("PUT");
            else if(request->method() == HTTP_PATCH)
                platform_debug::TracePrinter::printTrace("PATCH");
            else if(request->method() == HTTP_HEAD)
                ::TracePrinter::printTrace("HEAD");
            else if(request->method() == HTTP_OPTIONS)
                platform_debug::TracePrinter::printTrace("OPTIONS");
            else
                platform_debug::TracePrinter::printTrace("UNKNOWN");
                platform_debug::TracePrinter::printTrace("http://"+String( request->host().c_str()));
                platform_debug::TracePrinter::printTrace("http://"+String( request->url().c_str()));
            if(request->contentLength()){
                platform_debug::TracePrinter::printTrace("_CONTENT_TYPE: "+String(request->contentType().c_str()));
                platform_debug::TracePrinter::printTrace("_CONTENT_LENGTH: "+String(request->contentLength(),DEC));
            }

            int headers = request->headers();
            int i;
            for(i=0;i<headers;i++){
            AsyncWebHeader* h = request->getHeader(i);
                platform_debug::TracePrinter::printfTrace("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
            }

            int params = request->params();
            for(i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isFile()){
                platform_debug::TracePrinter::printfTrace("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if(p->isPost()){
                platform_debug::TracePrinter::printfTrace("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                platform_debug::TracePrinter::printfTrace("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
            }

            request->send(404);
}