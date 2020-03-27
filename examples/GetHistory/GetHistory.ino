#include <TridentTD_COVID19.h>

#define SSID      "-----------"
#define PASSWORD  "-----------"

void setup() {
  Serial.begin(115200); Serial.println();

  COVID19.begin("thailand");        // จะมีการ reload อัตโนมัติทุกๆ 15 นาที
  
  WiFi.begin(SSID, PASSWORD);
  while(!WiFi.isConnected()) delay(400);
  Serial.println(WiFi.localIP());
}

void loop() {
  // แสดง history ทั้งหมด
  if( COVID19.available()){ // เมื่อมีการ reload ใหม่
    Serial.print("cases               : "); Serial.println( COVID19.cases() );      // cases ติดเชื้อทั้งหมด ข้อมูลล่าสุด
    Serial.print("deaths              : "); Serial.println( COVID19.deaths() );     // deaths ยอดเสียชีวิตหมด ข้อมูลล่าสุด
    Serial.print("today cases         : "); Serial.println( COVID19.todayCases() ); // ยอดผู้ติดเชื่อเพิ่มใหม่ รายวัน
    Serial.print("today deaths        : "); Serial.println( COVID19.todayDeaths() );// ยอดผู้เสียชีวิตเพิ่มใหม่ รายวัน
    Serial.print("recovered           : "); Serial.println( COVID19.recovered() );  // ยอดผู้รักษาหาย
    Serial.print("active              : "); Serial.println( COVID19.active() );     // ยอดผู้ป่วยระหว่างการรักษา
    Serial.print("critical            : "); Serial.println( COVID19.critical() );   // ยอดผู้ป่วยอาการหนัก
    Serial.print("casesPerOneMillion  : "); Serial.println( COVID19.casesPerOneMillion() );
    Serial.print("deathsPerOneMillion : "); Serial.println( COVID19.deathsPerOneMillion() );

    // สำหรับ casess(...)  และ deathes(...) สามารถกำหนดเลือกวันที่ต้องการได้
    Serial.print("cases  15/03/2020   : "); Serial.println( COVID19.cases("15/03/2020") );  // cases ติดเชื้อ ณ วันที่ ที่ต้องการ
    Serial.print("deaths 15/03/2020   : "); Serial.println( COVID19.deaths("15/03/2020") ); // deaths ยอดเสียชีวิต ณ วันที่ ที่ต้องการ

    
    Serial.println("--------------------------------------");
    Serial.println("History ...");
    for(int i =0; i < COVID19.size(); i++){
      Serial.printf("%s : cases : %d  ;  deaths : %d\n", COVID19[i].date, COVID19[i].cases, COVID19[i].deaths  );
    }
  }
}
