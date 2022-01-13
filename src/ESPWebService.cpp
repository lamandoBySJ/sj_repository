#include "ESPWebService.h"

extern FFatHelper<rtos::Mutex> FatHelper;

void ESPWebService::delegateMethodWebSocketClientText(AsyncWebSocketClient *client,const String& text){
    client->text(text);
}

void ESPWebService::delegateMethodWebSocketClientEvent(const String& message, const String& event, uint32_t id, uint32_t reconnect){
    _events->send(message.c_str(),event.c_str(),id, reconnect);
}


void ESPWebService::run_web_service()
{
    TracePrinter::printTrace(platformio_api::get_web_properties().ap_ssid);
    TracePrinter::printTrace(platformio_api::get_web_properties().ap_pass);

    ThisThread::flags_wait_all(0x0);
    
   
     _dnsServer->start(80, "*", WiFi.softAPIP());
    //server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
     //MDNS.addService("http","tcp",80);
    _server->addHandler(new SPIFFSEditor(FFat, platformio_api::get_web_properties().http_user,platformio_api::get_web_properties().http_pass));
    _server->on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });
    _server->serveStatic("/", FFat, "/data").setDefaultFile("index.html");
 
    // _server->onNotFound([](AsyncWebServerRequest *request){});
    _server->onNotFound(std::bind(&ESPWebService::notFound,this,std::placeholders::_1));
    _server->on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
            if(request->method() == HTTP_GET)
                TracePrinter::printTrace("GET");
            else if(request->method() == HTTP_POST)
                TracePrinter::printTrace("POST");
            else if(request->method() == HTTP_DELETE)
                TracePrinter::printTrace("DELETE");
            else if(request->method() == HTTP_PUT)
                TracePrinter::printTrace("PUT");
            else if(request->method() == HTTP_PATCH)
                TracePrinter::printTrace("PATCH");
            else if(request->method() == HTTP_HEAD)
                TracePrinter::printTrace("HEAD");
            else if(request->method() == HTTP_OPTIONS)
                TracePrinter::printTrace("OPTIONS");
            else
                TracePrinter::printTrace("UNKNOWN");

            TracePrinter::printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

            if(request->contentLength()){
                TracePrinter::printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
                TracePrinter::printf("_CONTENT_LENGTH: %u\n", request->contentLength());
            }
            /*
            int headers = request->headers();
            int i;
             for(i=0;i<headers;i++){
                AsyncWebHeader* h = request->getHeader(i);
               // Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
               // TracePrinter::printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
            }
           
            int params = request->params();
                for(i=0;i<params;i++){
                AsyncWebParameter* p = request->getParam(i);
                if(p->isFile()){
                  //  TracePrinter::printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
                } else if(p->isPost()){
                 //   TracePrinter::printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                } else {
                  //  TracePrinter::printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
                }
            }*/
            request->send(200, "text/plain", "OK");
        },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
                String path = String("/data/")+filename;
                TracePrinter::printf("--Upload:path: %s\n", path.c_str());
                if(index==0){
                    TracePrinter::printf("--Upload:W: %s\n", filename.c_str());
                  //  FatHelper.writeFile(FFat,path,data,len);
                }else{
                    TracePrinter::printf("--Upload:Append: %s\n", filename.c_str());
                 //   FatHelper.appendFile(FFat,path,data,len); 
                }
                
        },[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            if(!index)
                TracePrinter::printf("BodyStart: %u\n", total);
                TracePrinter::printf("%s", (const char*)data);
            if(index + len == total)
                TracePrinter::printf("BodyEnd: %u\n", total);
    });

 


    _server->on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html",platformio_api::get_web_properties().server_upload_url);
    });
        
    _server->on("/getSTLB", HTTP_GET, [&](AsyncWebServerRequest *request) {
            DynamicJsonDocument  _docucment(600);
            _docucment["box_mac_id"] = platformio_api::get_web_properties().ap_ssid;
            _docucment["ssid"] = User::getProperties().ssid;
            _docucment["pass"] = User::getProperties().pass;
            _docucment["host"] = User::getProperties().host;
            _docucment["port"] = User::getProperties().port;
            request->send(200, "application/json", _docucment.as<String>());
    });   
     
    _server->on("/reboot", HTTP_POST, [] (AsyncWebServerRequest *request) {
            if (request->hasParam("reboot")) {
                TracePrinter::printTrace(request->getParam("reboot")->value());
            }
            request->send(200, "text/plain", "OK:Reboot Now!" );
            ThisThread::sleep_for(Kernel::Clock::duration_seconds(1));
            ESP.restart();
    });
    
    _server->on("/als", HTTP_GET, [] (AsyncWebServerRequest *request) {
        request->send(FFat, "/data/als.html", "text/html");
    });
        
    _server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
    });

    _server->on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
            bool result = Update.hasError();
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", result ? "FAIL" : "OK");
            response->addHeader("Connection", "close");
            request->send(response);
    }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
                if (!index) {
                    TracePrinter::printf("Update Start: %s\n", filename.c_str());
                    TracePrinter::printf("ESP.getFreeSketchSpace(): %x\n", ESP.getFreeSketchSpace());
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
                        TracePrinter::printf("Update Success: %uB\n", index + len);
                    } else {
                        Update.printError(Serial);
                    }
                }
    });
    
    _handler = new AsyncCallbackJsonWebHandler("/postSTLB", [](AsyncWebServerRequest * request, JsonVariant & json) {
            JsonObject jsonObj = json.as<JsonObject>();
            TracePrinter::printf( "check jsonObj:%s\n", json.as<String>().c_str());
        
            String text=json.as<String>();
            
            if(FatHelper.writeFile(FFat,User::getProperties().path.c_str(),text) ){
                User::getProperties().ssid =  jsonObj["ssid"].as<String>();
                User::getProperties().pass =  jsonObj["pass"].as<String>();
                User::getProperties().host =  jsonObj["host"].as<String>(); 
                User::getProperties().port =  jsonObj["port"].as<int>();
            }else{
                DynamicJsonDocument  doc(200);
                doc["ssid"] = "n/a";
                doc["pass"] = "n/a";
                doc["host"] = "n/a";
                doc["port"] = 0;
                text=doc.as<String>();
            }
            TracePrinter::printTrace(text);
            request->send(200, "application/json",text);
    });
        
  
     
    _server->addHandler(_handler);
    _wss->onEvent(std::bind(&ESPWebService::onWsEvent,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6) );
    _server->addHandler(_wss);

    _events->setAuthentication(platformio_api::get_web_properties().http_user.c_str(), platformio_api::get_web_properties().http_pass.c_str()); 
    _events->onConnect([](AsyncEventSourceClient* client) {
            if (client->lastId()) {
                TracePrinter::printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
            }
            //send event with message "hello!", id current millis
            // and set reconnect delay to 1 second
            if(client!=nullptr){
                // client->send("Welcome...", NULL, millis(), 1000);
            }else{
                 TracePrinter::printTrace("Returned :No Client connected!\n");
            }
    });

    _server->addHandler(_events);

    _server->begin(); 
    
    TracePrinter::printTrace("----------------- WEB Server is running... -----------------");
    
    while(true){
        _dnsServer->processNextRequest();
        _wss->cleanupClients();
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(10));
    }
}  

void ESPWebService::notFound(AsyncWebServerRequest *request) {
    TracePrinter::printTrace("NOT_FOUND: ");
            if(request->method() == HTTP_GET){
                TracePrinter::printTrace("GET");
                request->send(404);
            }
            else if(request->method() == HTTP_POST)
                TracePrinter::printTrace("POST");
            else if(request->method() == HTTP_DELETE)
                TracePrinter::printTrace("DELETE");
            else if(request->method() == HTTP_PUT)
                TracePrinter::printTrace("PUT");
            else if(request->method() == HTTP_PATCH)
                TracePrinter::printTrace("PATCH");
            else if(request->method() == HTTP_HEAD)
                TracePrinter::printTrace("HEAD");
            else if(request->method() == HTTP_OPTIONS)
                TracePrinter::printTrace("OPTIONS");
            else
                TracePrinter::printTrace("UNKNOWN");
            /*
            TracePrinter::printTrace("http://"+String( request->host().c_str()));
            TracePrinter::printTrace("http://"+String( request->url().c_str()));
            if(request->contentLength()){
                TracePrinter::printTrace("_CONTENT_TYPE: "+String(request->contentType().c_str()));
                TracePrinter::printTrace("_CONTENT_LENGTH: "+String(request->contentLength(),DEC));
            }

            int headers = request->headers();
            int i;
            for(i=0;i<headers;i++){
            AsyncWebHeader* h = request->getHeader(i);
                TracePrinter::printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
            }

            int params = request->params();
            for(i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isFile()){
                TracePrinter::printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if(p->isPost()){
                TracePrinter::printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                TracePrinter::printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
            }
            */
            
}
void ESPWebService::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient *client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    for(auto& call:this->_callbacks){
        call(server,client,type,arg,data,len);
    }
}
