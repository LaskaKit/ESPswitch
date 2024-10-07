/* Test code for LaskaKit ESPswitch
 * LED legend:
 * - White: Not connected to WiFi
 * - Green: Connected to WiFi and ready
 * - Blue: Heater is off
 * - Red: Heater is on
 *
 * Board:   LaskaKit ESPswitch (ESP32C3 Dev Module) https://www.laskakit.cz/laskakit-esp32-devkit/
 *
 * Library: by Adafruit         https://github.com/adafruit/Adafruit_NeoPixel 
 *                              https://github.com/adafruit/Adafruit-GFX-Library
 *                              https://github.com/adafruit/Adafruit_SSD1306
 *          by Miles Burton     https://github.com/milesburton/Arduino-Temperature-Control-Library
 *          by Paul Stoffregen  https://github.com/PaulStoffregen/OneWire
 *          by tzapu            https://github.com/tzapu/WiFiManager
 *
 * Email:podpora@laskakit.cz
 * Web:laskakit.cz
 */

#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiManager.h>
#include "page.h"

#define HOSTNAME "cupheater"

#define DELAYTIME 1000  // Delay in ms between temperature readings and display updates
#define i2c_Address 0x3c

#define PIN_SCL 19      // Clock pin on LaskaKit ESPswitch board  
#define PIN_SDA 18      // Data pin on LaskaKit ESPswitch board
#define DS18B20_PIN 3   // DS18B20 GPIO on Laskakit ESPswitch board
#define LED_PIN 8       // LED GPIO on Laskakit ESPswitch board
#define CH0_PIN 0       // Channel 0 GPIO on Laskakit ESPswitch board (not used in this example)
#define CH1_PIN 1       // Channel 1 GPIO on Laskakit ESPswitch board (not used in this example)
#define CH2_PIN 4       // Channel 2 GPIO on Laskakit ESPswitch board (not used in this example)
#define CH3_PIN 5       // Channel 3 GPIO on Laskakit ESPswitch board

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(128, 32, &Wire, -1);

OneWire oneWire(DS18B20_PIN);       // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature dallas(&oneWire); // Pass our oneWire reference to Dallas Temperature sensor

WebServer server(80);

bool heater_state = false;
float temp_bottom = 55;
float temp_top = 65;
/* Offset to stop the heater before reaching the set temperature,
*  this is to prevent the heater from overshooting the set temperature
*/
#define TEMP_OFFSET_TOP 5.0

void DNS_setup()
{
  if (MDNS.begin(HOSTNAME))
  {
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS responder started");
    Serial.print("You can now connect to http://");
    Serial.print(HOSTNAME);
    Serial.println(".local");
  }
}

// float get_temp()
// {
//   dallas.requestTemperatures();
//   float tempC = dallas.getTempCByIndex(0);
//   if (tempC == DEVICE_DISCONNECTED_C)
//   {
//     Serial.println("Error: Could not read temperature data");
//   }
//   return tempC;
// }

float get_temp()
{
  static uint32_t lastTempCheck = 0;
  static float lastTemp = DEVICE_DISCONNECTED_C;
  
  uint32_t currentMillis = millis();
  
  if (currentMillis - lastTempCheck >= DELAYTIME)
  {
    lastTempCheck = currentMillis;
    dallas.requestTemperatures();
    lastTemp = dallas.getTempCByIndex(0);
    if (lastTemp == DEVICE_DISCONNECTED_C)
    {
      Serial.println("Error: Could not read temperature data");
    }
  }
  
  return lastTemp;
}

void regulate_heater(float temp_bottom, float temp_top)
{
  if (heater_state)
  {
    float temp = get_temp();
    if (temp > temp_top - TEMP_OFFSET_TOP)
    {
      ledcWrite(CH3_PIN, 0);
    }
    else if (temp < temp_bottom)
    {
      ledcWrite(CH3_PIN, 255);
    }
  }
  else
  {
    ledcWrite(CH3_PIN, 0);
  }
}

void control_led()
{
  if (ledcRead(CH3_PIN) > 0)
  {
    // Red color to indicate that the heater is on
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  }
  else
  {
    // Blue color to indicate that the heater is off
    pixels.setPixelColor(0, pixels.Color(0, 0, 255));
  }
  pixels.show();
}

void control_display()
{
  static uint32_t lastUpdate = 0;

  uint32_t currentMillis = millis();
  
  if (currentMillis - lastUpdate >= DELAYTIME) {
    lastUpdate = currentMillis;

    display.clearDisplay();

    display.setCursor(5, 5);
    display.setTextSize(3); 
    display.println(String(get_temp()) + "C");

    display.display();
  }
}

void handle_root()
{
  server.send_P(200, "text/html", index_html); // Send web page
}

void handle_not_found()
{
  String message = "Error\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handle_temp_plate()
{
  char buff[7] = {0};
  sprintf(buff, "%0.2f ˚C", get_temp());
  server.send(200, "text/plain", buff);
}

void handle_switch_on()
{
  heater_state = true;
  server.send(200, "text/plain", "Switched on");
}

void handle_switch_off()
{
  heater_state = false;
  server.send(200, "text/plain", "Switched off");
}

void handle_get_switch_status()
{
  char status[6] = {0};
  strcpy(status, heater_state ? "true" : "false");
  server.send(200, "text/plain", status);
}

void handle_set_temperature()
{
  if (server.hasArg("temp_bottom") && server.hasArg("temp_top"))
  {
    temp_bottom = server.arg("temp_bottom").toFloat();
    temp_top = server.arg("temp_top").toFloat();
    server.send(200, "text/plain", "Temperature set");
  }
  else
  {
    server.send(400, "text/plain", "Invalid request");
  }
}

void handle_get_set_values()
{
  char response[50];
  sprintf(response, "temp_bottom=%.1f&temp_top=%.1f", temp_bottom, temp_top);
  server.send(200, "text/plain", response);
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, i2c_Address)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay(); 
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setTextSize(1); 

  pixels.begin();
  pixels.setBrightness(10);

  // White color to indicate that the device is not connected to WiFi
  pixels.setPixelColor(0, pixels.Color(255, 255, 255));
  pixels.show();

  WiFiManager wm;
  bool res = wm.autoConnect(HOSTNAME);
  if (!res)
  {
    Serial.println("Failed to connect");
    ESP.restart();
  }
  else
  {
    Serial.println("Connected to WiFi!");
  }
  DNS_setup();

  // Green color to indicate that the device is connected to WiFi and ready
  pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  pixels.show();

  ledcAttach(CH3_PIN, 200, 8); // Attach channel 3 to the GPIO pin to control the heater

  dallas.begin(); // Start the DS18B20 sensor

  server.on("/", handle_root);
  server.onNotFound(handle_not_found);
  server.on("/handle_temp_plate", handle_temp_plate);
  server.on("/handle_switch_on", handle_switch_on);
  server.on("/handle_switch_off", handle_switch_off);
  server.on("/get_switch_status", handle_get_switch_status);
  server.on("/set_temperature", HTTP_POST, handle_set_temperature);
  server.on("/get_set_values", handle_get_set_values);
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();
  regulate_heater(temp_bottom, temp_top);
  control_led();
  control_display();

  Serial.print("Heater state: ");
  Serial.println(heater_state);
  Serial.print("Min temperature: ");
  Serial.println(temp_bottom);
  Serial.print("Max temperature: ");
  Serial.println(temp_top);
  Serial.print("PWM value: ");
  Serial.println(ledcRead(CH3_PIN));
}
