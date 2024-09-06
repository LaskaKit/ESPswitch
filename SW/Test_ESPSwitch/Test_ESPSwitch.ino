//  ESP32-C3
//  ESP-IDF  3.0.X
//  OneWire  2.0.8

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>

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

  ledcAttach(CH3_PIN, 200, 8);  // Start the DS18B20 sensor
  
  dallas.begin();  // Start the DS18B20 sensor

  pixels.begin();
  pixels.setBrightness(10);
}

void loop() {

  dallas.requestTemperatures(); 
  float temperatureC = dallas.getTempCByIndex(0);
  Serial.print(temperatureC);
  Serial.println("ºC");

  pixels.setPixelColor(0, pixels.Color(255, 0, 0)); // R, G, B
  pixels.show();
  delay(1000);

  pixels.setPixelColor(0, pixels.Color(0, 255, 0)); // R, G, B
  pixels.show();
  delay(1000);

  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // R, G, B
  pixels.show();
  delay(1000);

  pixels.setPixelColor(0, pixels.Color(255, 255, 255)); // R, G, B
  pixels.show();

  for(int dutyCycle = 0; dutyCycle <= 255; dutyCycle++){   
    // changing the LED brightness with PWM
    ledcWrite(CH3_PIN, dutyCycle);
    delay(15);
  }

  // decrease the LED brightness
  for(int dutyCycle = 255; dutyCycle >= 0; dutyCycle--){
    // changing the LED brightness with PWM
    ledcWrite(CH3_PIN, dutyCycle);
    delay(15);
  }
  
  delay(1000);
}
