#include <ESP8266WiFi.h>
#include <ezTime.h>
#include <MD_Parola.h>     // include MajicDesigns Parola library
#include <MD_MAX72xx.h>    // include MajicDesigns MAX72xx LED matrix library
#include <SPI.h>

// configuration
static const char ourtimezone[] PROGMEM = "Europe/Helsinki";  //official timezone names https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
#define wifi_ssid "ssid" //ssid of your WiFi
#define wifi_pwd "password" //password for your WiFi

#define DISP_BRGTH 5                         // brightness of the display (0-15)
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW    // this line defines our dot matrix hardware type MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4                        // define number of total cascaded modules
#define DATA_PIN 13   // define Data pin
#define CLK_PIN 14   // define Clock pin
#define CS_PIN 15   // define CS (Chip Select) pin for WEMOS D1 MINI

uint8_t scrollSpeed = 150;    // set initial scroll speed, can be a value between 10 (max) and 150 (min)
textEffect_t scrollEffect  = PA_PRINT;  // Scroll disabled
textPosition_t scrollAlign = PA_CENTER; // scroll align
uint16_t scrollPause = 2000;            // scroll pause in milliseconds

char _chbuf[16] = {"READY"};

// Software SPI with custom PINs
// MD_Parola display = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

//Hardware SPI 
MD_Parola display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

Timezone myTZ;

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n* * * ESP BOOT * * *"));
  setDebug(INFO);

  Serial.println(F("Display Init"));
  // initialize the dot matrix display
  display.begin();
  // set the intensity (brightness) of the display
  display.setIntensity(DISP_BRGTH);
  Serial.println(F("Display Ready"));
  
  Serial.println(F("Connecting to WiFi"));
  WiFi.begin( wifi_ssid, wifi_pwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("\nWiFi connected"));

  //NTP Server
  setServer("pool.ntp.org");
  waitForSync();

  if (myTZ.setLocation(FPSTR(ourtimezone))) {
    Serial.println(F("Timezone lookup OK"));
  } else {
    Serial.println(F("\nTimezone lookup failed, will use UTC!"));
    myTZ = UTC;
  }
  display.displayText(_chbuf, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);
  display.displayAnimate();
}

void loop() {
  events();
  if (minuteChanged()) {
    // Length (with one extra character for the null terminator)
    int str_len = myTZ.dateTime("H:i").length() + 1; 
    myTZ.dateTime("H:i").toCharArray(_chbuf,str_len);
    // print text on the display
    display.displayText(_chbuf, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

    Serial.printf("\nTime: %s",_chbuf);
  
    if (display.displayAnimate()) {    // animate the display
      display.displayReset();          // reset the current animation
    }
  }
}
