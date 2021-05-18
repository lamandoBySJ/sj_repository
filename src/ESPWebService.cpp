#include "ESPWebService.h"
#include <DNSServer.h>
DNSServer dnsServer;
AsyncWebServer server(80);

using namespace web_server;

web_server::WebProperties ESPWebService::webProperties;

void ESPWebService::startup(){
    _thread.start(callback(this,&ESPWebService::run_web_service));
}
void ESPWebService::shutdown(){
     _thread.terminate();
} 
void ESPWebService::run_web_service()
{
    running =true;
   
     WiFi.softAP(ESPWebService::webProperties.ap_ssid.c_str(), ESPWebService::webProperties.ap_pass.c_str());
    dnsServer.start(53, "*", WiFi.softAPIP());
    //server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
    
     //MDNS.addService("http","tcp",80);
    server.addHandler(new SPIFFSEditor(FFat, ESPWebService::webProperties.http_user,ESPWebService::webProperties.http_pass));
    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });
    server.serveStatic("/", FFat, "/data").setDefaultFile("index.html");
    server.onNotFound(std::bind(&ESPWebService::notFound,this,std::placeholders::_1));
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
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
                platform_debug::TracePrinter::printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

            if(request->contentLength()){
                platform_debug::TracePrinter::printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
                platform_debug::TracePrinter::printf("_CONTENT_LENGTH: %u\n", request->contentLength());
            }

            int headers = request->headers();
            int i;
            for(i=0;i<headers;i++){
                AsyncWebHeader* h = request->getHeader(i);
                platform_debug::TracePrinter::printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
            }

            int params = request->params();
                for(i=0;i<params;i++){
                AsyncWebParameter* p = request->getParam(i);
                if(p->isFile()){
                    platform_debug::TracePrinter::printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
                } else if(p->isPost()){
                    platform_debug::TracePrinter::printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
                } else {
                    platform_debug::TracePrinter::printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
                }
            }
            request->send(200, "text/plain", "OK");
        },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
                String path = String("/data/")+filename;
                platform_debug::TracePrinter::printf("Upload:path: %s\n", path.c_str());
                if(index==0){
                    platform_debug::TracePrinter::printf("Upload:W: %s\n", filename.c_str());
                    FFatHelper::writeFile(FFat,path,data,len);
                }else{
                    platform_debug::TracePrinter::printf("Upload:Append: %s\n", filename.c_str());
                    FFatHelper::appendFile(FFat,path,data,len); 
                }
                
        },[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
            if(!index)
                platform_debug::TracePrinter::printf("BodyStart: %u\n", total);
                platform_debug::TracePrinter::printf("%s", (const char*)data);
            if(index + len == total)
                platform_debug::TracePrinter::printf("BodyEnd: %u\n", total);
    });

 


    server.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html",ESPWebService::webProperties.server_upload_url);
    });
        
    server.on("/getSTLB", HTTP_GET, [&](AsyncWebServerRequest *request) {
            DynamicJsonDocument  _docucment(600);
            _docucment["box_mac_id"] = ESPWebService::webProperties.ap_ssid;
            _docucment["ssid"] = MQTTNetwork::userProperties.ssid;
            _docucment["pass"] = MQTTNetwork::userProperties.pass;
            _docucment["host"] = MQTTNetwork::userProperties.host;
            _docucment["port"] = MQTTNetwork::userProperties.port;
            request->send(200, "application/json", _docucment.as<String>());
    });   
     
    server.on("/reboot", HTTP_POST, [] (AsyncWebServerRequest *request) {
            if (request->hasParam("reboot")) {
                platform_debug::TracePrinter::printTrace(request->getParam("reboot")->value());
            }
            request->send(200, "text/plain", "OK:Reboot Now!" );
            ThisThread::sleep_for(Kernel::Clock::duration_seconds(3));
            esp_restart();
    });
    
    server.on("/als", HTTP_GET, [] (AsyncWebServerRequest *request) {
        request->send(FFat, "/data/als.html", "text/html");
    });
        
    server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
    });

    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool result = Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", result ? "OK" : "FAIL");
        response->addHeader("Connection", "close");
        request->send(response);
        }, [](AsyncWebServerRequest * request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                platform_debug::TracePrinter::printf("Update Start: %s\n", filename.c_str());
                platform_debug::TracePrinter::printf("ESP.getFreeSketchSpace(): %x\n", ESP.getFreeSketchSpace());
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
                    platform_debug::TracePrinter::printf("Update Success: %uB\n", index + len);
                } else {
                    Update.printError(Serial);
                }
            }
        }
    );
     



       
    _handler = new AsyncCallbackJsonWebHandler("/postSTLB", [](AsyncWebServerRequest * request, JsonVariant & json) {
            JsonObject jsonObj = json.as<JsonObject>();
            platform_debug::TracePrinter::printf( "check jsonObj:%s\n", json.as<String>().c_str());
        
            String text=json.as<String>();
            
            if(FFatHelper::writeFile(FFat,MQTTNetwork::userProperties.path.c_str(),text) ){
                MQTTNetwork::userProperties.ssid =  jsonObj["ssid"].as<String>();
                MQTTNetwork::userProperties.pass =  jsonObj["pass"].as<String>();
                MQTTNetwork::userProperties.host =  jsonObj["host"].as<String>(); 
                MQTTNetwork::userProperties.port =  jsonObj["port"].as<int>();
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
        
  
     
    server.addHandler(_handler);
    _wss.onEvent(std::bind(&ESPWebService::onWsEvent,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3,std::placeholders::_4,std::placeholders::_5,std::placeholders::_6) );
    server.addHandler(&_wss);

    _events.setAuthentication(ESPWebService::webProperties.http_user.c_str(), ESPWebService::webProperties.http_pass.c_str()); 
    _events.onConnect([](AsyncEventSourceClient* client) {
            if (client->lastId()) {
                platform_debug::TracePrinter::printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
            }
            //send event with message "hello!", id current millis
            // and set reconnect delay to 1 second
            if(client!=nullptr){
                // client->send("Welcome...", NULL, millis(), 1000);
            }else{
                 platform_debug::TracePrinter::printTrace("Returned :No Client connected!\n");
            }
    });

    server.addHandler(&_events);
   
    server.begin(); 
    

    platform_debug::TracePrinter::printTrace("WEB Server is running...\n"); 


    while(1){
        dnsServer.processNextRequest();
        _wss.cleanupClients();
        ThisThread::sleep_for(Kernel::Clock::duration_milliseconds(100));
    }
    // platform_debug::TracePrinter::printTrace("Thread:ws_wss.cleanupClients()\n");
}  
void ESPWebService::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient *client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT) {
        platform_debug::TracePrinter::printf("ws[%s][%u] connect\n", server->url(), client->id());
        DynamicJsonDocument  doc(200);
        doc["box_mac_id"] = ESPWebService::webProperties.ap_ssid;
        doc["ws_evt_type"] = "WS_EVT_CONNECT";
        doc["r_offset"] = ColorCollector::rgb_properties.r_offset;
        doc["g_offset"] = ColorCollector::rgb_properties.g_offset;
        doc["b_offset"] = ColorCollector::rgb_properties.b_offset;
        client->text(doc.as<String>().c_str());
        client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
        platform_debug::TracePrinter::printf("ws[%s][%u] disconnect\n", server->url(), client);
    } else if (type == WS_EVT_ERROR) {
        platform_debug::TracePrinter::printf("ws[%s][%u] error(%u): %s\n", server->url(), client, *((uint16_t*)arg), (char*)data);
    } else if (type == WS_EVT_PONG) {
        platform_debug::TracePrinter::printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client, len, (len) ? (char*)data : "");
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo * info = (AwsFrameInfo*)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len) {
        //the whole message is in a single frame and we got all of it's data
            platform_debug::TracePrinter::printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

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
            platform_debug::TracePrinter::printTrace(msg);
            if (!error) //检查反序列化是否成功
            {
                if (doc["msg"].as<String>() == String("rgb_offset")) {
                    runCallbackPostMailToCollector(MeasEventType::EventSystemOffset,client);
                } else if (doc["msg"].as<String>() == String("rgb_measure")) {
                    runCallbackPostMailToCollector(MeasEventType::EventWebAppMeasure,client);
                }
            }
        }
    }
}

void ESPWebService::notFound(AsyncWebServerRequest *request) {
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
                platform_debug::TracePrinter::printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
            }

            int params = request->params();
            for(i=0;i<params;i++){
            AsyncWebParameter* p = request->getParam(i);
            if(p->isFile()){
                platform_debug::TracePrinter::printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if(p->isPost()){
                platform_debug::TracePrinter::printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                platform_debug::TracePrinter::printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
            }

            request->send(404);
}
