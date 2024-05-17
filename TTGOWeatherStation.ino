//
//#include "ani.h"
#include <FS.h> // Include the filesystem library
#include <SPIFFS.h> 
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <ArduinoJson.h>          //https://github.com/bblanchon/ArduinoJson.git
#include <NTPClient.h>           //https://github.com/taranais/NTPClient
#include "timezone.h"
#include <time.h>
#include <UrlEncode.h>


TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
TFT_eSprite ScrollTextSprite = TFT_eSprite(&tft); // Create a Sprite object called ScrollTextSprite
TFT_eSprite ForecastSprite = TFT_eSprite(&tft); // Create a Sprite object called ForecastSprite

#define TFT_GREY 0x5AEB
#define lightblue 0x01E9
#define darkred 0xA041
#define blue 0x5D9B
#define MAXFORECASTS 100

#define FORECASTMODE_TEMP 0
#define FORECASTMODE_HUM 1
#define FORECASTMODE_RAIN 2
#define FORECASTMODES 3

#include "Orbitron_Medium_20.h"
#include <WiFi.h>

#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <FS.h> // Include the filesystem library
#include "openmeteo.h"

String OpenMeteoUrl = "";

OpenMeteoLocation Location;

#define NUMLOCATIONS 10
String locations[NUMLOCATIONS] = {"Tokyo", "Paris", "Sydney", "Rio de Janeiro", "Beijing", "Cairo", "Rome", "Berlin", "Bangkok", "Toronto"};
const char* ssid = "ESSID";         //EDDIT
const char* password = "PASSWORD";  //EDDIT
String payload=""; //whole json 
String tmp="" ; //temperature
String hum="" ; //humidity
String tt="";

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;
int current_location = 0;

StaticJsonDocument<20000> doc;
char inp[20000];

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
String currenTS;

String forecast_time[MAXFORECASTS];
int forecast_temp[MAXFORECASTS];
int forecast_humidity[MAXFORECASTS];
int forecast_rain[MAXFORECASTS];
int forecast_count=0;
int forecast_mode=FORECASTMODE_TEMP;


int backlight[5] = {10,30,60,120,220};
byte b=1;

int ScrollStep = -1;
int ScrollStepCounter = 0;

String town="";
String Country="";

void setup(void) {
   pinMode(0,INPUT_PULLUP);
   pinMode(35,INPUT);
  tft.init();
  tft.setSwapBytes(true);
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);

  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  ledcWrite(pwmLedChannelTFT, backlight[b]);
  setupopenmeteo();
  Serial.begin(115200);
  tft.print("Connecting to ");
  tft.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    tft.print(".");
  }
  
  tft.println("");
  tft.println("WiFi connected.");
  tft.println("IP address: ");
  tft.println(WiFi.localIP());
  delay(1000);
  // Initialize a NTPClient to get time
  timeClient.begin(); 
  changeLocation(locations[current_location]);
  getData();
}

void changeLocation(String nextlocation) {
  tt="";
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.println("Loading:");
  tft.println(nextlocation);
  getLocation(nextlocation);
  town = Location.name;
  Serial.println("latitude: " + Location.latitude);
  Serial.println("longitude: " + Location.longitude);
  Serial.println("Town: " + Location.name);

  int timeoffset = getOffsetByName(Location.timezone.c_str());
  Serial.print("Timeofset TZ: ");
  Serial.print(timeoffset);
  timeoffset += getDSTOffsetInSeconds(Location.country_code.c_str(), timeClient.getEpochTime());
  Serial.print(", Timeoffset TZ incl. DST: ");
  Serial.print(Location.country_code);
  Serial.println(timeoffset);
  timeClient.setTimeOffset(timeoffset);
  Serial.println(timeoffset);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  
  tft.setCursor(2, 232, 1);
  tft.println(WiFi.localIP());
  tft.setCursor(80, 204, 1);
  tft.println("BRIGHT:");

  tft.setCursor(80, 152, 2);
  tft.println("SEC:");
  tft.setTextColor(TFT_WHITE,lightblue);
  tft.setCursor(4, 152, 2);
  tft.println("TEMP:");

  tft.setCursor(4, 192, 2);
  tft.println("HUM: ");
  tft.setTextColor(TFT_WHITE,TFT_BLACK);

  tft.setFreeFont(&Orbitron_Medium_20);
  tft.setCursor(6, 82);
  //tft.println(town);
  int MsgPixWidth = tft.textWidth(town);
  ScrollTextSprite.setFreeFont(&Orbitron_Medium_20);
  ScrollTextSprite.setTextColor(TFT_WHITE,TFT_BLACK); // Yellow text, black background
  ScrollTextSprite.setColorDepth(8); // How big a sprite you can use and how fast it moves is greatly influenced by the color depth.
  ScrollTextSprite.createSprite(tft.width(), ScrollTextSprite.fontHeight() + 2); // Sprite width is display plus the space to allow text to scroll from the right.
  ScrollTextSprite.fillScreen(TFT_BLACK);
  ScrollTextSprite.drawString(town, 0, 0);
  ScrollStep = -1;
  ScrollStepCounter = 0;

  tft.fillRect(68,152,1,74,TFT_GREY);

  for(int i=0;i<b+1;i++)
    tft.fillRect(78+(i*7),216,3,10,blue);
  // Initialize a NTPClient to get time
  getData();
}

int i=0;
int count=0;
bool inv=1;
int press1=0; 
int press2=0;////
int pressboth=0;
int frame=0;
String curSeconds="";

void loop() {
  /*tft.pushImage(0, 88,  135, 65, ani[frame]);
  frame++;
  if(frame>=10)
    frame=0;
  */
 ScrollStepCounter -= ScrollStep;
 if (ScrollTextSprite.textWidth(town) < tft.width()){
  ScrollStep *= 0;
 }
 else if ((ScrollStepCounter > (ScrollTextSprite.textWidth(town) - tft.width())) || ScrollStepCounter <= 0){
  ScrollStep *= -1;
 }
 ScrollTextSprite.fillScreen(TFT_BLACK);
 ScrollTextSprite.drawString(town, -ScrollStepCounter, 0);
 //ScrollTextSprite.scroll(ScrollStep);
 ScrollTextSprite.pushSprite(6, 80 - ScrollTextSprite.fontHeight());
 
 //int offset = (int)(count % (45)) * 3;
 //tft.fillRect(0 + offset, 88, 4, 62, TFT_BLACK);
 if (forecast_mode==FORECASTMODE_TEMP){
  draw_forecast("Temperature",forecast_temp,135, 62, TFT_GREEN);
 }
 if (forecast_mode==FORECASTMODE_HUM){
  draw_forecast("Humidity",forecast_humidity, 135, 62, TFT_YELLOW);
 }
 if (forecast_mode==FORECASTMODE_RAIN){
  draw_forecast("Rain",forecast_rain, 135, 62, TFT_BLUE);
 }
 ForecastSprite.pushSprite(0, 88);

  //draw_forecast(forecast_humidity, 0, 88,  135, 65, TFT_WHITE);
  //draw_forecast(forecast_rain, 0, 88,  135, 65, TFT_BLUE);
  if(digitalRead(35)==0){
    if(press2==0 && (pressboth == 0)) {
      press2=1;
    }
  }
  else if (press2 == 1  && (pressboth == 0))
      press2=2;
  else if (press2 == 1  && (pressboth == 1))
      press2=0;
  else if (press2 == 2)
      press2=0;

  if(digitalRead(0)==0){
    if((press1==0) && (pressboth == 0)) {
      press1=1;

    }
  } 
  else if (press1 == 1  && (pressboth == 0))
      press1=2;
  else if (press1 == 1  && (pressboth == 1))
      press1=0;
  else if (press1 == 2)
      press1=0;

  if ((press1 == 1) && (press2 == 1)){
      pressboth = 1;
  }
  else if ((pressboth==1) && (press1 == 0) ^ (press2 == 0)){
      pressboth = 2;
  }
  else if ((pressboth==2) && (press1 == 0) && (press2 == 0)){
      pressboth = 1;
  }

  if (press1 == 2){
      //inv=!inv;
      current_location=(current_location + 1) % NUMLOCATIONS;
      Serial.println(current_location);
      changeLocation(locations[current_location]);
      //tft.invertDisplay(inv);    
  }
  if (press2 == 2){
      forecast_mode=(forecast_mode+1)%FORECASTMODES;
      Serial.println(forecast_mode);
  }
  if (pressboth == 2){
      tft.fillRect(78,216,44,12,TFT_BLACK);
      b++;
      if(b>=5)
        b=0;

      for(int i=0;i<b+1;i++)
        tft.fillRect(78+(i*7),216,3,10,blue);
      ledcWrite(pwmLedChannelTFT, backlight[b]);
      pressboth = 0;
  }
  if(count==0){
    getData();
  }
  count++;
  if(count>10000)
    count=0;
   
  tft.setFreeFont(&Orbitron_Medium_20);
  tft.setCursor(2, 187);
  tft.println(tmp.substring(0,3));

  tft.setCursor(2, 227);
  tft.println(hum+"%");

  tft.setTextColor(TFT_ORANGE,TFT_BLACK);
  tft.setTextFont(2);
  tft.setCursor(6, 44);
  tft.println(dayStamp);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:tmp13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();
  //Serial.println(formattedDate);

  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
         
  if(curSeconds!=timeStamp.substring(6,8)){
    tft.fillRect(78,170,48,28,darkred);
    tft.setFreeFont(&Orbitron_Light_24);
    tft.setCursor(81, 192);
    tft.println(timeStamp.substring(6,8));
    curSeconds=timeStamp.substring(6,8);
  }

  tft.setFreeFont(&Orbitron_Light_32);
  String current=timeStamp.substring(0,5);
  if(current!=tt) {
    tft.fillRect(3,8,120,30,TFT_BLACK);
    tft.setCursor(5, 34);
    tft.println(timeStamp.substring(0,5));
    tt=timeStamp.substring(0,5);
  }
  
  delay(40);
}

void getLocation(String search) {
  OpenMeteoUrl = String("https://geocoding-api.open-meteo.com/v1/search?name="+urlEncode(search)+"&count=1&language=en&format=json");
  payload = ProxyGetRequest(OpenMeteoUrl.c_str(), 80000);  
  payload.toCharArray(inp,30000);
  deserializeJson(doc,inp);
  Serial.println("location deserialized");
  JsonArray loc_results = doc["results"].as<JsonArray>();
  for (JsonVariant item : loc_results) {
    Location.latitude = item["latitude"].as<String>();
    Location.longitude = item["longitude"].as<String>();
    Location.name = item["name"].as<String>();
    Location.country_code = item["country_code"].as<String>();
    Location.timezone = item["timezone"].as<String>();
  }
  Serial.println("location assigned");
}

void getData() {
  OpenMeteoUrl = String("https://api.open-meteo.com/v1/forecast?latitude="+Location.latitude+"&longitude="+Location.longitude+"&current=temperature_2m,relative_humidity_2m,rain&hourly=temperature_2m,relative_humidity_2m,rain&past_days=0&forecast_days=3&models=best_match");
  Serial.println("weather..");
  payload = ProxyGetRequest(OpenMeteoUrl.c_str(), 900);  
  Serial.println("weather loaded");
  payload.toCharArray(inp,30000);
  deserializeJson(doc,inp);
  Serial.println("weather deserialized");
  JsonArray fc_temp = doc["hourly"]["temperature_2m"].as<JsonArray>();
  JsonArray fc_hum = doc["hourly"]["relative_humidity_2m"].as<JsonArray>();
  JsonArray fc_rain = doc["hourly"]["rain"].as<JsonArray>();
  JsonArray fc_time = doc["hourly"]["time"].as<JsonArray>();
  //tft.pushImage(0, 88,  135, 65, ani[frame]);
  Serial.println("weather assigned");
  
  save_forecast_float(fc_temp, forecast_temp);
  save_forecast_float(fc_hum, forecast_humidity);
  save_forecast_float(fc_rain, forecast_rain);
  save_forecast_string(fc_time, forecast_time);
  
  //draw_forecast(fc_temp, 0, 88,  135, 65, TFT_WHITE);
  /*
  JsonArray fc_hum = doc["hourly"]["relative_humidity_2m"];
  JsonArray fc_rain = doc["hourly"]["rain"];

  Serial.println(fc_temp.size());
  */
  

  tmp = doc["current"]["temperature_2m"].as<String>();
  hum = doc["current"]["relative_humidity_2m"].as<String>();
  currenTS = doc["current"]["time"].as<String>();
  
  Serial.println("Temperature: " + tmp);
  Serial.println("Humidity: " + hum);
}

void save_forecast_float(JsonArray values, int * target){
  //Serial.println(values.size());
  float value = 0;
  float maxvalue=0;
  float minvalue=10000;
  int count=0;
  for (JsonVariant item : values) {
    value=item.as<float>();
    target[count] = (int)value+0.5;
    count++;
    if (count >= MAXFORECASTS){
      break;
    }
    //Serial.println(value);
  }  
  forecast_count = count;
}

void save_forecast_string(JsonArray values, String * target){
  //Serial.println(values.size());
  String value = "";
  int count=0;
  for (JsonVariant item : values) {
    value=item.as<String>();
    target[count] = value;
    count++;
    if (count >= MAXFORECASTS){
      break;
    }
    //Serial.println(value);
  }  
  forecast_count = count;
}

int colormod = 0;

void draw_forecast(String label, int * values, int sizex, int sizey, uint16_t color){
  float value = 0;
  float maxvalue=0;
  float minvalue=10000;
  ForecastSprite.createSprite(sizex,sizey);
  ForecastSprite.fillScreen(TFT_BLACK);
  if (forecast_count == 0){
    return;
  }

  for (int i=0; i<forecast_count; i++) {
    value=values[i];
    if (value > maxvalue){
      maxvalue = value;
    }
    if (value < minvalue){
      minvalue = value;
    }
  }  
  draw_grid(sizex,sizey,TFT_GREY);
  int curx = 0;
  int cury = 0;
  int hilite = count % forecast_count;
  uint16_t drawcolor = color;
  for (int i=0; i<forecast_count + 30; i++) {
    uint8_t iv = i % forecast_count;
    value=values[iv];
    int drawx = map(iv, 0, forecast_count, 0, sizex);
    int drawy = map(value, int(minvalue), int(maxvalue), sizey, 0);
    if (curx>0){
      if (hilite == iv){
        colormod=20;
      }
      if (colormod>0){
        drawcolor = colorbrightness(color, 100-colormod);
        colormod-=1;
      }
      else{
        drawcolor = color;      
      }
      if ((abs(curx)-abs(drawx)) < (sizex-10)){
        ForecastSprite.drawLine(curx, cury, drawx, drawy, drawcolor);
      }
    }
    curx=drawx;
    cury=drawy;
  }
  draw_labels(label, minvalue, maxvalue, sizex,sizey,TFT_WHITE);
 
}

void draw_grid2(String label, int x, int y, int sizex, int sizey, uint16_t color){
  for (int i=0; i<sizey; i+= (sizey / 4)){
      tft.drawLine(x, y + i, x + sizex, y + i, color);
  }  
  for (int i=0; i<sizex; i+= (sizex / 5)){
      tft.drawLine(x + i, y, x + i, y + sizey, color);
  }  
}


void draw_grid( int sizex, int sizey, uint16_t color) {
  // Draw grid lines
  String search = "FOO";
  for (int i=0; i<forecast_count; i++) {
    if (forecast_time[i].indexOf(search) == -1){
      int drawx = map(i, 0, forecast_count, 0, sizex);
      ForecastSprite.drawLine(drawx, 0, drawx, sizey, color);
    }
    if (forecast_time[i].indexOf(currenTS.substring(0,14)) > -1){
      int drawx = map(i, 0, forecast_count, 0, sizex);
      ForecastSprite.drawLine(drawx, 0, drawx, sizey, TFT_RED);
    }
    search=forecast_time[i].substring(0,10);
  }
  for (int i = 0; i < sizey; i += (sizey / 4)) {
     ForecastSprite.drawLine(0, i, sizex, i, color);
  }
  ForecastSprite.drawLine(0, 0, sizex, 0, TFT_LIGHTGREY);
  ForecastSprite.drawLine(0, sizey, sizex, sizey, TFT_LIGHTGREY);
  /*for (int i = 0; i < sizex; i += (sizex / 5)) {
    tft.drawLine(x + i, y, x + i, y + sizey, color);
  }*/

}
void draw_labels(String label, int rangemin, int rangemax, int sizex, int sizey, uint16_t color) {
  // Draw mainlabel
  ForecastSprite.setTextFont(1);
  ForecastSprite.setTextSize(1);
  ForecastSprite.setCursor(2, 3);
  ForecastSprite.print(label);

  ForecastSprite.setTextColor(TFT_WHITE);
  // Draw small labels and values
  ForecastSprite.setTextFont(2);
  int valueWidth = ForecastSprite.textWidth(String(rangemin));
  ForecastSprite.setCursor(sizex - valueWidth, sizey - 16);
  ForecastSprite.print(rangemin);
  valueWidth = ForecastSprite.textWidth(String(rangemax));
  ForecastSprite.setCursor(sizex - valueWidth, 10 - 9);
  ForecastSprite.print(rangemax);
  ForecastSprite.setTextSize(1);
}

void color16toRGB(uint16_t color565, uint8_t *rgb)
{
  rgb[0] = (color565 >> 8) & 0xF8; rgb[0] |= (rgb[0] >> 5);
  rgb[1] = (color565 >> 3) & 0xFC; rgb[1] |= (rgb[1] >> 6);
  rgb[2] = (color565 << 3) & 0xF8; rgb[2] |= (rgb[2] >> 5);
}

uint16_t colorbrightness(uint16_t color, int percentage){
  uint8_t rgb[] = {0, 0, 0};
  color16toRGB(color, rgb);
  float factor = percentage / 100.;
  rgb[0] = rgb[0] * factor <= 255?rgb[0] * factor : 255;
  rgb[1] = rgb[1] * factor <= 255?rgb[1] * factor : 255;
  rgb[2] = rgb[2] * factor <= 255?rgb[2] * factor : 255;
  return tft.color565(rgb[0], rgb[1], rgb[2]);
}
