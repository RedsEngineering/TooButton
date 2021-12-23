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
#include <OctoPrintAPI.h>
//#include <ESP_Mail_Client.h>
#define HOST_NAME "Too Button"

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
//bool gReverseDirection = false;
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
//IPAdress ip = configManager.data.octoprintip;
//IPAddress ip(192, 168, 2, 191);  
char *octoprint_host = configManager.data.octoprintip; // Or your hostname. Comment out one or the other.
const int octoprint_httpPort = 80;                     //If you are connecting through a router this will work, but you need a random port forwarded to the OctoPrint server from your router. Enter that port here if you are external
//String octoprint_apikey = "C2CF813EAF6E49B1A3DC9B1C3E4C8728"; //See top of file or GIT Readme about getting API key good key
char *octoprint_apikey = configManager.data.octoprintapikey;
unsigned long api_mtbs = 5000; //mean time between api requests (10 seconds)
unsigned long api_lasttime = 0; //last time api request has been done
byte connection_retry = 0;
byte point = 0;

long printed_timeout = 3600000; //60 mins in milliseconds - timeout after printing completed, to clear strip
long printed_timeout_timer = printed_timeout;
//dashboardData data;

int led1;
int led2;

unsigned long previousMillis = 0;
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
    13,   // Input pin for the button
    true, // Button is active LOW
    true  // Enable internal pull-up resistor
);

OneButton button2 = OneButton(
    14,   // Input pin for the button
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

unsigned long now = millis();

RemoteDebug Debug;

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
   leds[0] = led_color[led1_color];
    FastLED.show();
  WiFiClient client1;
  OctoprintApi api(client1, octoprint_host, octoprint_httpPort, octoprint_apikey); 

  if (millis() - api_lasttime > api_mtbs || api_lasttime == 0)
  { //Check if time has expired to go check OctoPrint
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
        api.octoPrintCoreShutdown();
       
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

      case 2:
        
         Debug.println("Octoprint Start Job");
         Debug.println("");
         api.octoPrintJobStart();
        break;
      case 3:
       
           Debug.println("Octoprint Cancel Job");
           Debug.println("");
          api.octoPrintJobCancel();
          break;
      case 4:
        
        Debug.println("Octoprint Pause Job");
        Debug.println("");
        api.octoPrintJobPause();
        break;
      case 5:
          
              Debug.println("Octoprint Resume Job");
              Debug.println("");
              api.octoPrintJobResume();
        break;
      
       
      case 6:
      
      
         Debug.println("Octoprint Restart Job");
         Debug.println("");
           api.octoPrintJobRestart();
        break;
      default:
        // if nothing else matches, do the default
        // default is optional
        break;
      }
    }
    api_lasttime = millis(); //Set api_lasttime to current milliseconds run
  }

  else
  {
Debug.println("Mean time between OctoPrint API calls 5 seconds.");

  }
 
} // octoPrint


void click1()
{
 
maxflash1 = 1;
  Debug.println("Button 1 click.");
  Debug.printf("Power status: %d\n", power);
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

  LittleFS.begin();
  GUI.begin();
  configManager.begin();
  configManager.setConfigSaveCallback(saveCallback);
  WiFiManager.begin(configManager.data.projectName);
  timeSync.begin();
  dash.begin(500);

  // Initialize the server (telnet or web socket) of RemoteDebug
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
  int led_color1;
  int led_color2;
  int brightness;
  led1_color = configManager.data.led1_color;
    led2_color = configManager.data.led2_color;
    leds[1] = led_color[led2_color];
    FastLED.show();
    leds[0] = led_color[led1_color];
    FastLED.show();
   
  
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

    led1_color = configManager.data.led1_color;
    led2_color = configManager.data.led2_color;
    leds[1] = led_color[led2_color];
    FastLED.show();
    leds[0] = led_color[led1_color];
    FastLED.show();
    configManager.save();
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
  led1 = digitalRead(13); // read the input pin
  led2 = digitalRead(14); // read the input pin

  if (maxflash1 == 1)
  {
    if (configManager.data.enableledflash)

    {

      if (led1 == 0 && led1_color != 0)
      {

        unsigned long currentMillis = millis();
        if (currentMillis - previousMillis >= interval)
        {

          previousMillis = currentMillis;
          leds[0] = CRGB::Black;
          FastLED.show();
          delay(50);
          leds[0] = led_color[led1_color];
          FastLED.show();
        }
      }
    }

    if (maxflash2 == 1)
    {

      if (configManager.data.enableledflash)
      {
        if (led2 == 0 && led2_color != 0)
        {

          unsigned long currentMillis = millis();
          if (currentMillis - previousMillis >= interval)
          {

            previousMillis = currentMillis;

            leds[1] = CRGB::Black;
            FastLED.show();

            delay(50);

            leds[1] = led_color[led2_color];
            FastLED.show();

            // set the LED with the ledState of the variable:
          }
        }
      }
    }
  } //enable disable led flash
}
