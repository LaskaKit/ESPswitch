/* Test code for LaskaKit ESPswitch
 * 
 * Board:   LaskaKit ESPswitch (ESP32C3 Dev Module) https://www.laskakit.cz/laskakit-esp32-devkit/
 * 
 * Library: by Adafruit         https://github.com/adafruit/Adafruit_NeoPixel
 *          by Miles Burton     https://github.com/milesburton/Arduino-Temperature-Control-Library
 *          by Paul Stoffregen  https://github.com/PaulStoffregen/OneWire
 *          by tzapu            https://github.com/tzapu/WiFiManager
 *
 * Email:podpora@laskakit.cz
 * Web:laskakit.cz
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiManager.h>
#include "page.h"

#define HOSTNAME "ESPswitch"
#define TEMP_BOTTOM 55
#define TEMP_TOP    65

#define DS18B20_PIN 3     // DS18B20 GPIO on Laskakit ESPswitch board
#define LED_PIN     8     // LED GPIO on Laskakit ESPswitch board
#define CH0_PIN     0     // Channel 0 GPIO on Laskakit ESPswitch board (not used in this example)
#define CH1_PIN     1     // Channel 1 GPIO on Laskakit ESPswitch board (not used in this example)
#define CH2_PIN     4     // Channel 2 GPIO on Laskakit ESPswitch board (not used in this example)
#define CH3_PIN     5     // Channel 3 GPIO on Laskakit ESPswitch board

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);

OneWire oneWire(DS18B20_PIN);         // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature dallas(&oneWire);   // Pass our oneWire reference to Dallas Temperature sensor 

WebServer server(80);

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

float get_temp()
{
  dallas.requestTemperatures(); 
  float tempC = dallas.getTempCByIndex(0);

  if (tempC != DEVICE_DISCONNECTED_C)
  {
    Serial.print("Temperature is: ");
    Serial.println(tempC);
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
  }
  return tempC;
}

void regulate_heater(float temp_bottom, float temp_top) {
  float temp = get_temp();
  if (temp < temp_bottom) {
    ledcWrite(CH3_PIN, 255);
  } else if (temp > temp_top) {
    ledcWrite(CH3_PIN, 0);
  }
}

void control_led() {
  if (ledcRead(CH3_PIN) > 0) {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
  } else {
    pixels.setPixelColor(0, pixels.Color(0, 255, 0));
  }
  pixels.show();
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

void setup() {
  Serial.begin(115200);

  pixels.begin();
  pixels.setBrightness(10);

  // White color to indicate that the device is not connected to WiFi
  pixels.setPixelColor(0, pixels.Color(255, 255, 255));
  pixels.show();

  WiFiManager wm;
  bool res = wm.autoConnect("ESPswitch");
  if (!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } else {
    Serial.println("Connected to WiFi!");
  }

  DNS_setup();

  ledcAttach(CH3_PIN, 200, 8); // Attach channel 3 to the GPIO pin to control the heater
  
  dallas.begin();  // Start the DS18B20 sensor

  server.on("/", handle_root);
	server.onNotFound(handle_not_found);
	server.on("/handle_temp_plate", handle_temp_plate);
	server.begin();
	Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  regulate_heater(TEMP_BOTTOM, TEMP_TOP);
  control_led();
}
