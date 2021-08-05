/*
   Open Weather One Call Library
   v3.0.4
   Copyright 2020 - Jessica Hershey
   www.github.com/JHershey69

   One Call API key at www.openweathermap.org
   Google Developer Key no longer required.

   Simple_Latitude_Longitude_Weather_Example.ino

   Returns ALL information and uses Latitude/Longitude, CITY ID, or IP Address
   If using a hotspot to connect your ESP32 to the WWW your results for IP
   search maybe be radically out of range. Please consult the documentation
   for use and variables available in the returned weather message

*/

// ===== Required libraries (Other required libraries are installed in header file)
#include <OpenWeatherOneCall.h>
#include <WiFi.h> //<-- Might be native to ESP32
#include "arduino_secrets.h"
// ===============================================================================


// ========= Required only for TFT Display ==============
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
//========= End TFT Library =============================


char HOMESSID[] = SECRET_SSID;
char HOMEPW[] =  SECRET_PASS;
char ONECALLKEY[] = SECRET_APIKEY;

// Only needed if WiFiTri installed for Triangulation may have fee from Google
//#define GOOGLEKEY "YOUR GOOGLE DEVELOPER API KEY""


//  OpenWeatherOneCall variables

//For Latitude and Longitude Location setting if used
float myLATITUDE = SECRET_LAT;    //<-----Some location on theGulf Coast of Florida
float myLONGITUDE = SECRET_LON;

//For City ID Location setting if used
//int myCITYID = 4504476;          //<-----Toms River, NJ USA

int myUNITS = IMPERIAL;          //<-----METRIC, IMPERIAL, KELVIN (IMPERIAL is default)

//Can't get current and historical at the same time
int myHISTORY = NULL;            //<-----Only required for historical data up to 5 days

//See manual for excludes, only CURRENT Data allows 1,000,000 calls a month
//int myEXCLUDES = EXCL_D+EXCL_H+EXCL_M+EXCL_A;              //<-----0 Excludes is default
int myEXCLUDES = EXCL_H+EXCL_M;
//int myEXCLUDES = 0;

//for debugging loop counting
int nextCall = 0;
//int callAttempt = 1;

OpenWeatherOneCall OWOC;              // <------ Invoke Library like this

// WiFi Connect function **********************************
void connectWifi() {
  WiFi.begin(HOMESSID, HOMEPW);
  Serial.printf("Connecting.");
  int TryNum = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(200);
    TryNum++;
    if (TryNum > 20) {
      Serial.printf("\nUnable to connect to WiFi. Please check your parameters.\n");
      for (;;);
    }
  }
  Serial.println("Connected to WiFi");
} //================== END WIFI CONNECT =======================



void setup() {
  // In this sketch for display of values
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  Serial.println("Serial Monitor Initialized");

  // WiFi Connection required *********************
  while (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  } //<----------------End WiFi Connection Check


  /*
  Getting the Weather
  If you desire WiFi Triangulation to find your
  Latitude and Longitude, please install the
  WiFiTriangulation Library
  Otherwise Latitude and Longitude can be from any
  source such as a GPS or entered manually.
  If you DO NOT set the Latitude and Longitude
  the program will attempt to find your location
  by IP Address (which could be grossly innacurate
  if you are using a HOTSPOT on your phone)
  */

  // First set your Key value
  OWOC.setOpenWeatherKey(ONECALLKEY);

  // Second set your position (can be CITY ID, IP ADDRESS, GPS, or Manual LATITUDE/LONGITUDE)
  // Choose one of the following options

  // Here we use the Lat and Lon for Pensacola, Florida (Using the predefined values)
  OWOC.setLatLon(myLATITUDE, myLONGITUDE);
  
  // If we are using CITY ID
  //OWOC.setLatLon(myCITYID);
  
  // If we want to use IP Address Geolocation use no arguments
  //OWOC.setLatLon();

  // Third set any EXCLUDES if required (Here we are not using any
  OWOC.setExcl(myEXCLUDES);

  // Set History if you want historical weather other wise NULL
  OWOC.setHistory(NULL);
   
    // Set UNITS of MEASURE otherwise default is IMPERIAL
  OWOC.setUnits(myUNITS);

    tft.init();
    tft.setRotation(3);
    tft.setTextColor(TFT_GREEN);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.fillScreen(TFT_BLACK);
  
}

void loop() {
    tft.setCursor(0, 0);  
    tft.fillScreen(TFT_BLACK);
      
   //Now call the weather. Please notice no arguments are required in this call
  OWOC.parseWeather();

  //Now display some information, note the pointer requirement for current and alert, this is NEW for v3.0.0

  // Location info is available for ALL modes (History/Current)
  Serial.printf("\nLocation: % s, % s % s\n", OWOC.location.CITY, OWOC.location.STATE, OWOC.location.COUNTRY);

  //Verify all other values exist before using
  if (myHISTORY) //Remember you can't get historical and current weather at the same time
  {
    if (OWOC.history)
    {
      Serial.printf("Mean Temp for % s : % .0f\n", OWOC.history[0].weekDayName, OWOC.history[0].temperature);
    }
  } else
  {      
    if (OWOC.alert) {
      Serial.printf("ALERT *** ALERT *** ALERT\n");
      Serial.printf("Sender : % s\n", OWOC.alert->senderName);
      Serial.printf("Event : % s\n", OWOC.alert->event);
      Serial.printf("ALERT : % s\n", OWOC.alert->summary);      
      tft.setTextColor(TFT_RED);
      tft.printf("ALERT:%s\n", OWOC.alert->event);    
    }
    
    if (OWOC.current)
    {
      Serial.printf("Last Update: % s\n", OWOC.current->readableDateTime);
      Serial.printf("Current Temp : % .0f\n", OWOC.current->temperature);      
      Serial.printf("Current Humidity : % .0f\n", OWOC.current->humidity);
      Serial.printf("Current Pressure : % .0f\n", OWOC.current->pressure);             
      Serial.printf("Current Wind : % .0f\n", OWOC.current->windSpeed);      
      Serial.printf("Current Condition : % s\n", OWOC.current->summary);
      tft.setTextColor(TFT_WHITE);            
      tft.printf("T:%.0f", OWOC.current->temperature);
      tft.printf(" H:%.0f", OWOC.current->humidity);
      tft.printf(" P:%.0f\n", OWOC.current->pressure);
      tft.printf("W:%.0f", OWOC.current->windSpeed);      
      tft.printf(" %s\n", OWOC.current->summary);                              
    }

    if (OWOC.forecast)
    {
      Serial.printf("Forecast Temp Today : % .0f\n", OWOC.forecast[0].temperatureHigh);
      Serial.printf("Forecast Wind Today : % .0f\n", OWOC.forecast[0].windSpeed);      
      Serial.printf("Forecast Condition Today : % s\n", OWOC.forecast[0].summary);                
      Serial.printf("Forecast Temp Tomorrow : % .0f\n", OWOC.forecast[1].temperatureHigh);
      Serial.printf("Forecast Wind Tomorrow : % .0f\n", OWOC.forecast[1].windSpeed);      
      Serial.printf("Forecast Condition Tomorrow : % s\n", OWOC.forecast[1].summary);
      tft.setTextColor(TFT_GREEN);      
      tft.println("Today");
      tft.printf("Low:%.0f", OWOC.forecast[0].temperatureLow);      
      tft.printf(" Hi:%.0f\n", OWOC.forecast[0].temperatureHigh);
      tft.printf("W:%.0f", OWOC.forecast[0].windSpeed);             
      tft.printf(" %s\n", OWOC.forecast[0].summary);     
      tft.setTextColor(TFT_YELLOW);      
      tft.println("Tomorrow");
      tft.printf("Low:%.0f", OWOC.forecast[1].temperatureLow);      
      tft.printf(" Hi:%.0f\n", OWOC.forecast[1].temperatureHigh);
      tft.printf("W:%.0f", OWOC.forecast[1].windSpeed);             
      tft.printf(" %s\n", OWOC.forecast[1].summary);                   
    }
}


//void loop() {
delay(600000);
}
