#include <M5StickCPlus.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <time.h>
#include <ArduinoJson.h>
#include <WiFi.h>


#define DEBUG 0

struct ErrorConfig_t{
    int NO_WIFI = 0;
    int NO_RESPONSE = 1;
    int ALL_OK = 2;
};

ErrorConfig_t errConfig;

#define GMT +3
const int GMT_SEC = GMT * 3600; 
const char* ntp_server = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const char* SSID = "MTS_KV21";
const char* PASSWORD = "89255427507";
const String REG_ID = "213";
const String PATH = "https://yandex.com/time/sync.json?geo=";
String REQ_PATH = PATH + REG_ID;

StaticJsonDocument<1000> json;
WiFiClient client;
HTTPClient http;
RTC_TimeTypeDef time_struct;
RTC_DateTypeDef date_struct;


float get_voltage(){
    return M5.Axp.GetBatVoltage();
}



void set_time_RTC(struct tm *time){
    
    time_struct.Hours = time -> tm_hour;
    time_struct.Minutes = time -> tm_min;
    time_struct.Seconds = time -> tm_sec;
    date_struct.Date = time ->tm_mday;
    date_struct.Month = time ->tm_mon + 1;
    date_struct.WeekDay =  time ->tm_wday;
    date_struct.Year = time ->tm_year - 100;
    M5.Rtc.SetTime(&time_struct);
    M5.Rtc.SetData(&date_struct);
   

}

void get_time(){
struct tm time;
  if(!getLocalTime(&time)){
    Serial.println("Connection failed");
    return;
  }
  #if DEBUG == 1
  Serial.println(&time, "%A, %B %d %Y %H:%M:%S");
  Serial.print("tm_hour: "); Serial.println(time.tm_hour); 
  Serial.print("tm_isdst: ");Serial.println(time.tm_isdst);
  Serial.print("tm_mday: ");Serial.println(time.tm_mday); 
  Serial.print("tm_min: ");Serial.println(time.tm_min);
  Serial.print("tm_mon: ");Serial.println(time.tm_mon); 
  Serial.print("tm_sec: ");Serial.println(time.tm_sec);
  Serial.print("tm_wday: ");Serial.println(time.tm_wday); 
  Serial.print("tm_yday: ");Serial.println(time.tm_yday);
  Serial.print("tm_year: ");Serial.println(time.tm_year);
  #endif 
  set_time_RTC(&time);
}

int get_weather_data(){
    
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
   
    configTime(gmtOffset_sec, GMT_SEC, ntp_server);
    get_time();
    Serial.println(F("Connected"));
}

void loop() {
    
    if(M5.BtnA.wasPressed()){
       get_weather_data();
       M5.Rtc.GetData(&date_struct);
       M5.Rtc.GetTime(&time_struct);
    }

    M5.update();
}
