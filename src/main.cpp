#include <Arduino.h>
#include "LittleFS.h"
#include "Wire.h"
#include "WiFiManager.h"
#include "webServer.h"
#include "updater.h"
#include "configManager.h"
#include "dashboard.h"
#include "timeSync.h"
#include "RemoteDebug.h"
#include "OneButton.h"
#include "timeSync.h"
#include <EEPROM.h>
//#include <ESP_Mail_Client.h>
#define HOST_NAME "Too Button"


#define XSTR(x) #x
#define STR(x) XSTR(x)

#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>

configData data;
#define LED_PIN 12
#define COLOR_ORDER GRB
#define CHIPSET WS2812
#define NUM_LEDS 3

#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 60
#define FASTLED_ESP8266_RAW_PIN_ORDER

#define FASTLED_ALLOW_INTERRUPTS 1


 void octoprint(int);
CRGB leds[NUM_LEDS];
CRGB led_color[] = {
    CRGB::Black,
    CRGB::MidnightBlue,
    CRGB::Red,
    CRGB::Green,
    CRGB::Purple,
    CRGB::Yellow,
    CRGB::Lime,
    CRGB::DarkViolet,
    CRGB::Teal,
    CRGB::FireBrick,
    CRGB::White};

int led1_color = 0;
int led2_color = 0;
#define ROW 20
#define COL  20

char ArrayOfString[ROW][COL];
int led_color1;
  int led_color2;
  int brightness;
int feedrate=100;
char gcode[70];
char CMD[40];
char version[20];
unsigned long api_mtbs = 5000; //mean time between api requests (5 seconds)
unsigned long api_lasttime = 0; //last time api request has been done
byte connection_retry = 0;
byte point = 0;

long printed_timeout = 3600000; //60 mins in milliseconds - timeout after printing completed, to clear strip
long printed_timeout_timer = printed_timeout;
//dashboardData data;
//strcpy(configManager.data.projectVersion,"copy successful");

int switch1;
int switch2;
uint8_t rr;
const long interval = 50;

String luvmsg = "";
//const char* tplinkip = "10.0.0.117";
//const char *tplinkip = configManager.data.tplinktip;
char clientBuf[64];
int power = 0;
struct task
{
  unsigned long rate;
  unsigned long previous;
};

task taskA = {.rate = 60000, .previous = 0};

void saveCallback()
{
  //  Debug.println("EEPROM saved");
}

OneButton button1 = OneButton(
    14,   // Input pin for the button
    true, // Button is active LOW
    true  // Enable internal pull-up resistor
);

OneButton button2 = OneButton(
    13,   // Input pin for the button
    true, // Button is active LOW
    true  // Enable internal pull-up resistor
);

OneButton button3 = OneButton(
    4,    // Input pin for the button
    true, // Button is active LOW
    true  // Enable internal pull-up resistor
);

OneButton button4 = OneButton(
    3,    // Input pin for the button
    true, // Button is active LOW
    true  // Enable internal pull-up resistor
);

int ledswitch1 = 12;
int ledswitch2 = 16;
int ledswitch3 = 5;
int ledswitch4 = 1;
unsigned long maxflash1 = 1;
unsigned long maxflash2 = 1;
int addr = 0;
unsigned long now = millis();

RemoteDebug Debug;




size_t vSeparateSringByComma (char* string)
{
    const char *delims = ",\n";
    char *s = string;
    size_t n = 0, len;

    for (s = strtok (s, delims); s && n < ROW; s = strtok (NULL, delims))
        if ((len = strlen (s)) < COL)
            strcpy (ArrayOfString[n++], s);
        else
            fprintf (stderr, "error: '%s' exceeds COL - 1 chars.\n", s);

    return n;
}





void sendcommand(String uri, String postdata = ""){
if (rr != 50)
   exit(0);
WiFiClient client3;
IPAddress addr;

if (!client3.connect(configManager.data.octoprintip, configManager.data.octoprintport)){
    Debug.println("octoRequest Connection failed!");
    return;
  } else {
    Debug.println("Connected to server!");
    
    client3.println("POST " +uri+" HTTP/1.1");
   
     client3.printf("Host: %s\n", configManager.data.octoprintip);
    client3.println("Cache-Control: no-cache");
     
      client3.printf("X-Api-Key: %s\n", configManager.data.octoprintapikey);
      client3.println("postData JSON: "+postdata);
      client3.println("Content-Type: application/json");
      client3.print("Content-Length: ");
      client3.println(postdata.length());
   
    client3.println("");
  
      client3.println(postdata);
  
    client3.println("");

    client3.stop();
    return;   

  }

}
void smartplug()
{

  if (power == 1)
  {

    WiFiClient client;

    if (!client.connect(configManager.data.tplinktip, 9999))
    {
      Debug.println("connection to tplink fail");

      return;
    }
    else
    {
      Debug.println("connected to tplink, turning off.");

      //char packet[] = "\x00\x00\x00*\xd0\xf2\x81\xf8\x8b\xff\x9a\xf7\xd5\xef\x94\xb6\xc5\xa0\xd4\x8b\xf9\x9c\xf0\x91\xe8\xb7\xc4\xb0\xd1\xa5\xc0\xe2\xd8\xa3\x81\xf2\x86\xe7\x93\xf6\xd4\xee\xdf\xa2\xdf\xa2";
      char packet[] = "\x00\x00\x00*\xd0\xf2\x81\xf8\x8b\xff\x9a\xf7\xd5\xef\x94\xb6\xc5\xa0\xd4\x8b\xf9\x9c\xf0\x91\xe8\xb7\xc4\xb0\xd1\xa5\xc0\xe2\xd8\xa3\x81\xf2\x86\xe7\x93\xf6\xd4\xee\xde\xa3\xde\xa3";
      client.write((const uint8_t *)packet, 64);

      power = 0;

      return;
    }
  }

  if (power == 0)
  {
    if (rr != 50)
     exit(0);
    WiFiClient client;

    if (!client.connect(configManager.data.tplinktip, 9999))
    {
      Debug.println("connection to tplink fail");

      return;
    }
    else
    {
      Debug.println("connected to tplink, turning on.");
      char packet[] = "\x00\x00\x00*\xd0\xf2\x81\xf8\x8b\xff\x9a\xf7\xd5\xef\x94\xb6\xc5\xa0\xd4\x8b\xf9\x9c\xf0\x91\xe8\xb7\xc4\xb0\xd1\xa5\xc0\xe2\xd8\xa3\x81\xf2\x86\xe7\x93\xf6\xd4\xee\xdf\xa2\xdf\xa2";
      client.write((const uint8_t *)packet, 64);
      power = 1;

      return;
    }
  }
}

void octoPrnt(int opcall)
{
 
 Debug.println("Octoprint() call");
  delay(200);
   leds[0] = led_color[led1_color];
    FastLED.show();
  
 

 
    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status
     //  Debug.println("Connected to Octoprint");
      switch (opcall)
      {
      
      case 0:
      {
        break;
      }
      
      case 1:
       {
        if (power == 0) {
         Debug.println("Octoprint() call");
    
          Debug.printf("Power status: %s\n", configManager.data.tplinktip);
     if(strcmp(configManager.data.tplinktip, "0.0.0.0") == 0){
     
        Debug.println("Cycling internal power status to On. No valid IP for Smart Plug.");
        power = 1;
      }
     else {
        Debug.printf("Power status: %d\n", power);
        Debug.println("Turning Smartplug On");
        Debug.println("");
        
        delay(200);
        smartplug();


     }
     
      }
       else
       {

        Debug.printf("Power status: %d\n", power);
       
       
       
        delay(200);
        
        sendcommand("/api/system/commands/core/shutdown", "");
        
       
       
        if(strcmp(configManager.data.tplinktip, "0.0.0.0") != 0){
         Debug.println("Turning smartplug Off, delay 15 seconds.");
        delay(15000);
      
        Debug.println("");
        delay(200);
        smartplug();
        }
        else
        {
         
        Debug.println("Cycling internal power status to off. No valid IP for Smart Plug. Just shutting down Octoprint.");
         power=0;
        }

       }
        
        break;
      }
      case 2:
       {        
         Debug.println("Octoprint Start Job");
         Debug.println("");
        
         strcpy(CMD, "{\"command\": \"start\"}"); 
        sendcommand("/api/job", CMD);
        
               
        break;
      }
      case 3:
       {
           Debug.println("Octoprint Cancel Job");
           Debug.println("");
     
          strcpy(CMD, "{\"command\": \"cancel\"}"); 
          
          sendcommand("/api/job", CMD);
         
        Debug.printf("Cancel Job\n%s\n", CMD);

          break;
       }
      case 4:
       { 
        Debug.println("Octoprint Pause Job");
        Debug.println("");
       
          strcpy(CMD, "{\"command\": \"pause\"}"); 
         sendcommand("/api/job", CMD);
        break;
      }
      case 5:
      {    
              Debug.println("Octoprint Resume Job");
              Debug.println("");
        
              strcpy(CMD, "{\"command\": \"resume\"}"); 
              sendcommand("/api/job", CMD );
             
           
        break;
      }
       
      case 6:
      {
         Debug.println("Octoprint Restart Job");
         Debug.println("");
       
           strcpy(CMD, "{\"command\": \"restart\"}"); 
           sendcommand("/api/job", CMD);
          
         
        break;
      }
     case 7:
     {
         feedrate=feedrate+2;
       
              strcpy(CMD, "{\"command\": \"feedrate\", \"factor\": "); 
              char buffer[10];
              snprintf(buffer, 10, "%d", feedrate);
             
              strcat(CMD, buffer);
              strcat(CMD, "}");

        Debug.printf("Feedrate +2 percent%s\n", CMD);
      sendcommand("/api/printer/printhead", CMD);



     break;
     }
     case 8:
     {
         feedrate=feedrate-2;
       
              strcpy(CMD, "{\"command\": \"feedrate\", \"factor\": "); 
              char buffer[10];
              snprintf(buffer, 10, "%d", feedrate);
             
              strcat(CMD, buffer);
              strcat(CMD, "}");

        
        Debug.printf("Feedrate -2 percent%s\n", CMD);
      sendcommand("/api/printer/printhead", CMD);


     break;
     }
     case 9:
     {
       
         strcpy(gcode, configManager.data.customgcode1);
     size_t n = vSeparateSringByComma (gcode);

    for (size_t i = 0; i < n; i++){
     //  Debug.printf ("ArrayOfString[%zu] : '%s'\n", i, ArrayOfString[i]);
       strcpy(CMD, "{\"command\": \"");
       strcat(CMD, ArrayOfString[i]);
        strcat(CMD, "\"}");
      Debug.printf("Feedrate +2% 1%s\n", CMD);
      sendcommand("/api/printer/command", CMD);
   }  
              
              
     break;
     }
     case 10:
  {
       
         strcpy(gcode, configManager.data.customgcode2);
     size_t n = vSeparateSringByComma (gcode);

    for (size_t i = 0; i < n; i++){
     //  Debug.printf ("ArrayOfString[%zu] : '%s'\n", i, ArrayOfString[i]);
       strcpy(CMD, "{\"command\": \"");
       strcat(CMD, ArrayOfString[i]);
        strcat(CMD, "\"}");
        Debug.printf("Custom Gcode command 2%s\n", CMD);
      sendcommand("/api/printer/command", CMD);
   }  
              
              
     break;
     }
     break;

    case 11:
    {
     strcpy(CMD, "{\"command\": \"pause\", \"action\": \"toggle\"}"); 
             

        
        Debug.printf("Pause / Resume toggle%s\n", CMD);
      sendcommand("/api/job", CMD);
                 
     break;
    }
     
    case 12:
    {

    
     

     strcpy(CMD, "{\"commands\":[\"M300\",\"S440\",\"P200\"]}");
              sendcommand("/api/printer/command", CMD);
                 Debug.printf("Beep %s\n", CMD);


    } 
     
     
    default:
        // if nothing else matches, do the default
        // default is optional
        break;
      }
    }
    api_lasttime = millis(); //Set api_lasttime to current milliseconds run
  

 
 
} // octoPrint


void click1()
{
 maxflash1 = 1;
 
  Debug.println("Button 2 click.");
 Debug.printf("Memory status: %d\n", rr);

 octoPrnt(configManager.data.button1_click);
} // click1

void click2()
{
 maxflash2 = 1;
  Debug.println("Button 2 click.");
  octoPrnt(configManager.data.button2_click);
} // click2

void doubleclick1()
{
maxflash1 = 1;
  Debug.println("Button 1 doubleclick.");
  
octoPrnt(configManager.data.button1_doubleclick);
} // doubleclick1

void doubleclick2()
{
  maxflash2 = 1;
   Debug.println("Button 2 doubleclick.");
 octoPrnt(configManager.data.button2_doubleclick);
} // doubleclick2

void longPressStart1()
{
  Debug.println("Button 1 longPress start");
  maxflash1 = 0;
octoPrnt(configManager.data.button1_hold);
} // longPress1

void longPressStart2()
{
  maxflash2 = 0;
   Debug.println("Button 2 longPress start");
 octoPrnt(configManager.data.button2_hold);
} // longPressStart2

void longPressStop2()
{
  maxflash2 = 1;
   Debug.println("Button 1 longPress stop");
 
} // longPressStop2

void longPressStop1()
{
  maxflash1 = 1;
   Debug.println("Button 2 longPress stop");
 
} // longPressStop2







 
 

void setup()
{

 


  


  pinMode(ledswitch1, OUTPUT);
  pinMode(ledswitch2, OUTPUT);

  WiFiClient client;
   EEPROM.begin(512);  
   EEPROM.get(0x11, rr);
  LittleFS.begin();
  GUI.begin();
  configManager.begin();
  configManager.setConfigSaveCallback(saveCallback);
  WiFiManager.begin(configManager.data.projectName);
  timeSync.begin();
  dash.begin(500);
  strcpy (version, AUTO_VERSION);
  int len = strlen(version);
  version[len-9] = '\0';
  
  strcpy(configManager.data.projectVersion, version);
  Debug.begin(HOST_NAME);
  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachLongPressStart(longPressStart1);

  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachLongPressStart(longPressStart2);
  button2.attachLongPressStop(longPressStop2);
  //button2.attachDuringLongPress(longPressStart2);
  button1.attachLongPressStop(longPressStop1);
  //button1.attachDuringLongPress(longPressStart1);
  button1.setDebounceTicks(25);
  button2.setDebounceTicks(25);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  FastLED.setBrightness(configManager.data.led_brightness * 25.5);
}

void loop()
{
  

    if (rr == 50){
    led1_color = configManager.data.led1_color;
    led2_color = configManager.data.led2_color;
    leds[1] = led_color[led2_color];
    FastLED.show();
    leds[0] = led_color[led1_color];
    FastLED.show();
}
  
  if (brightness != (configManager.data.led_brightness * 25.5))
  {

    FastLED.setBrightness(configManager.data.led_brightness * 25.5);
    brightness = (configManager.data.led_brightness * 25.5);
  }

  if (taskA.previous == 0 || (millis() - taskA.previous > taskA.rate))
  {
    taskA.previous = millis();

    //write directly to the configData ram mirror
    //configManager.data.dummyInt++;
    //configManager.data.projectName;
    //save the newest values in the EEPROM
    if (rr == 50){
    led1_color = configManager.data.led1_color;
    led2_color = configManager.data.led2_color;
    leds[1] = led_color[led2_color];
    FastLED.show();
    leds[0] = led_color[led1_color];
    FastLED.show();
    //char *octoprint_host = configManager.data.octoprintip; // Or your hostname. Comment out one or the other.
    //char *octoprint_apikey = configManager.data.octoprintapikey;
    configManager.save();
    }
  }

  //software interrupts
  WiFiManager.loop();
  updater.loop();
  configManager.loop();
  dash.loop();
  Debug.handle();
  button1.tick();
  button2.tick();

  // You can implement other code in here or just wait a while
  //delay(10);
switch1 = digitalRead(13); // read the input pin
switch2 = digitalRead(14); // read the input pin
if (switch1 ==0 && switch2 == 0)
{

  delay(5000);

switch1 = digitalRead(13); // read the input pin
switch2 = digitalRead(14); // read the input pin

  if (switch1 ==0 && switch2 == 0){


 

    leds[1] = CRGB::Blue;
    FastLED.show();
    leds[0] = CRGB::Blue;
    FastLED.show();
    
    delay(5000);
  switch1 = digitalRead(13); // read the input pin
  switch2 = digitalRead(14); // read the input pin

 if (switch1 ==1 && switch2 == 0){




    leds[1] = CRGB::Black;
    FastLED.show();
    leds[0] = CRGB::Black;
    FastLED.show();
   






 
  EEPROM.begin(512);
  delay(100);
  EEPROM.write(0x11, 50);
  delay(100);
  EEPROM.commit();  
  delay(100);
  ESP.restart();
 }



  }     

}




  if (maxflash1 == 1)
  {
    if (configManager.data.enableledflash)

    {

      if (switch1 == 0 && led1_color != 0)
      {

       
       

       
          leds[0] = CRGB::Black;
          FastLED.show();
          delay(50);
          leds[0] = led_color[led1_color];
          FastLED.show();
        
      }
    }

    if (maxflash2 == 1)
    {

      if (configManager.data.enableledflash)
      {
        if (switch2 == 0 && led2_color != 0)
        {

         

         

            leds[1] = CRGB::Black;
            FastLED.show();

            delay(50);

            leds[1] = led_color[led2_color];
            FastLED.show();

            // set the LED with the ledState of the variable:
          
        }
      }
    }
  } //enable disable led flash
}
