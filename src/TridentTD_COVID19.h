#ifndef __TRIDENTTD_COVID19_H__
#define __TRIDENTTD_COVID19_H__

#include <Arduino.h>
#include <time.h>

#if defined(ESP8266)
  #include <ESP8266WiFi.h>  
  #include <ESP8266HTTPClient.h>
#elif defined (ESP32)
  #include <WiFi.h>
  #include <HTTPClient.h>
#endif

#define RELOAD_INTERVAL     15   // minutes


typedef struct _covid19{
  char      date[11];
  time_t    timestamp;
  uint16_t  cases;
  uint16_t  deaths;
} covid19_t;


class TridentTD_COVID19{
  public:
    TridentTD_COVID19();

    void   begin(String country="thailand");
    bool   available();

    static size_t getHistory(String country = "thailand");
    static void   getToday(String country = "thailand");

    size_t    size();
    covid19_t operator[](uint16_t index);


    void       country(String Country);
    String     country();
    const char* current_date();
    uint16_t  cases(String date="");   // eg. date :  "24/03/2020" 
    uint16_t  deaths(String date="");  // eg. date :  "24/03/2020" 
    uint16_t  todayCases();
    uint16_t  todayDeaths();
    uint16_t  recovered();
    uint16_t  active();
    uint16_t  critical();
    uint16_t  casesPerOneMillion();
    uint16_t  deathsPerOneMillion();
};

extern TridentTD_COVID19 COVID19;

#endif //__TRIDENTTD_COVID19_H__
