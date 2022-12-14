#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <WiFiClientSecure.h>
#include <GxEPD2_BW.h> // including both doesn't use more code or ram
#include <GxEPD2_3C.h> // including both doesn't use more code or ram
#include <U8g2_for_Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <MQTTClient.h>
#include <ArduinoJson.h>
#include "Secrets.h"
#include "WiFi.h"
#include "images.h"

#define EPD_DC      7
#define EPD_CS      8
#define EPD_BUSY    -1 //5
#define SRAM_CS     -1 //8
#define EPD_RESET   6

//ThinkInk_290_Grayscale4_T5 display(EPD_DC, EPD_RESET, EPD_CS, SRAM_CS, EPD_BUSY);

#define ENABLE_GxEPD2_GFX 0
#define VREF_PIN         4

#define NEOPIX_POWER_PIN GPIO_NUM_21
#define NEOPIX_PIN       GPIO_NUM_1
#define PIXEL_COUNT       4        // Number of NeoPixels on MagTag
#define PIXEL_BRIGHTNESS 16
// Used for software SPI
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
// Used for hardware & software SPI
#define LIS3DH_CS 10

//partial update works in some rotations (0 and 2)
//if you have another microcontroller or another e-ink display module you have to change the following lines
GxEPD2_BW<GxEPD2_290_T5, GxEPD2_290_T5::HEIGHT> display(GxEPD2_290_T5(EPD_CS, EPD_DC, EPD_RESET, -1));

Adafruit_NeoPixel strip( PIXEL_COUNT, NEOPIX_PIN, NEO_GRB + NEO_KHZ800 );

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;
unsigned long counter = 1;
static bool lightOn = false;
static bool imageOn = false;
static char *str;

// I2C
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

// The higher the value the less sensitive the detection.
// This changes based on the accelerometer range. Good values are
// For 16G, try 5-10 for 8G, try 10-20. for 4G try 20-40. for 2G try 40-80
#define CLICKTHRESHHOLD 40

// AWS pubSub
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
/*
The maximum size for packets being published and received is set by default to 128 bytes.
To change the buffer sizes, you need to use MQTTClient client(256)
instead of just MQTTClient client on the top of your sketch.
The passed value denotes the read and write buffer size.
*/
MQTTClient client = MQTTClient(256);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  } 

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);
  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
  // Create a message handler
  client.onMessage(messageHandler);
}

void publishMessage()
{
  StaticJsonDocument<200> doc;
  doc["title"] = "door_event";
  char jsonBuffer[128];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println(" Sent -> ");
  Serial.println(jsonBuffer);
  Serial.println(" to -> ");
  Serial.println(AWS_IOT_PUBLISH_TOPIC);
}

void setMessage(const char* str){
  display.setPartialWindow(0, 0, 298, 70); // x, y, width, height
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(str, 2, 2, &tbx, &tby, &tbw, &tbh);
  Serial.println(str);
  display.firstPage();
  do {
    u8g2Fonts.setCursor(0, 10); //cursor(x,y)

    u8g2Fonts.println(str);
  } while (display.nextPage());
}

void messageHandler(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["payload"][0];
  //Serial.println(message);
  setMessage(message);
}

void setup(void) {
  // Buttons
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_D, INPUT_PULLUP);
  // Led
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(NEOPIXEL_POWER, OUTPUT);
  digitalWrite(NEOPIX_POWER_PIN, LOW);
  strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
  strip.setBrightness( PIXEL_BRIGHTNESS );
  strip.show();  // Initialize all pixels to 'on'
  // Display
  display.init();
  display.setTextColor(GxEPD_BLACK);
  display.firstPage();
  display.setRotation(3);
  // Fonts
  u8g2Fonts.begin(display); // connect u8g2 procedures to Adafruit GFX
  delay(1000);

  uint16_t bg = GxEPD_WHITE;
  uint16_t fg = GxEPD_BLACK;
  u8g2Fonts.setForegroundColor(fg);  // apply Adafruit GFX color
  u8g2Fonts.setBackgroundColor(bg);

  do {
    display.fillScreen(GxEPD_WHITE);
    u8g2Fonts.setFont(u8g2_font_t0_17b_te);  //working fonts u8g2_font_silmple1_te, u8g2_font_fub11_tr, u8g2_font_t0_13_te
    u8g2Fonts.setCursor(0, 100);             //cursor(x,y)
    u8g2Fonts.print("Test.......");
  } while (display.nextPage());

  client.onMessage(messageHandler);
  connectAWS();
  
  if (! lis.begin(0x19)) {   // change this to 0x18 for alternative i2c address
    Serial.println("Couldn't start");
    while (1) yield();
  }
  
  lis.setRange(LIS3DH_RANGE_2_G);   // 2, 4, 8 or 16 G!

  // 0 = turn off click detection & interrupt
  // 1 = single click only interrupt output
  // 2 = double click only interrupt output, detect single click
  // Adjust threshhold, higher numbers are less sensitive
  lis.setClick(1, CLICKTHRESHHOLD);
  delay(100);
}

void setCounter(){
  u8g2Fonts.setCursor(10, 100);
  u8g2Fonts.print(counter++);
}

void setNeoPixelsColor( uint32_t color ) {
  for( int i=0; i < strip.numPixels(); i++ ) {
    strip.setPixelColor(i, color);          //  Set pixel's color (in RAM)
  }
  strip.show();
}

void ledLight(){
if(!lightOn){
  setNeoPixelsColor( strip.Color(0, 0, 255) );
  lightOn = true;
}
else{
  digitalWrite( LED_BUILTIN, HIGH );
  strip.clear();
  strip.show();
  lightOn = false;
  }
}

void loop() {
  uint8_t click = lis.getClick();
  display.setPartialWindow(0, 0, 150, 100); // x, y, width, height
  display.firstPage();
  if (click == 0) return;
  if (! (click & 0x30)) return;
  Serial.print("Movement detected (0x"); Serial.print(click, HEX); Serial.print("): \n");
  if (click & 0x10) {
    if (!client.connected()) { connectAWS(); }
    setCounter();
    publishMessage();
    ledLight();
    delay(4000);
    ledLight();
  }
  while (display.nextPage());
  client.loop();
  delay(10);
  return;
}
