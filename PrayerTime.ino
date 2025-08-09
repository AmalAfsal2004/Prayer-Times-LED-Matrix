//LIBRARIES FOR OVER THE AIR UPDATES
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <WebSerial.h>
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

const char* country = COUNTRY; 
const char* zipcode = ZIPCODE;
String api_link = "https://www.islamicfinder.us/index.php/api/prayer_times?country=" + String(country) + "&zipcode=" + String(zipcode);

//The Clock being displayed synchronizes using this NTP SERVER
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -28800; //GMT OFFSET
const int daylightOffset_sec = 3600; //Daylight Savings offset

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
unsigned long prev_clock_millis = 0;
unsigned long prev_display_millis = 0;

AsyncWebServer server(80);

void setup() {
  
  Serial.begin(115200); 
  delay(3000);

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

  static_background(); //Creates the static background

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

  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient client;
    client.useHTTP10(true);
    client.begin(api_link);
    int httpCode = client.GET();

    if (httpCode > 0) {
      get_n_display_times(client, get_prayer_times, httpCode);
    }
    else {
      Serial.println("Error on initial fetch prayer times HTTP Request");
    }
  }
  else {
    Serial.println("Connection Lost on initial fetch prayer times request");
  }

/*
  WebSerial.begin(&server); //ONLINE SERIAL MONITOR
  WebSerial.onMessage([&](uint8_t *data, size_t len) {
    Serial.printf("Received %u bytes from WebSerial: ", len);
    Serial.write(data, len);
    Serial.println();
    WebSerial.println("Received Data...");
    String d = "";
    for(size_t i=0; i < len; i++){
      d += char(data[i]);
    }
    WebSerial.println(d);
  });
  */
  ElegantOTA.begin(&server);   
  ElegantOTA.setAutoReboot(true);
  //ESP32 OVER THE AIR UPDATE FUNCTIONALITY, REFER TO ELEGANT OTA DOCUMENTATION
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  server.begin();
  Serial.println("HTTP server started");

}


void loop() {
  ElegantOTA.loop(); //OVER THE AIR UPDATE FUNCTIONALITY 

  const int interval_clock = 1000;
  const int interval_display = 7200000;

  if ((millis() - prev_clock_millis) >= interval_clock) {
    prev_clock_millis = millis();
    get_n_display_clk();
  }

  if ((millis() - prev_display_millis) >= interval_display) {
    prev_display_millis = millis();

    if ((WiFi.status() == WL_CONNECTED)) {
      HTTPClient client;
      client.useHTTP10(true);
      client.begin(api_link);
      int httpCode = client.GET();

      if (httpCode > 0) {
        get_n_display_times(client, get_prayer_times, httpCode);
      }
      else {
        Serial.println("Error on HTTP Request");
      }
    }
    else {
      Serial.println("Connection Lost");
    }

  }
}
