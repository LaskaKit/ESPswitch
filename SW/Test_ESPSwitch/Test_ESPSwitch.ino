/* Test code for LaskaKit ESPswitch
 * 
 * Board:   LaskaKit ESPswitch (ESP32C3 Dev Module) https://www.laskakit.cz/laskakit-esp32-devkit/
 * 
 * Library: by Adafruit         https://github.com/adafruit/Adafruit_NeoPixel
 *          by Miles Burton     https://github.com/milesburton/Arduino-Temperature-Control-Library
 *          by Paul Stoffregen  https://github.com/PaulStoffregen/OneWire
 *
 * Email:podpora@laskakit.cz
 * Web:laskakit.cz
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

// pro verze do v1.2
#define DS18B20_PIN 3     // DS18B20 GPIO on Laskakit ESPswitch board
#define LED_PIN     8     // LED GPIO on Laskakit ESPswitch board
#define CH0_PIN     0     // Channel 0 GPIO on Laskakit ESPswitch board (not used in this example)
#define CH1_PIN     1     // Channel 1 GPIO on Laskakit ESPswitch board (not used in this example)
#define CH2_PIN     4     // Channel 2 GPIO on Laskakit ESPswitch board (not used in this example)
#define CH3_PIN     5     // Channel 3 GPIO on Laskakit ESPswitch board

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);


OneWire oneWire(DS18B20_PIN);         // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature dallas(&oneWire);   // Pass our oneWire reference to Dallas Temperature sensor 

void setup() {
  Serial.begin(115200);

  ledcAttach(CH0_PIN, 200, 8);
  ledcAttach(CH1_PIN, 200, 8);
  ledcAttach(CH2_PIN, 200, 8);
  ledcAttach(CH3_PIN, 200, 8);

  dallas.begin();  // Start the DS18B20 sensor

  pixels.begin();
  pixels.setBrightness(10);
}

void loop() {

  dallas.requestTemperatures(); 
  float temperatureC = dallas.getTempCByIndex(0);
  if (temperatureC == -127) {
    Serial.println("No sensor connected! Connect DS18B20 or check wiring");
  } else {
    Serial.print(temperatureC);
    Serial.println("ºC");
  }  

// Test Channel 0, RED LED when active
  Serial.println("Test channel 0");
  pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // R, G, B
  pixels.show();


  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    // changing the motor rpm with with PWM
    ledcWrite(CH0_PIN, dutyCycle);
    delay(15);
  }
  delay(1000);

  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    // changing the LED brightness with PWM
    ledcWrite(CH0_PIN, dutyCycle);
    delay(15);
  }
  delay(500);

// Test Channel 1, GREEN LED when active
  Serial.println("Test channel 1");
  pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // R, G, B
  pixels.show();


  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    // changing the motor rpm with with PWM
    ledcWrite(CH1_PIN, dutyCycle);
    delay(15);
  }
  delay(1000);

  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    // changing the LED brightness with PWM
    ledcWrite(CH1_PIN, dutyCycle);
    delay(15);
  }
  delay(500);

// Test Channel 2, BLUE LED when active
  Serial.println("Test channel 2");
  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // R, G, B
  pixels.show();


  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    // changing the motor rpm with with PWM
    ledcWrite(CH2_PIN, dutyCycle);
    delay(15);
  }
  delay(1000);

  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    // changing the LED brightness with PWM
    ledcWrite(CH2_PIN, dutyCycle);
    delay(15);
  }
  delay(500);

// Test Channel 3, VIOLET LED when active
  Serial.println("Test channel 3");
  pixels.setPixelColor(0, pixels.Color(255, 0, 255)); // R, G, B
  pixels.show();


  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    // changing the motor rpm with with PWM
    ledcWrite(CH3_PIN, dutyCycle);
    delay(15);
  }
  delay(1000);

  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    // changing the LED brightness with PWM
    ledcWrite(CH3_PIN, dutyCycle);
    delay(15);
  }
  delay(500);
}
