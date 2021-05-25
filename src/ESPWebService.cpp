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
    TracePrinter::printTrace(Platform::getWebProperties().ap_ssid);
    TracePrinter::printTrace(Platform::getWebProperties().ap_pass);
    WiFi.mode(WIFI_OFF);
    
    WiFi.mode(WIFI_AP_STA);
    WiFi.enableAP(true);
    ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(1000));
    WiFi.softAP(Platform::getWebProperties().ap_ssid.c_str(), Platform::getWebProperties().ap_pass.c_str());
    _dnsServer->start(53, "*", WiFi.softAPIP());
    //server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    
     //MDNS.addService("http","tcp",80);
    _server->addHandler(new SPIFFSEditor(FFat, Platform::getWebProperties().http_user,Platform::getWebProperties().http_pass));
    _server->on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });
    _server->serveStatic("/", FFat, "/data").setDefaultFile("index.html");
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
            request->send(200, "text/plain", "OK");
        },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
                String path = String("/data/")+filename;
                TracePrinter::printf("Upload:path: %s\n", path.c_str());
                if(index==0){
                    TracePrinter::printf("Upload:W: %s\n", filename.c_str());
                    FatHelper.writeFile(FFat,path,data,len);
                }else{
                    TracePrinter::printf("Upload:Append: %s\n", filename.c_str());
                    FatHelper.appendFile(FFat,path,data,len); 
                }
                
        },[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            if(!index)
                TracePrinter::printf("BodyStart: %u\n", total);
                TracePrinter::printf("%s", (const char*)data);
            if(index + len == total)
                TracePrinter::printf("BodyEnd: %u\n", total);
    });

 


    _server->on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html",Platform::getWebProperties().server_upload_url);
    });
        
    _server->on("/getSTLB", HTTP_GET, [&](AsyncWebServerRequest *request) {
            DynamicJsonDocument  _docucment(600);
            _docucment["box_mac_id"] = Platform::getWebProperties().ap_ssid;
            _docucment["ssid"] = Platform::getUserProperties().ssid;
            _docucment["pass"] = Platform::getUserProperties().pass;
            _docucment["host"] = Platform::getUserProperties().host;
            _docucment["port"] = Platform::getUserProperties().port;
            request->send(200, "application/json", _docucment.as<String>());
    });   
     
    _server->on("/reboot", HTTP_POST, [] (AsyncWebServerRequest *request) {
            if (request->hasParam("reboot")) {
                TracePrinter::printTrace(request->getParam("reboot")->value());
            }
            request->send(200, "text/plain", "OK:Reboot Now!" );
            ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
            esp_restart();
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
        }
    );
    
    _handler = new AsyncCallbackJsonWebHandler("/postSTLB", [](AsyncWebServerRequest * request, JsonVariant & json) {
            JsonObject jsonObj = json.as<JsonObject>();
            TracePrinter::printf( "check jsonObj:%s\n", json.as<String>().c_str());
        
            String text=json.as<String>();
            
            if(FatHelper.writeFile(FFat,Platform::getUserProperties().path.c_str(),text) ){
                Platform::getUserProperties().ssid =  jsonObj["ssid"].as<String>();
                Platform::getUserProperties().pass =  jsonObj["pass"].as<String>();
                Platform::getUserProperties().host =  jsonObj["host"].as<String>(); 
                Platform::getUserProperties().port =  jsonObj["port"].as<int>();
            }else{
                DynamicJsonDocument  doc(200);
                doc["ssid"] = "n/a";
                doc["pass"] = "n/a";
                doc["host"] = "n/a";
                doc["port"] = 0;
                text=doc.as<String>();
            }
            TracePrinter::printTrace(text);
            request->send(200, "application/json",text.c_str());
    });
        
  
     
    _server->addHandler(_handler);
    _wss->onEvent(std::bind(&ESPWebService::onWsEvent,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6) );
    _server->addHandler(_wss);

    _events->setAuthentication(Platform::getWebProperties().http_user.c_str(), Platform::getWebProperties().http_pass.c_str()); 
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

void ESPWebService::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient *client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT) {
        TracePrinter::printf("ws[%s][%u] connect\n", server->url(), client->id());
        DynamicJsonDocument  doc(200);
        doc["box_mac_id"] = Platform::getWebProperties().ap_ssid;
        doc["ws_evt_type"] = "WS_EVT_CONNECT";
        doc["r_offset"] = Platform::getRGBProperties().r_offset;
        doc["g_offset"] = Platform::getRGBProperties().g_offset;
        doc["b_offset"] = Platform::getRGBProperties().b_offset;
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
                    runCallbackPostMailToCollector(MeasEventType::EventWebAppOffset,client);
                } else if (doc["msg"].as<String>() == String("rgb_measure")) {
                    runCallbackPostMailToCollector(MeasEventType::EventWebAppMeasure,client);
                }
            }
        }
    }
}

void ESPWebService::notFound(AsyncWebServerRequest *request) {
    TracePrinter::printTrace("NOT_FOUND: ");
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
                ::TracePrinter::printTrace("HEAD");
            else if(request->method() == HTTP_OPTIONS)
                TracePrinter::printTrace("OPTIONS");
            else
                TracePrinter::printTrace("UNKNOWN");
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

            request->send(404);
}
