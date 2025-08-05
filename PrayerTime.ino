
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ssid";
const char* password = "pass";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); 
  delay(4000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:

}
