#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <ArduinoJson.h>

#include <DFRobotDFPlayerMini.h>

// A basic everyday NeoPixel strip test program.

// NEOPIXEL BEST PRACTICES for most reliable operation:
// - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
// - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
// - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
// - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
//   connect GROUND (-) first, then +, then data.
// - When using a 3.3V microcontroller with a 5V-powered NeoPixel strip,
//   a LOGIC-LEVEL CONVERTER on the data line is STRONGLY RECOMMENDED.
// (Skipping these may work OK on your workbench but can fail in the field)
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LED_PIN 7
#define PIR_PIN 6
#define LED_COUNT 66

int pirStat = 0;
int playing = 0;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
IPAddress server(10, 0, 0, 113);
int status = WL_IDLE_STATUS;
WiFiSSLClient client;
// Use arduinojson.org/v6/assistant to compute the capacity.
StaticJsonDocument<38> doc;
int curBuffer = 0;
int maxBuffer = 100;
char *json = new char[100];

DFRobotDFPlayerMini myDFPlayer;
// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

// setup() function -- runs once at startup --------------------------------
//#define DEBUG
void setup()
{
  strip.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.clear();
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  blink(1000);
  blink(1000);
  blink(1000);
  Serial1.begin(9600);
  Serial.begin(9600);
  dormant();
#ifndef DEBUG
  while (!Serial1)
  {
    blink(2000);
  }
  if (!myDFPlayer.begin(Serial1))
  {
    while (true)
    {
      blink(250);
    }
  }
  myDFPlayer.volume(30);
#endif
  // check for the WiFi module:

  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true)
      ;
  }

  String fv = WiFi.firmwareVersion();

  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:

  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  Serial.println("Connected to wifi");
  printWiFiStatus();
  Serial.println("\nStarting connection to server...");
}
void loop()
{
  pirStat = digitalRead(PIR_PIN);
#ifndef DEBUG
  if (pirStat == HIGH && playing == 0) 
#else
  if (true)
#endif
  {
    // if you get a connection, report back via serial:
    if (client.connect(server, 443))
    {
      Serial.println("connected to server");
      // Make a HTTP request:
      client.println("GET /Query HTTP/1.1");
      client.println("Host: TreeTopService");
      client.println("Connection: close");
      client.println();

      curBuffer = 0;
      while (true)
      {
        while (client.available())
        {
          char c = client.read();
          json[curBuffer] = c;
          curBuffer++;
          Serial.write(c);
        }
        if (!client.connected())
        {
          Serial.println();
          Serial.println("disconnecting from server.");
          client.stop();
          break;
        }
      }
      json[curBuffer] = '\0';
      curBuffer++;
      DeserializationError error = deserializeJson(doc, json);

      // Test if parsing succeeds.
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        //cleanup
        return;
      }

      long standByMillis = doc["standByMillis"];
      int danceId = doc["danceId"];

      if (standByMillis == 0)
      {
        delay(standByMillis);
      }
      else if (danceId > 0)
      {
        dance(danceId);
        delay(5 * 60 * 1000); //wait 5 minutes before triggering again
      }
    }
  }
}

void dance(int danceId)
{
  playing = 1;
  switch (danceId)
  {
  case 1:
    starman();
    break;
  case 2:
    kart();
    break;
  default:
    break;
  }
  dormant();
  playing = 0;
}

void printWiFiStatus()
{

  // print the SSID of the network you're attached to:

  Serial.print("SSID: ");

  Serial.println(WiFi.SSID());

  // print your board's IP address:

  IPAddress ip = WiFi.localIP();

  Serial.print("IP Address: ");

  Serial.println(ip);

  // print the received signal strength:

  long rssi = WiFi.RSSI();

  Serial.print("signal strength (RSSI):");

  Serial.print(rssi);

  Serial.println(" dBm");
}

void blink(int delayms)
{
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delayms);
  digitalWrite(LED_BUILTIN, LOW);
  delay(delayms);
}

void dormant()
{
  //uint32_t color = strip.Color(255,160,180);// #ffa044
  uint32_t color = strip.Color(228, 92, 16); // #e45c10
  for (int i = 0; i < LED_COUNT; i++)
  {
    strip.setPixelColor(i, color);
    strip.show();
  }
}

void starman()
{
  /*
    uint32_t colors[]{
    strip.Color(255,160,180),// #ffa044
    strip.Color(228,92,16),// #e45c10
    strip.Color(248,56,0),// #f83800
    strip.Color(140,16,0)//#8c1000
    strip.Color(240,208,176),// #f0d0b0
    strip.Color(172,124,0),// #ac7c00
    strip.Color(254,139,54),// #fe8b36
    strip.Color(255,224,168),// #ffe0a8
    strip.Color(240,208,176)// #f0d0b0

    strip.Color(136,112,0),//887000
    strip.Color(216,40,0),//d82800

    strip.Color(200,76,12),//c84c0c
    strip.Color(0,0,0),//000000

    strip.Color(216,40,0),//d82800
    strip.Color(252,216,168),//fcd8a8

    strip.Color(252,152,56),//fc9838
    strip.Color(0,168,0),//00a800--53-59

    strip.Color(252,152,56),//fc9838
    strip.Color(216,40,0),//d82800
    };
  */
  myDFPlayer.play(1);
  uint32_t colors[]{
      //  strip.Color(228, 92, 16), // #e45c10
      //  strip.Color(248, 56, 0),  // #f83800
      //  strip.Color(140, 16, 0)   //#8c1000

      strip.Color(136, 112, 0), //887000
      strip.Color(216, 40, 0),  //d82800

      strip.Color(200, 76, 12), //c84c0c
      strip.Color(0, 0, 0),     //000000

      strip.Color(216, 40, 0),    //d82800
      strip.Color(252, 216, 168), //fcd8a8

      strip.Color(252, 152, 56), //fc9838
      strip.Color(0, 168, 0),    //00a800

      strip.Color(252, 152, 56), //fc9838
      strip.Color(216, 40, 0)    //d82800
  };
  for (int c = 0; c < 13; c++)
  {
    //int rand = random(0,8);
    for (int j = 0; j < 10; j++)
    {
      for (int i = 0; i < LED_COUNT; i++)
      {
        strip.setPixelColor(i, colors[j]);
      }
      strip.show();
      delay(50);
    }
  }
}

void kart()
{
  myDFPlayer.play(2);
  uint32_t colors[]{
      strip.Color(255, 0, 0),   //R
      strip.Color(237, 109, 0), //O
      strip.Color(0, 255, 255)  //G
  };
  for (int c = 0; c < 3; c++)
  {
    for (int i = 0; i < LED_COUNT; i++)
    {
      strip.setPixelColor(i, colors[c]);
    }
    strip.show();
    delay(1000);
  }
  delay(2000);
}
