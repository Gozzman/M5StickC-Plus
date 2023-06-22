#include <M5StickCPlus.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>


#define DEBUG 1

struct ErrorConfig_t{
    int NO_WIFI = 0;
    int NO_RESPONSE = 1;
    int ALL_OK = 2;
};

ErrorConfig_t errConfig;

const char* SSID = "MTS_KV21";
const char* PASSWORD = "89255427507";
const String REG_ID = "213";
const String PATH = "https://yandex.com/time/sync.json?geo=";
String REQ_PATH = PATH + REG_ID;

StaticJsonDocument<1000> json;
WiFiClient client;
HTTPClient http;




int get_data(){
    
    if(WiFi.status() != WL_CONNECTED){
       Serial.println(F("No WiFi connection"));
       return errConfig.NO_WIFI;
    }

    http.begin(client, REQ_PATH);
    int get = http.GET();

    if(get <= 0){
        Serial.println(F("No response"));
        return errConfig.NO_RESPONSE;
    }

    String response = http.getString();
    DeserializationError err = deserializeJson(json, response);

    if (err) {Serial.println(String(err.c_str()));}
    else {Serial.println(F("DeserializationError == OK"));}

    JsonObject obj = json["clocks"]["213"];
    const char* sunrise = obj["sunrise"];
    const char* sunset = obj["sunset"];
    JsonObject obj_weather = obj["weather"];
    int temperature = obj_weather["temp"];

    #if DEBUG == 1
        Serial.println(sunrise);
        Serial.println(sunset);
        Serial.println(temperature);
    #endif
    http.end();

    return errConfig.ALL_OK;
}



void setup() {
    M5.begin();
    Serial.begin(115200);
    WiFi.begin(SSID, PASSWORD);
    Serial.println(F("Connecting"));

    while (WiFi.status() != WL_CONNECTED)
    {
       Serial.println('.');
       delay(500);
    }

    Serial.println(F("Connected"));

}
void loop() {

    if(M5.BtnA.wasPressed()){
        get_data();
    }
    M5.update();
}
