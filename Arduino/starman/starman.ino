#define ARDUINOJSON_USE_LONG_LONG 1
//#define ARDUINOJSON_USE_INT64 1
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
WiFiClient client;
int curBuffer = 0;
int maxBuffer = 100;

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
    if (client.connect(server, 5000))
    {
      Serial.println("connected to server");
      // Make a HTTP request:
      client.println("GET /Query HTTP/1.0");
      client.println("Host: TreeTopService");
      client.println("Connection: close");
      if (client.println() == 0) {
        Serial.println(F("Failed to send request"));
        delay(5000);
        return;
      }

      char status[32] = {0};
      client.readBytesUntil('\r', status, sizeof(status));
      if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
        delay(5000);
        return;
      }

      // Skip HTTP headers
      char endOfHeaders[] = "\r\n\r\n";
      if (!client.find(endOfHeaders)) {
        Serial.println(F("Invalid response"));
        delay(5000);
        return;
      }

      // Use arduinojson.org/v6/assistant to compute the capacity.
      DynamicJsonDocument doc(5000);
      DeserializationError error = deserializeJson(doc, client);
      // Test if parsing succeeds.
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        delay(5000);
        //cleanup
        return;
      }

      long standByMillis = doc["standByMillis"];
      int track = doc["track"];
      JsonArray steps = doc["steps"].as<JsonArray>();
      Serial.println(F("Response:"));
      Serial.println(standByMillis);
      Serial.println(track);

      if (track >= 0)
      {
        dance(track, steps);
      }

      if (standByMillis > 0)
      {
#ifdef DEBUG
        standByMillis = 500;
#endif
        delay(standByMillis);
      }
    }
  }
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

void dance(int track, JsonArray steps) {
  playing = 1;
  if (track > 0) {
    myDFPlayer.play(track);
  }
  for (JsonVariant stp : steps) {
    //Serial.println(stp.as<signed __int64>());
    long long x = stp.as<signed long long>();

    bool all = x < 0;
    x = all ? x * -1 : x;
    int p = (x / 1000000000000000);
    int r = (x / 1000000000000) - p * 1000;
    int g = (x / 1000000000)    - p * 1000000          - r * 1000;
    int b = (x / 1000000)       - p * 1000000000       - r * 1000000       - g * 1000;
    int w = (x / 10)            - p * 100000000000000  - r * 100000000000  - g * 100000000    - b * 100000;
    int s =  x                  - p * 1000000000000000 - r * 1000000000000 - g * 1000000000   - b * 1000000 - w * 10;
    Serial.print("p:");
    Serial.println(p);
    Serial.print("r:");
    Serial.println(r);
    Serial.print("g:");
    Serial.println(g);
    Serial.print("b:");
    Serial.println(b);
    Serial.print("w:");
    Serial.println(w);
    Serial.print("s:");
    Serial.println(s);
    uint32_t color = strip.Color(r, g, b);
    if (all == 1) {
      Serial.print("ALL");
      for (int c = 0; c < LED_COUNT; c++) {
        strip.setPixelColor(c, color);
      }
    }
    else {
      Serial.print("SINGLE");
      strip.setPixelColor(p, color);
    }

    if (s == 1) {
      Serial.print("SHOW");
      strip.show();
    }
    if (w > 0) {
      Serial.print("WAIT:");
      Serial.println(w);
      delay(w);
    }
  }
  dormant();
  playing = 0;
}
