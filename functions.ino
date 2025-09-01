#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "time.h"
#include <local_info.h> //Local file that holds Internet information, plus Location and Country for API query
#include <Fonts/TomThumb.h>


/*
* println() for both online and real life Serial Monitor
*/
void Serial_n_Webln(auto text) {
  Serial.println(text);
  WebSerial.println(text);
}

/*
* print() for both online and real life Serial Monitor
*/
void Serial_n_Web(auto text) {
  Serial.print(text);
  WebSerial.print(text);
}

/*
* Displays the general Style of the Prayer schedule
*/
void static_background() {
  dma_display->setFont(&TomThumb);
  dma_display->setTextWrap(false);
  dma_display->setTextSize(1);
  dma_display->drawRect(0,0,dma_display->width(), dma_display->height(), myORANGE);
  dma_display->drawLine(0,13,64,13,myORANGE);
  dma_display->setCursor(3,21);
  dma_display->setTextColor(myWHITE);
  dma_display->print("FAJR");
  dma_display->drawLine(0,23,64,23,myORANGE);
  dma_display->setCursor(3,31);
  dma_display->print("DUHR");
  dma_display->drawLine(0,33,64,33,myORANGE);
  dma_display->setCursor(3,41);
  dma_display->print("ASR");
  dma_display->drawLine(0,43,64,43,myORANGE);
  dma_display->setCursor(3,51);
  dma_display->print("MAGH.");
  dma_display->drawLine(0,53,64,53,myORANGE);
  dma_display->setCursor(3,61);
  dma_display->print("ISHA");
}

/*
* Gets time from NTP server
* Displays Clock on the rgb matrix
*/
void get_n_display_clk() {
  configTime(gmtOffset_sec,daylightOffset_sec,ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial_n_Webln("Failed to obtain time");
  }
  else {
    int hour = timeinfo.tm_hour;
    int min = timeinfo.tm_min;
    int sec = timeinfo.tm_sec;
    String am_pm = "AM";
    if (hour == 0) {
      am_pm = "AM";
      hour = 12;
    }
    else if (hour == 12) {
      am_pm = "PM";
    }
    else if (hour > 12 ) {
      hour -= 12;
      am_pm = "PM";
    }
    if (!get_prayer_times) {
      String hour_str = String(hour);
      String min_str = String(min);
      String sec_str = String(sec);
      if (min < 10) {
        min_str = "0" + String(min);
      }
      if (sec < 10) {
        sec_str = "0" + String(sec);
      }
      dma_display->setFont(&TomThumb);
      dma_display->setCursor(2,12);
      dma_display->setTextSize(2);
      dma_display->fillRect(1,1,62,12,myBLACK);
      dma_display->print(hour_str + ":" + min_str + ":" + sec_str);
      dma_display->setTextSize(1);
      dma_display->setCursor(55,7);
      dma_display->print(am_pm);         
    }
  }
}

/*
* Gets prayer schedule from IslamicFinder API
* Displays schedule on the RGB matrix
*/
void get_n_display_times(HTTPClient &client, bool &get_prayer_times, int httpCode) {
  const char* fajr = nullptr;
  const char* sunrise = nullptr;
  const char* dhuhr = nullptr;
  const char* asr = nullptr;
  const char* maghrib = nullptr;
  const char* isha = nullptr;

  get_prayer_times = true;

  String payload = client.getString();
  payload.replace("%am%", "AM");
  payload.replace("%pm%", "PM");
  Serial_n_Webln("\nStatuscode: " + String(httpCode));
  Serial_n_Webln(payload);

  DynamicJsonDocument doc(1024);
  deserializeJson(doc,payload);

  fajr = doc["results"]["Fajr"];
  sunrise = doc["results"]["Duha"];
  dhuhr = doc["results"]["Dhuhr"];
  asr = doc["results"]["Asr"];
  maghrib = doc["results"]["Maghrib"];
  isha = doc["results"]["Isha"];

  Serial_n_Webln(fajr);
  Serial_n_Webln(sunrise);
  Serial_n_Webln(dhuhr);
  Serial_n_Webln(asr);
  Serial_n_Webln(maghrib);
  Serial_n_Webln(isha);
  
  client.end();

  dma_display->clearScreen();
  static_background();

  dma_display->setTextWrap(false);
  dma_display->setCursor(38,21);
  dma_display->setTextColor(myWHITE);
  dma_display->print(String(fajr));

  dma_display->setCursor(38,31);
  dma_display->setTextColor(myWHITE);
  dma_display->print(String(dhuhr));

  dma_display->setCursor(38,41);
  dma_display->setTextColor(myWHITE);
  dma_display->print(String(asr));

  dma_display->setCursor(38,51);
  dma_display->setTextColor(myWHITE);
  dma_display->print(String(maghrib));

  dma_display->setCursor(38,61);
  dma_display->setTextColor(myWHITE);
  dma_display->print(String(isha))  ;

  get_prayer_times = false; 
}