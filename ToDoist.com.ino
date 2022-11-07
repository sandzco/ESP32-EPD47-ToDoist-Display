#ifndef BOARD_HAS_PSRAM
#error "Please enable PSRAM !!!"
#endif


#include <Arduino.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd_driver.h"

#include "firasans.h"
#include "owm_credentials.h"

#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "esp_adc_cal.h"
#include <WiFi.h>               // In-built
#include <SPI.h>                // In-built
#include <time.h>               // In-built

#include <string.h>

#define SCREEN_WIDTH   EPD_WIDTH
#define SCREEN_HEIGHT  EPD_HEIGHT


int     wifi_signal, CurrentHour = 0, CurrentMin = 0, CurrentSec = 0, EventCnt = 0, vref = 1100;

long SleepDuration   = 15; // Sleep time in minutes, aligned to the nearest minute boundary, so if 30 will always update at 00 or 30 past the hour
int  WakeupHour      = 6;  // Don't wakeup until after 05:00 to save battery power
int  SleepHour       = 22; // Sleep after 22:00 to save battery power
long StartTime       = 0;
long SleepTimer      = 0;
long Delta           = 30; // ESP32 rtc speed compensation, prevents display at xx:59:yy and then xx:00:yy (one minute later) to save power

String  Time_str = "--:--:--";
String  Date_str = "-- --- ----";

DynamicJsonDocument doc(16384);

#define BATT_PIN            36
#define SD_MISO             12
#define SD_MOSI             13
#define SD_SCLK             14
#define SD_CS               15

uint8_t *framebuffer;

void BeginSleep() {
  epd_poweroff_all();
  UpdateLocalTime();
  SleepTimer = (SleepDuration * 60 ); //Some ESP32 have a RTC that is too fast to maintain accurate time, so add an offset
  esp_sleep_enable_timer_wakeup(SleepTimer * 1000000LL); // in Secs, 1000000LL converts to Secs as unit = 1uSec
  Serial.println("Awake for : " + String((millis() - StartTime) / 1000.0, 3) + "-secs");
  Serial.println("Entering " + String(SleepTimer) + " (secs) of sleep time");
  Serial.println("Starting deep-sleep period...");
  esp_deep_sleep_start();  // Sleep for e.g. 30 minutes
}

boolean SetupTime() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer, "time.nist.gov"); //(gmtOffset_sec, daylightOffset_sec, ntpServer)
  setenv("TZ", Timezone, 1);  //setenv()adds the "TZ" variable to the environment with a value TimeZone, only used if set to 1, 0 means no change
  tzset(); // Set the TZ environment variable
  delay(100);
  return UpdateLocalTime();
}

uint8_t StartWiFi() {
  Serial.println("\r\nConnecting to: " + String(ssid));
  IPAddress dns(8, 8, 8, 8); // Use Google DNS
  WiFi.disconnect();
  WiFi.mode(WIFI_STA); // switch off AP
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("STA: Failed!\n");
    WiFi.disconnect(false);
    delay(500);
    WiFi.begin(ssid, password);
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifi_signal = WiFi.RSSI(); // Get Wifi Signal strength now, because the WiFi will be turned off to save power!
    Serial.println("WiFi connected at: " + WiFi.localIP().toString());
  }
  else Serial.println("WiFi connection *** FAILED ***");
  return WiFi.status();
}

void StopWiFi() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi switched Off");
}

void InitialiseSystem() {
  StartTime = millis();
  Serial.begin(115200);
  while (!Serial);
  Serial.println(String(__FILE__) + "\nStarting...");
  epd_init();
  framebuffer = (uint8_t *)ps_calloc(sizeof(uint8_t), EPD_WIDTH * EPD_HEIGHT / 2);
  if (!framebuffer) Serial.println("Memory alloc failed!");
  memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
}

void setup() {
  
  InitialiseSystem();
  if (StartWiFi() == WL_CONNECTED && SetupTime() == true) { // 
    bool WakeUp = false;
    
    if (WakeupHour > SleepHour) WakeUp = (CurrentHour >= WakeupHour || CurrentHour <= SleepHour);
    else  WakeUp = (CurrentHour >= WakeupHour && CurrentHour <= SleepHour);

    if (StartWiFi() == WL_CONNECTED && WakeUp) {
        epd_poweron();      // Switch on EPD display
        epd_clear();        // Clear the screen
        drawBorders();      //draw borders
        drawBattery(860, 520); // draw battery
        getTasks();         //connect to ToDoist API
        StopWiFi();         // Reduces power consumption
        edp_update();       // Update the display to show the information
        epd_poweroff_all(); // Switch off all power to EPD
    }
    
  }

  Serial.println("Going to Sleep!!");
  BeginSleep();

}

void loop() {
  // Do Nothing
}

void drawBorders() {
  int lineWidth = 42;
  //draw borders
  //epd_fill_rect(0,0,960,50,220,framebuffer);
  epd_draw_hline(0,  50, 960, 0, framebuffer);
  epd_draw_vline(480, 0, 500, 128, framebuffer);
  WriteLine(10, lineWidth, section + " - 🚀" );
  WriteLine(490, lineWidth,  "OTHER - 🙊" );
  epd_draw_hline(0,  480, 960, 0, framebuffer);
  WriteLine(10, 525, "ToDoist.com - " +  Date_str + " " + Time_str );

}

String getHttp(String url, String headers) {
  
  byte Attempts = 1;
  int httpCode = 0;
  
  HTTPClient http;
  String payload = "";
  Serial.print("[HTTP] begin...\n");
  http.begin(url); //HTTP

  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  if (headers != "") http.addHeader("Authorization", headers);

  while (httpCode != 200 &&  Attempts <= 2){
        httpCode = http.GET();
        Attempts++;
  }
  
  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      //Serial.println(payload);

    } // if (httpCode == HTTP_CODE_OK)
    else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
  }
  http.end();
  return payload;
}



void json2Obj (String payload){
  
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, payload);
  // Test if parsing succeeds.
  if (error) {
    Serial.println(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  Serial.println("Success deserializeJson");
  
  }

void getProjectId(){

    Serial.println("Reading Projects");
    String payload = getHttp(projectsUrl, apikey);
    json2Obj (payload);   

    for (JsonObject item : doc.as<JsonArray>()) {
        Serial.println(item["name"].as<String>());
      if (item["name"].as<String>() == project) {
        Serial.println("Found Project: id=" + item["id"].as<String>());
        project_id = item["id"].as<uint32_t>();
        break;
        }
    }

}

void getSectionId(){
    Serial.println("Reading Sections");
    Serial.println(sectionsUrl + String(project_id));
    String payload = getHttp(sectionsUrl + String(project_id), apikey);
    json2Obj (payload);   

    for (JsonObject item : doc.as<JsonArray>()) {
      Serial.println(item["name"].as<String>());
      if (item["name"].as<String>() == section) {
        Serial.println("Found Section: id=" + item["id"].as<String>());
        section_id = item["id"].as<uint32_t>();
        break;
        }
    }
}

void getTasks() {
    Serial.println("Got Project ID:" + String(project_id)); 
    if (project_id == 1) {
      getProjectId();
      Serial.println("Got Project ID:" + String(project_id)); 
    }
    Serial.println("Got Section ID:" + String(section_id));
    if (section_id == 1) {
    getSectionId();
    Serial.println("Got Section ID:" + String(section_id));
    }
    
  Serial.println("Getting Tasks");
  String payload = getHttp(tasksUrl, apikey);
  json2Obj (payload);

  Serial.println("Got Tasks Json");

  int lineWidth = 42;
  int cursor_x = 10;
  int work_y = 50 + lineWidth;
  int other_y = 50 + lineWidth;
  int workCnt =0, otherCnt=0;

  for (JsonObject item : doc.as<JsonArray>()) {

    int priority = item["priority"].as<int>();

    String bullet = "➊ ";

    if (priority == 4) bullet = "➍ ";
    else if (priority == 3) bullet = "➌ ";
    else if (priority == 2) bullet = "➋ ";

    Serial.println(bullet + item["content"].as<String>());

    if (item["section_id"].as<uint32_t>() != section_id ) { //Right side
      cursor_x = 490;
      WriteLine(cursor_x, other_y, bullet + item["content"].as<String>() );
      other_y += lineWidth;
      otherCnt +=1;
    }
    else { // Left side
      cursor_x = 10;
      WriteLine(cursor_x, work_y, bullet + (item["content"].as<String>()).substring(0,20) );
      work_y += lineWidth;
      workCnt +=1;
    }

    if (workCnt > 9 || otherCnt > 9) break;

  } // Json For loop

}

void drawBattery(int x, int y) {
  uint8_t percentage = 100;
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
    vref = adc_chars.vref;
  }
  float voltage = analogRead(36) / 4096.0 * 6.566 * (vref / 1000.0);
  if (voltage > 1 ) { // Only display if there is a valid reading
    Serial.println("\nVoltage = " + String(voltage));
    percentage = 2836.9625 * pow(voltage, 4) - 43987.4889 * pow(voltage, 3) + 255233.8134 * pow(voltage, 2) - 656689.7123 * voltage + 632041.7303;
    if (voltage >= 4.20) percentage = 100;
    if (voltage <= 3.20) percentage = 0;  // orig 3.5
    epd_draw_rect(x + 25, y - 14, 40, 15, 0, framebuffer);
    epd_fill_rect(x + 65, y - 10, 4, 7, 0, framebuffer);
    epd_fill_rect(x + 27, y - 12, 36 * percentage / 100.0, 11, 0, framebuffer);
    //WriteLine(x + 85, y - 14, String(percentage) + "%  " + String(voltage, 1) + "v");
  }
}

boolean UpdateLocalTime() {
  struct tm timeinfo;
  char   time_output[30], day_output[30], update_time[30];
  while (!getLocalTime(&timeinfo, 5000)) { // Wait for 5-sec for time to synchronise
    Serial.println("Failed to obtain time");
    return false;
  }
  CurrentHour = timeinfo.tm_hour;
  CurrentMin  = timeinfo.tm_min;
  CurrentSec  = timeinfo.tm_sec;
  //See http://www.cplusplus.com/reference/ctime/strftime/
  Serial.println(&timeinfo, "%a %b %d %Y   %H:%M:%S");      // Displays: Saturday, June 24 2017 14:05:49
  if (Units == "M") {
    sprintf(day_output, "%s, %02u %s %04u", weekday_D[timeinfo.tm_wday], timeinfo.tm_mday, month_M[timeinfo.tm_mon], (timeinfo.tm_year) + 1900);
    strftime(update_time, sizeof(update_time), "%H:%M:%S", &timeinfo);  // Creates: '@ 14:05:49'   and change from 30 to 8 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    sprintf(time_output, "%s", update_time);
  }
  else
  {
    strftime(day_output, sizeof(day_output), "%a %b-%d-%Y", &timeinfo); // Creates  'Sat May-31-2019'
    strftime(update_time, sizeof(update_time), "%r", &timeinfo);        // Creates: '@ 02:05:49pm'
    sprintf(time_output, "%s", update_time);
  }
  Date_str = day_output;
  Time_str = time_output;
  return true;
}

void WriteLine(int x, int y, String text) {
  char * data  = const_cast<char*>(text.c_str());
  write_string((GFXfont *)&FiraSans, data, &x, &y, framebuffer);
}

void edp_update() {
  epd_draw_grayscale_image(epd_full_screen(), framebuffer); // Update the screen
}
