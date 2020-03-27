#include "TridentTD_COVID19.h"
#include <vector>
#include <Ticker.h>

typedef struct _covid19_today{
  uint16_t  cases;
  uint16_t  todayCases;
  uint16_t  deaths;
  uint16_t  todayDeaths;
  uint16_t  recovered;
  uint16_t  active;
  uint16_t  critical;
  uint16_t  casesPerOneMillion;
  uint16_t  deathsPerOneMillion;
} covid19_today_t;


static HTTPClient       _http;

static covid19_t covid19null;
static std::vector<covid19_t> covid_list;
static Ticker           _covid_ticker;
static String           _country;
static covid19_today_t  _today_data;
static bool             _avail = false;

TridentTD_COVID19 COVID19;

TridentTD_COVID19::TridentTD_COVID19(){
  covid_list.clear();
  
  covid19null.timestamp = 0;
  covid19null.cases     = 0;
  covid19null.deaths    = 0;

  memset(&_today_data, 0, sizeof(_today_data));
}

void covid19_request(){
  if(!WiFi.isConnected()) return;
  TridentTD_COVID19::getHistory(_country);
  if( covid_list.size()) {
    _covid_ticker.attach_ms(RELOAD_INTERVAL*60000, covid19_request);
  }
}

void  TridentTD_COVID19::begin(String country){
  _country = country;
  _covid_ticker.attach_ms(5000L, covid19_request);
}

size_t TridentTD_COVID19::getHistory(String country ){
  if(!WiFi.isConnected()) return 0;
  // this->_country = country;
  Serial.println("covid19 request...");

#if defined(ESP8266)
  // ยังไม่ work
  _http.begin( String("https://corona.lmao.ninja/v2/historical/") + country, "‎7b 00 29 73 45 13 a3 f7 95 84 42 ea e0 28 1b f4 0d b8 11 53"); 
#elif defined (ESP32)
  _http.begin( String("https://corona.lmao.ninja/v2/historical/") + country );
#endif
  
  int http_code = _http.GET();
  // Serial.println(http_code);
  if( http_code == 200) {
    WiFiClient& client = _http.getStream();
    
    if(client.available()){
      client.find("\"cases\":{");
      covid_list.clear();
      String raw_data;
      bool finished = false;
      bool deaths_loop = false;

      while(client.available()){
        String raw_data = client.readStringUntil(','); raw_data.trim();
        if(raw_data[raw_data.length()-1] == '}') { raw_data.replace("}", ""); finished = true; }

        int d, m, y, cnt;
        sscanf(raw_data.c_str(), "\"%d/%d/%d\":%d", &m, &d, &y, &cnt);

        y += 2000;
        if( y < 2018 ) continue;
        
        covid19_t new_covid;
        
        struct tm info;
          info.tm_year  = y - 1900;
          info.tm_mon   = m - 1;
          info.tm_mday  = d;
          info.tm_hour  = 0;
          info.tm_min   = 0;
          info.tm_sec   = 0;
          info.tm_isdst = 0;
        snprintf(new_covid.date, sizeof(new_covid.date), "%02d/%02d/%04d", d, m, y );

        time_t cur_timestamp = mktime(&info);
        if(!deaths_loop) {
          new_covid.timestamp = cur_timestamp;
          new_covid.cases     = cnt;
          covid_list.push_back( new_covid);
        }else{
          for(int i=0; i < covid_list.size(); i++){
            if(covid_list[i].timestamp == cur_timestamp ){
              covid_list[i].deaths = cnt;
            }
          }
        }

        
        if(finished) { 
          if( !deaths_loop) {
            deaths_loop  = true;
            finished     = false;
            client.find("\"deaths\":{");
//            Serial.println("------------------\ndeaths");
          }
        }
      }
      
    }
    // Serial.println(covid_list.size());
    TridentTD_COVID19::getToday(country);
    if(covid_list.size()) {

      if( (_today_data.cases != 0 && _today_data.deaths != 0) &&
          (_today_data.cases != covid_list[covid_list.size()-1].cases || 
           _today_data.deaths != covid_list[covid_list.size()-1].deaths ) )
      {
        time_t ts = covid_list[covid_list.size()-1].timestamp + 86400;
        struct tm tm;
        localtime_r(&ts, &tm);
        covid19_t new_covid;
        snprintf(new_covid.date, sizeof(new_covid.date), "%02d/%02d/%04d", tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900 );
        new_covid.timestamp = ts;
        new_covid.cases     = _today_data.cases;
        new_covid.deaths    = _today_data.deaths;
        covid_list.push_back( new_covid);
      }
    }

    _http.end();
    if( covid_list.size() > 0) _avail = true;
    return covid_list.size();
  }else{
    Serial.println("_http can't connected");
    _http.end();
    return 0;
  }
}

bool TridentTD_COVID19::available(){
  // Serial.println(_avail);
  if( _avail ) { _avail = false; return true; }

  return false;
}

size_t TridentTD_COVID19::size(){
  return covid_list.size();
}

covid19_t TridentTD_COVID19::operator[](uint16_t index) {
  if(index >= covid_list.size()) return covid19null;

  return covid_list[index];
}

uint16_t TridentTD_COVID19::cases(String date){
  if( covid_list.size() == 0) return 0;
  if(date == "") date = current_date();
  for(int i=0; i < covid_list.size(); i++){
    if(String(covid_list[i].date) == date ){
      return covid_list[i].cases;
    }
  }
  return 0;
}

uint16_t TridentTD_COVID19::deaths(String date){
  if( covid_list.size() == 0) return 0;
  if(date == "") date = current_date();
  for(int i=0; i < covid_list.size(); i++){
    if(String(covid_list[i].date) == date ){
      return covid_list[i].deaths;
    }
  }
  return 0;
}

void TridentTD_COVID19::country(String Country) {
  this->getHistory(Country);
}

String  TridentTD_COVID19::country() {
  return _country;
}

const char* TridentTD_COVID19::current_date(){
  if( covid_list.size() == 0) return "";
  return (const char*)covid_list[covid_list.size()-1].date;
}


void TridentTD_COVID19::getToday(String country){
  if(!WiFi.isConnected()) return;
  // this->_country = country;

#if defined(ESP8266)
  // ยังไม่ work
  _http.begin( String("https://corona.lmao.ninja/countries/") + country , "‎7b 00 29 73 45 13 a3 f7 95 84 42 ea e0 28 1b f4 0d b8 11 53"); 
#elif defined (ESP32)
  _http.begin( String("https://corona.lmao.ninja/countries/") + country );
#endif
  
  int http_code = _http.GET();

  if( http_code == 200) {

    String payload = _http.getString();
    // Serial.println(payload);
    int idx;
    memset( &_today_data, 0, sizeof( _today_data));

    idx = payload.indexOf("\"cases\":");
    if(idx > -1) {  _today_data.cases = payload.substring( idx + 8).toInt(); }

    idx = payload.indexOf("\"todayCases\":");
    if(idx > -1) {  _today_data.todayCases = payload.substring( idx + 13).toInt(); }

    idx = payload.indexOf("\"deaths\":");
    if(idx > -1) {  _today_data.deaths = payload.substring( idx + 9).toInt(); }

    idx = payload.indexOf("\"todayDeaths\":");
    if(idx > -1) {  _today_data.todayDeaths = payload.substring( idx + 14).toInt(); }

    idx = payload.indexOf("\"recovered\":");
    if(idx > -1) {  _today_data.recovered = payload.substring( idx + 12).toInt(); }

    idx = payload.indexOf("\"active\":");
    if(idx > -1) {  _today_data.active = payload.substring( idx + 9).toInt(); }

    idx = payload.indexOf("\"critical\":");
    if(idx > -1) {  _today_data.critical = payload.substring( idx + 11).toInt(); }

    idx = payload.indexOf("\"casesPerOneMillion\":");
    if(idx > -1) {  _today_data.casesPerOneMillion = payload.substring( idx + 21).toInt(); }

    idx = payload.indexOf("\"deathsPerOneMillion\":");
    if(idx > -1) {  _today_data.deathsPerOneMillion = payload.substring( idx + 22).toInt(); }

  }else{
    Serial.println("_http can't connected");
    
  }
  _http.end();
  // return _today_data;
}

uint16_t  TridentTD_COVID19::todayCases() {
  return _today_data.todayCases;
}

uint16_t  TridentTD_COVID19::todayDeaths() {
  return _today_data.todayDeaths;
}

uint16_t  TridentTD_COVID19::recovered() {
  return _today_data.recovered;
}

uint16_t  TridentTD_COVID19::active() {
  return _today_data.active;
}

uint16_t  TridentTD_COVID19::critical() {
  return _today_data.critical;
}

uint16_t  TridentTD_COVID19::casesPerOneMillion() {
  return _today_data.casesPerOneMillion;
}

uint16_t  TridentTD_COVID19::deathsPerOneMillion() {
  return _today_data.deathsPerOneMillion;
}

