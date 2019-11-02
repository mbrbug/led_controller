#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
//#include "libs/FastLED/FastLED.h"
//#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_ESP8266_DMA
#define FASTLED_INTERRUPT_RETRY_COUNT 1
//#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"
#include "PubSubClient.h"
//#include <PubSubClient.h>

#define NUM_LEDS 27  // # of LEDS in the strip
CRGB leds[NUM_LEDS];
CHSV hsv[NUM_LEDS];
//#define LEDPIN D5
#define LEDPIN 3
#define COLOR_ORDER GRB

MDNSResponder mdns;

const char* ssid = "mbr";
const char* wifipass = "l2NBYULiCPcPPxjcPQy5ypnE";
const char* mqtt_server = "192.168.111.2";

//#define topic_scene "/myhome/in/RGBTest_scene"
//#define topic_rgb "/myhome/in/RGBTest"
//#define topic_rgb_out "/myhome/out/RGBTest"
//#define topic_info "BED"
#define mqtt_item "BED"
#define mqtt_global "/myhome/"

String topic_scene_in = "/myhome/out/BED_scene/command"; //changed
String topic_rgb_in = "/myhome/out/BED_rgb/command"; //changed
String topic_rgb_out = "/myhome/in/BED_rgb/state"; //changed
String topic_auto_in = "/myhome/in/BED_auto"; //changed
String topic_RSSI = "/myhome/out/BED_RSSI";
String topic_IP = "/myhome/out/BED_IP";
String topic_mac = "/myhome/out/BED_mac";
String topic_uptime = "/myhome/out/BED_uptime";
//String topic_item_in = "/myhome/in/BED";
String topic_item_out = "/myhome/in/BED/state";

String topic_auto_level = "/myhome/out/BED_auto_level/command";

//String topic_scene_in = "/myhome/in/BED_scene";
//String topic_rgb_in = "/myhome/in/BED_rgb";
//String topic_rgb_out = "/myhome/out/BED_rgb";
//String topic_auto_in = "/myhome/out/BED_auto";
//String topic_RSSI = "/myhome/out/BED_RSSI";
//String topic_IP = "/myhome/out/BED_IP";
//String topic_mac = "/myhome/out/BED_mac";
//String topic_uptime = "/myhome/out/BED_uptime";
//String topic_item_in = "/myhome/in/BED";
//String topic_item_out = "/myhome/out/BED";

IPAddress server(192, 168, 111, 2);

int brightness = 0;

#define BUFFER_SIZE 100

//String topic = "OpenHab/RGB/#";//this is your 'catch all'  for MQTT topics

WiFiClient wclient;
PubSubClient client(wclient, server);


boolean rgb = false;
boolean rgb_on = false;
boolean light_on = false;
boolean animate_on = false;
boolean auto_on = true;
boolean fade_on = false;
boolean button_on = false;
boolean autolight_crgb = false;
byte mode;
//String scene;
int autobright = 0;

#define pushButtonD6 D6
int buttonD6State = 0;         // current state of the button
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 600;

int scene = 0;
uint8_t lval = 0;
uint8_t lhue = 0;
int val = 0;
uint8_t sat = 0;
uint8_t hue = 255;

#define TRANSMITINTERVAL 300
long  TXinterval = TRANSMITINTERVAL; // can set this in userdata.h
int   signalStrength;             // WiFi signal strength
long  lastMinute = -1;            // timestamp last minute
long  upTime = 0;
String IP;
String MAC;

// OTA configuration
//
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#define HOST  "OTA-fastled" //your OTA identifier
const char* host = HOST; //set this in userdata.h

void callback(const MQTT::Publish& pub);
void rainbow();
void rainbow_beat();
void rainbowbeat();
void rainbowmarch();
void rainbow_march();
void fade(int autobright);
//void bright50();
//void bright220();
void fadeout();
//void cylonup();
//void cylondown();
void cylon();
void setupOTA();
//void rainbowfade();
//void animate();
//void mode();
void light();
void animate();

void setup() {

  //topic_scene = mqtt_global "in/" mqtt_item "_scene";

  Serial.begin(115200);
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);
  delay(250);
  digitalWrite(LED_BUILTIN, 1);
  delay(250);
  digitalWrite(LED_BUILTIN, 0);
  delay(250);
  digitalWrite(LED_BUILTIN, 1);
  delay(250);
  digitalWrite(LED_BUILTIN, 0);
  delay(250);
  digitalWrite(LED_BUILTIN, 1);

  //pinMode(pushButtonD6, INPUT);

  FastLED.addLeds<WS2812B, LEDPIN, COLOR_ORDER>(leds, NUM_LEDS).setTemperature( Tungsten40W );
  FastLED.clear();
  FastLED.show();

  LEDS.showColor(CRGB(255, 0, 0));
  delay(300);
  LEDS.showColor(CRGB(0, 255, 0));
  delay(300);
  LEDS.showColor(CRGB(0, 0, 255));
  delay(300);
  LEDS.showColor(CRGB(0, 0, 0));

  setupOTA();

  //setup_Wifi();

  //client.setCallback(callback);

MAC = WiFi.macAddress();

}

void loop() {
//Serial.print("scene = ");
//Serial.println(scene);
//Serial.print("light_on = ");
//Serial.println(light_on);
//Serial.print("animate_on = ");
//Serial.println(animate_on);
//Serial.print("fade_on = ");
//Serial.println(fade_on);
//Serial.print("auto_on = ");
//Serial.println(auto_on);
//Serial.println(topic_scene);
//delay(250);
  ArduinoOTA.handle();
  if (millis() % (TXinterval * 1000) == 0) {
     //client.publish("home/nb/weer/version", VERSION); // no quotes or   "String" as it is a macro
     signalStrength = WiFi.RSSI();
     client.publish(topic_RSSI, String(signalStrength));// these topics are my choice, can change them, but then do as well in yr itemsfile
     IP = WiFi.localIP().toString();
     client.publish(topic_IP, String(IP));
     client.publish(topic_mac, String(MAC));
     client.publish(topic_uptime, String(upTime));
     //Serial.println(MAC);
   }
   // INCREASE UPTIME

 //Serial.println("o");

   if (lastMinute != (millis() / 60000)) {         // another minute passed ?
     lastMinute = millis() / 60000;
     upTime++;
   }

//Serial.print(".");
//delay(10);
//Serial.println(scene);

 if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, wifipass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    Serial.println("WiFi connected");
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.mode(WIFI_STA);
    if (!client.connected()) {

      Serial.println("Connecting to MQTT server");
      if (client.connect(MQTT::Connect("led_control")
			 .set_auth("admin", "y30TtxXvy7nJirPlvvoW"))) {
        Serial.println("Connected to MQTT server");
        client.publish("outTopic","hello world123");
        client.set_callback(callback);
        client.subscribe(topic_rgb_in);
        client.subscribe(topic_scene_in);
        client.subscribe(topic_auto_level);
        //client.subscribe(topic_item_in);
        signalStrength = WiFi.RSSI();
        client.publish(topic_RSSI, String(signalStrength));// these topics are my choice, can change them, but then do as well in yr itemsfile
        IP = WiFi.localIP().toString();
        //client.publish("home/nb/weer/IP", String(IP));
        client.publish(topic_IP, String(IP));
        client.publish(topic_mac, String(MAC));
        //client.publish("home/nb/weer/uptime", String(upTime));


      }
  //    if (client.connect("BED_rgb")) {
  //client.publish("outTopic","hello world1");
  //signalStrength = WiFi.RSSI();
  //client.publish(topic_RSSI, String(signalStrength));// these topics are my choice, can change them, but then do as well in yr itemsfile
  //IP = WiFi.localIP().toString();
  ////client.publish("home/nb/weer/IP", String(IP));
  //client.publish(topic_IP, String(IP));
  //client.publish(topic_mac, String(MAC));
  //client.publish("home/nb/weer/uptime", String(upTime));
  //client.set_callback(callback);
  //client.subscribe(topic_rgb_in);
  //client.subscribe(topic_scene_in);
  //client.subscribe("/myhome/in/BED_auto_level");
  //client.subscribe(topic_item_in);
  //client.subscribe("OpenHab/RGB/HSL");
  //Serial.println("WiFi Connected");
  //    }
    }

    if (client.connected())
      client.loop();
      //Serial.println("loop");
  }

//Serial.println("before:");
//Serial.println(scene);

  int buttonD6State = digitalRead(pushButtonD6);
  //int reading = buttonD6State;
  //Serial.println(reading);
  //delay(1000);

  //if (reading != lastButtonD6State) {
    // reset the debouncing timer
    //lastDebounceTime = millis();
  //}

  if ((millis() - lastDebounceTime) > debounceDelay)
  {
    //Serial.println("yepp!");
    //delay(1000);
    //Serial.println(buttonD6State);
    //Serial.println(buttonState);
    //delay(1000);
    if (buttonD6State == HIGH)
       {
         Serial.println("yepp!2");
         lastDebounceTime = millis();
         Serial.println("Button pressed");
           if (light_on == true)
           {
             //light_on = false;
            // delay(100);
              Serial.println("Button pressed, light is on, off");
              fade_on = true;
              animate_on = false;
              button_on = false;
              rgb = false;
              auto_on = false;
              //client.publish(topic_item_out, "OFF");
              //mode = 0

           }
           else
           {
             Serial.println("Button pressed, light is off, on");
             autolight_crgb = true;
             Serial.println("autolight_crgb = true");
             fade_on = true;
             button_on = true;
             //client.publish(topic_item_out, "ON");
             //mode = 0

           }
       }
  }

//lastButtonD6State = reading;


if ((light_on == false) && (animate_on == false) && (button_on == true) && (fade_on == true))
{
  mode = 1;
  autobright = 100;
  sat = 0;
  light();
  //client.publish(topic_rgb_out, "ON");
}



if ((fade_on == true) && (rgb == true))
{
  mode = 3;
  light();
  //light_on = true;
  //fade_on = false;
  //rgb = false;

}

if ((light_on == true) && (animate_on == false) && (fade_on == true) && (rgb == false) && (button_on == false) && (auto_on == false))
{
    mode = 0;
    autobright = 0;
    light();
    //client.publish(topic_rgb_out, "OFF");
    //client.publish(topic_item_out, "OFF");
}

if ((rgb_on == true) && (fade_on == true))
{
  mode = 1;
  autobright = 100;
  //sat = 0;
  light();
  //client.publish(topic_rgb_out, "ON");
  //client.publish(topic_item_out, "ON");
}

if ((animate_on == true) && (fade_on == true))
{
  //scene = 1;
  //autobright = 100;
  //sat = 0;
  //client.publish(topic_rgb_out, "ON");
  //client.publish(topic_item_out, "ON");
  animate();
}

if ((auto_on == true) && (fade_on == true))
{
  if (light_on == false)
  {
  mode = 1;
  sat = 0;
  light();
  }
  else
  {
    //Serial.println("auto, light is on, do nothing");
    mode = 1;
    light();
  }
}

}

void light()
{
  switch (mode) {
    case 0:
    client.publish(topic_rgb_out, "OFF");
    client.publish(topic_item_out, "OFF");
    Serial.println("case 0");
    fadeout();
    light_on = false;
    fade_on = false;
    auto_on = false; //check if is working
    button_on = false;
    FastLED.setBrightness(255);
    mode = -1;
    break;
    case 1:
    client.publish(topic_rgb_out, "ON");
    client.publish(topic_item_out, "ON");
    Serial.println("case 1");
    //fill_solid( leds, NUM_LEDS, CHSV(hue,sat,val));
    //FastLED.show();
    fade(autobright);
    light_on = true;
    fade_on = false;
    rgb_on = false;
    animate_on = false;
    mode = -1;
    //button_on = false;
    Serial.println("hue, sat, val");
    Serial.print(hue);
    Serial.print(sat);
    Serial.println(val);
    break;
    case 3:

    fade_on = false;
    //rgb = false;
    Serial.println("case 3");
    FastLED.setBrightness(255);

    if (light_on == false)
      {
        //client.publish(topic_rgb_out, "ON");
        client.publish(topic_item_out, "ON");
        Serial.println("fade on rgb");
      for (int i = 0; i <= 255; i++)
      {
        fill_solid( leds, NUM_LEDS, CHSV( hue, sat, i) );
        FastLED.show();
        Serial.println(i);
        delay(8);
      }

    } else
     {
       //client.publish(topic_rgb_out, "ON");
       client.publish(topic_item_out, "ON");
       Serial.print("just switch color rgb");
       fill_solid( leds, NUM_LEDS, CHSV( hue, sat, val) );
       FastLED.show();

    }



    light_on = true;
    break;
  }
}

void animate()
{
  switch (scene) {
    case 1:
    //client.publish(topic_rgb_out, "ON");
    //client.publish(topic_item_out, "ON");
    rainbowmarch();
    break;
    case 2:
    //client.publish(topic_rgb_out, "ON");
    //client.publish(topic_item_out, "ON");
    cylon();
    break;
    //fade_on = false;
    case 3:
    rainbowbeat();
    break;
    case 4:
    rainbowmarch();
    break;
  }
}

void fade(int autobright)
{

if (val < autobright) {

  client.publish(topic_rgb_out, "ON");
  for (int i = val; val < autobright; val++) {
  FastLED.setBrightness(val+1);
  //fill_solid( leds, NUM_LEDS, CRGB(255,255,255));
  fill_solid( leds, NUM_LEDS, CRGB(255,255,120));
  FastLED.show();
  Serial.println(val+1);
  delay(30);
  }

} else {
  Serial.println("less");
  //client.publish(topic_rgb_out, "OFF");
  for (int i = val; val > autobright; val--) {
  FastLED.setBrightness(val-1);
  //fill_solid( leds, NUM_LEDS, CRGB(255,255,255));
  fill_solid( leds, NUM_LEDS, CRGB(255,255,120));
  FastLED.show();
  Serial.println(val-1);
  delay(30);

    }
  }
   //hue = 0;
   //sat = 0;
}

void fadeout ()
{
      if (autolight_crgb == true) {

        for (uint8_t i=val; val > 0; val--){
        FastLED.setBrightness(val - 1);
        //fill_solid( leds, NUM_LEDS, CHSV(255,255,val));
        fill_solid( leds, NUM_LEDS, CRGB(255,255,255));
        FastLED.show();
        Serial.println(val);
        delay(30);
        }
        //brightness = 0;
        //FastLED.setBrightness( 0 );
        fill_solid( leds, NUM_LEDS, CRGB(255,255,0));
        FastLED.clear();
        FastLED.show();
        //Serial.println(brightness);
        Serial.println(val);
        Serial.println("off");
        autolight_crgb = false;
        Serial.println("autolight_crgb = false");
      }
      else {
        Serial.println("fadeout.else");
        Serial.println("hue, sat, val");
        Serial.print(hue);
        Serial.print(sat);
        Serial.println(val);
        //sat = 255;
        for (uint8_t i=val; val > 23; val--){
        //FastLED.setBrightness( brightness );
        //fill_solid( leds, NUM_LEDS, CHSV(255,255,val));
        fill_solid( leds, NUM_LEDS, CHSV(hue,sat,val));
        FastLED.show();
        Serial.println(val);
        delay(10);
        }
        //brightness = 0;
        //FastLED.setBrightness( 0 );
        fill_solid( leds, NUM_LEDS, CHSV(255,255,0));
        val = 0;
        FastLED.clear();
        FastLED.show();
        //Serial.println(brightness);
        Serial.println(val);
        Serial.println("off");
        //light_on = false;
      }
  //client.publish("/myhome/out/Kitchen_Light", "OFF");

}

void callback(const MQTT::Publish& pub) {
 Serial.println("z");
  if (pub.has_stream())
  {
    uint8_t buf[BUFFER_SIZE];
    int read;
    while (read == pub.payload_stream()->read(buf, BUFFER_SIZE))
    {
      Serial.write(buf, read);
    }
    pub.payload_stream()->stop();
    Serial.println("*");
  }
  else
    Serial.print(pub.topic());
    Serial.print(" ");
    Serial.println(pub.payload_string());

    if (pub.topic() == topic_rgb_in) {
        if ((pub.payload_string() != "ON") && (pub.payload_string() != "OFF"))
        {
        autolight_crgb = false;
        Serial.println("autolight_crgb = false");
        uint8_t y = 0;
        brightness = 50;
        FastLED.setBrightness( 255 );
        //scene = -1;
        //light_on = true;
        //animate_on = false;
        String values = pub.payload_string();
        Serial.print(values);
        int c1 = pub.payload_string().indexOf(',');
        int c2 = pub.payload_string().indexOf(',', c1 + 1);
        int lhue = pub.payload_string().toInt();
        int lsat = pub.payload_string().substring(c1 + 1).toInt();
        int lval = pub.payload_string().substring(c2 + 1).toInt();
        Serial.print(lhue);
        Serial.print(lsat);
        Serial.print(lval);
        hue = map(lhue, 0, 360, 0, 255);
        sat = map(lsat, 0, 100, 0, 255);
        val = map(lval, 0, 100, 0, 255);
        Serial.print("map y: ");
        Serial.println(y);
        //llval = (lval * 2.55);
        //llsat = (lsat * 2.55);
        //llhue = ((lhue / 3.6) * 2.55);
        Serial.print("округление: ");
        Serial.println(hue);
        Serial.println(sat);
        Serial.println(val);
        rgb = true;
        fade_on = true;
        animate_on = false;
        auto_on = false;
        //fill_solid( leds, NUM_LEDS, CHSV( hue, sat, val) );
        //FastLED.show();
        }
  }

  if (pub.topic() == topic_rgb_in) {
        if ((pub.payload_string() == "ON"))
        {
          //llval = 255;
          //brightness = 50;
          //FastLED.setBrightness( 255 );
          //light_on = false;
          //animate_on = false;
          Serial.println(pub.payload_string());
          //fill_solid( leds, NUM_LEDS, CHSV( llhue, llsat, 255) );
          //FastLED.show();
          //bright50();
          rgb_on = true;
          fade_on = true;
          autolight_crgb = true;
          Serial.println("autolight_crgb = true");
          //auto_on = false;
        }
    }

    if (pub.topic() == topic_rgb_in) {
          if ((pub.payload_string() == "OFF"))
          {
            //llval = 255;
            //brightness = 50;
            //FastLED.setBrightness( 255 );
            //light_on = false;
            //animate_on = false;
            Serial.println(pub.payload_string());
            //fill_solid( leds, NUM_LEDS, CHSV( llhue, llsat, 255) );
            //FastLED.show();
            //bright50();
            rgb_on = false;
            fade_on = true;
            animate_on = false;
            rgb = false;
            button_on = false;
            auto_on = false;
            //auto_on = false;
          }
      }

      if (pub.topic() == topic_auto_level) {
        autobright = pub.payload_string().toInt();
        Serial.print("autobright: ");
        Serial.println(autobright);
        //bright_auto(autobright);
        autolight_crgb = true;
        Serial.println("autolight_crgb = true");
        auto_on = true;
        fade_on = true;

        }

        if (pub.topic() == topic_scene_in) {
          autolight_crgb = false;
          Serial.println("autolight_crgb = false");
          Serial.println("hue, sat, val");
          Serial.print(hue);
          Serial.print(sat);
          Serial.println(val);
        if (pub.payload_string() == "rainbow")
        {
          //llval = 255;
          //brightness = 50;
          FastLED.setBrightness( 255 );
          val = 255;
          scene = 1;
          animate_on = true;
          light_on = true;
          rgb = false;
          fade_on = true;
          auto_on = false;
          Serial.print(scene);
          client.publish(topic_rgb_out, "ON");
          client.publish(topic_item_out, "ON");
        }
        }

        if (pub.topic() == topic_scene_in) {
          if (pub.payload_string() == "cylon")
          {

            FastLED.setBrightness( 255 );
            val = 255;
            scene = 2;
            animate_on = true;
            light_on = true;
            rgb = false;
            fade_on = true;
            auto_on = false;
            client.publish(topic_rgb_out, "ON");
            client.publish(topic_item_out, "ON");
          }
        }

}

void rainbowmarch()
{
  //rainbow_march(200, 10);
  rainbow_march();
  FastLED.show();
}

void rainbow_march() {     // The fill_rainbow call doesn't support brightness levels.

  uint8_t thishue = millis()*(255-200)/255;             // To change the rate, add a beat or something to the result. 'thisdelay' must be a fixed value.

// thishue = beat8(50);                                       // This uses a FastLED sawtooth generator. Again, the '50' should not change on the fly.
// thishue = beatsin8(50,0,255);                              // This can change speeds on the fly. You can also add these to each other.

  fill_rainbow(leds, NUM_LEDS, thishue, 10);            // Use FastLED's fill_rainbow routine.

}

void rainbowbeat()
{
  rainbow_beat();
  FastLED.show();
}

void rainbow_beat() {

  uint8_t beatA = beatsin8(17, 0, 255);                        // Starting hue
  uint8_t beatB = beatsin8(13, 0, 255);
  fill_rainbow(leds, NUM_LEDS, (beatA+beatB)/2, 8);            // Use FastLED's fill_rainbow routine.

}

void rainbow()
{
  //Serial.print("fill_rainbow");
  fill_rainbow( leds, NUM_LEDS, hue, 255 / NUM_LEDS);
  //Serial.println(hue);
  hue++;
  FastLED.show();
  delay(10);
  }

  void cylon()
  {
    //Serial.println("hue, sat, val");
    //Serial.print(hue);
    //Serial.print(sat);
    //Serial.println(val);
    sat = 255;
    for(int i = 0; i < NUM_LEDS; i++) {
  		// Set the i'th led to red
  		leds[i] = CHSV(hue++, 255, 255);
  		// Show the leds
  		FastLED.show();
  		// now that we've shown the leds, reset the i'th led to black
  		// leds[i] = CRGB::Black;
  		for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); }
  		// Wait a little bit before we loop around and do it again
  		delay(20);
  	}
  	//Serial.print("x");

  	// Now go in the other direction.
  	for(int i = (NUM_LEDS)-1; i >= 0; i--) {
  		// Set the i'th led to red
  		leds[i] = CHSV(hue++, 255, 255);
  		// Show the leds
  		FastLED.show();
  		// now that we've shown the leds, reset the i'th led to black
  		// leds[i] = CRGB::Black;
  		for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); }
  		// Wait a little bit before we loop around and do it again
  		delay(20);
  	}
  }

void setupOTA()
{
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}
