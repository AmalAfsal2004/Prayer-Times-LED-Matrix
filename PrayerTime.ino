//LIBRARIES FOR OVER THE AIR UPDATES
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
//ESSENTIAL LIBRARIES FOR OVERALL LIBRARIES
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "time.h"
#include <local_info.h> //Local file that holds Internet information, plus Location and Country for API query
#include <Fonts/TomThumb.h>

const char* ssid = SSID; //WIFI USERNAME, ex: ssid = "House"
const char* password = PASSWORD; //WIFI PASSWORD , ex: password = "House123"

//The Clock being displayed synchronizes using this NTP SERVER
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -28800; //GMT OFFSET
const int daylightOffset_sec = 3600; //Daylight Savings offset

TaskHandle_t Task1; //Utilizing ESP32 Dual Core processing

//----------------------------------------Defines the connected PIN between P3 and ESP32.
#define R1_PIN 4
#define G1_PIN 2
#define B1_PIN 32
#define R2_PIN 33
#define G2_PIN 25
#define B2_PIN 26

#define A_PIN 14
#define B_PIN 12
#define C_PIN 23
#define D_PIN 22
#define E_PIN 27 //--> required for 1/32 scan panels, like 64x64px. Any available pin would do, i.e. IO32.

#define LAT_PIN 19
#define OE_PIN 18
#define CLK_PIN 21
//----------------------------------------

//----------------------------------------Defines the P5 Panel configuration.
#define PANEL_RES_X 64  //--> Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64  //--> Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 1   //--> Total number of panels chained one to another
//----------------------------------------

MatrixPanel_I2S_DMA *dma_display = nullptr;

//----------------------------------------Variable for color.
// color565(0, 0, 0); --> RGB color code. Use the "color picker" to use or find another color code.
uint16_t myWHITE = dma_display->color565(255, 255, 255);
uint16_t myBLACK = dma_display->color565(0, 0, 0);
uint16_t myORANGE = dma_display->color565(255, 154, 0);

//----------------------------------------

/*
* There could be an instance where the digital clock 
*/
bool get_prayer_times = false;

AsyncWebServer server(80);

void setup() {
  
  Serial.begin(115200); 
  delay(3000);

  xTaskCreatePinnedToCore(
      Task1code,
      "Task1",
      2048,
      NULL,
      1,
      &Task1,
      0);
  delay(1000);

  // Initialize the connected PIN between Panel P3 and ESP32.
  HUB75_I2S_CFG::i2s_pins _pins={R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN};
  delay(10);

  //----------------------------------------Module configuration.
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   //--> module width.
    PANEL_RES_Y,   //--> module height.
    PANEL_CHAIN,   //--> Chain length.
    _pins          //--> pin mapping.
  );
  delay(10);
  //----------------------------------------

  // Set I2S clock speed.
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_10M;  // I2S clock speed, better leave as-is unless you want to experiment.
  delay(10);

  //----------------------------------------Display Setup.
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(3); //--> 0-255.
  //----------------------------------------
  
  dma_display->clearScreen();
  delay(1000);

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

  //WIFI SETUP
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  //ESP32 OVER THE AIR UPDATE FUNCTIONALITY, REFER TO ELEGANT OTA DOCUMENTATION
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  server.begin();
  Serial.println("HTTP server started");

  ElegantOTA.begin(&server);    // Start ElegantOTA
}

void Task1code(void * pvParameters) {
  delay(15000);
  Serial.println("Task1 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;) {
    configTime(gmtOffset_sec,daylightOffset_sec,ntpServer);
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
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
        if (hour < 10) {
          hour_str = "0" + String(hour);
        }
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
        Serial.println(hour_str + ":" + min_str + ":" + sec_str);
        dma_display->setTextSize(1);
        dma_display->setCursor(55,7);
        dma_display->print(am_pm);         
      }
      UBaseType_t stackLeft = uxTaskGetStackHighWaterMark(NULL);
      Serial.print("\n");
      Serial.print("Stack space left: ");
      Serial.println(stackLeft);
      vTaskDelay(1000);
    }
  }
}
void loop() {
    ElegantOTA.loop(); //OVER THE AIR UPDATE FUNCTIONALITY 

    const char* fajr = nullptr;
    const char* sunrise = nullptr;
    const char* dhuhr = nullptr;
    const char* asr = nullptr;
    const char* maghrib = nullptr;
    const char* isha = nullptr;

    //Refer to local_info.h
    const char* country = COUNTRY; 
    const char* zipcode = ZIPCODE;
    String api_link = "https://www.islamicfinder.us/index.php/api/prayer_times?country=" + String(country) + "&zipcode=" + String(zipcode);


  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient client;
    client.useHTTP10(true);
    client.begin(api_link);
    int httpCode = client.GET();

    if (httpCode > 0) {
      get_prayer_times = true; 
      String payload = client.getString();
      payload.replace("%am%", "AM");
      payload.replace("%pm%", "PM");
      Serial.println("\nStatuscode: " + String(httpCode));
      Serial.println(payload);

      DynamicJsonDocument doc(1024);
      deserializeJson(doc,payload);

      fajr = doc["results"]["Fajr"];
      sunrise = doc["results"]["Duha"];
      dhuhr = doc["results"]["Dhuhr"];
      asr = doc["results"]["Asr"];
      maghrib = doc["results"]["Maghrib"];
      isha = doc["results"]["Isha"];

      Serial.println(fajr);
      Serial.println(sunrise);
      Serial.println(dhuhr);
      Serial.println(asr);
      Serial.println(maghrib);
      Serial.println(isha);

      client.end();

      dma_display->fillRect(38,17,23,5,myBLACK);
      dma_display->fillRect(38,27,23,5,myBLACK);
      dma_display->fillRect(38,37,23,5,myBLACK);
      dma_display->fillRect(38,47,23,5,myBLACK);
      dma_display->fillRect(38,57,23,5,myBLACK);

      dma_display->setTextWrap(false);
      dma_display->setCursor(38,21);
      dma_display->setTextColor(myWHITE);
      dma_display->print(fajr);

      dma_display->setCursor(38,31);
      dma_display->setTextColor(myWHITE);
      dma_display->print(dhuhr);

      dma_display->setCursor(38,41);
      dma_display->setTextColor(myWHITE);
      dma_display->print(asr);

      dma_display->setCursor(38,51);
      dma_display->setTextColor(myWHITE);
      dma_display->print(maghrib);

      dma_display->setCursor(38,61);
      dma_display->setTextColor(myWHITE);
      dma_display->print(isha);

      get_prayer_times = false; 
    }
    else {
      Serial.println("Error on HTTP Request");
    }
  }
  else {
    Serial.println("Connection Lost");
  }
  delay(7200000); //Prayer times will update every two hours
}
