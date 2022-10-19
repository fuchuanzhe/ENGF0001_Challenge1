#include <SPI.h>
#include <TFT_eSPI.h>  

#include <WiFi.h>
#include "esp_wpa2.h"   // wpa2 for connection to enterprise networks
#include "time.h"

#define EAP_IDENTITY "xxxxxxx@ucl.ac.uk"                
#define EAP_PASSWORD "xxxxxxxx"

const char* essid = "eduroam";
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

TFT_eSPI tft = TFT_eSPI(); 

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  tft.setRotation(3);
  tft.invertDisplay( true ); 
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,10,7);
  tft.setTextColor(0xFBE0, TFT_BLACK);
  tft.println(&timeinfo, "%H:%M:%S");
  tft.setTextFont(4);
  tft.println("------------------------------");
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println(&timeinfo, "%A");
  tft.println(&timeinfo, "%d %B %Y");
}

void setup() {
  bool eduroamFound = false;
  Serial.begin(115200);
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  while (!eduroamFound) {
    Serial.println("scan start");
    int n = WiFi.scanNetworks(); // WiFi.scanNetworks returns the number of networks found
    Serial.println("scan done");
    
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        
        for (int i = 0; i < n; ++i) {
            String ssid = WiFi.SSID(i);
            int    rssi = WiFi.RSSI(i);
          
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(ssid);
            Serial.print(" (");
            Serial.print(rssi);
            Serial.print(")");
            Serial.print((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
            
            ssid.trim();
            if (ssid == essid) {
              Serial.print(" <==== eduroam found");
              eduroamFound = true;
            }
            Serial.println("");
        }
    }
    Serial.println("");

    // Wait a bit before scanning again
    if (!eduroamFound)
      delay(5000);
  }

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(essid);
  
  // Scan available WiFi networks until eduroam is seen
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_IDENTITY, strlen(EAP_IDENTITY));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
  esp_wifi_sta_wpa2_ent_enable();
  
  WiFi.begin(essid);       //connect to eduroam
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi is connected to ");
  Serial.println(essid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //print LAN IP

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0,0,4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);  
  tft.println("Initialising...");
  delay(5000);

}


void loop() {
  delay(1000);
  printLocalTime();
}

