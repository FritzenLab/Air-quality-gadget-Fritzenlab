// Air quality monitor with AGS10 TVOC sensor, Attiny85 and WS2812b addressable LED
/*There are 20 different colors possible, representing AGS10 TVOC values between 
sixty five (65) or smaller and five thousand five hundred (5500) or larger
(five green, five yellow, five orange and five red colors)
- RGB LED on pin 1, SDA on pin 0 and SCL on pin 2

*/
#include <TinyWireM.h>
#define AGS10_ADDR 0x1A
#include <Adafruit_NeoPixel.h>

#define LED_PIN 1
#define NUMPIXELS 1

Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

uint32_t tvoc = 0;
unsigned long mainTimer = 0;

uint32_t readAGS10()
{
  uint8_t data[5];

  // Select register 0x00
  TinyWireM.beginTransmission(AGS10_ADDR);
  TinyWireM.write(0x00);
  TinyWireM.endTransmission();

  delay(5);

  // Request 5 bytes
  TinyWireM.requestFrom(AGS10_ADDR, 5);

  for (uint8_t i = 0; i < 5; i++)
  {
    if (TinyWireM.available())
    {
      data[i] = TinyWireM.read();
    }
  }

  // TVOC value is bytes 1,2,3
  uint32_t tvoc =
      ((uint32_t)data[1] << 16) |
      ((uint32_t)data[2] << 8)  |
      data[3];

  return tvoc;
}
void setColor(uint8_t r, uint8_t g, uint8_t b)
{
  pixel.setPixelColor(0, pixel.Color(r, g, b));
  pixel.show();
}

void showTVOC(uint32_t tvoc)
{
  // ===== GREEN =====
  if(tvoc < 65)          setColor(0, 20, 0);
  else if(tvoc < 130)    setColor(0, 40, 0);
  else if(tvoc < 195)    setColor(0, 60, 0);
  else if(tvoc < 260)    setColor(20, 80, 0);
  else if(tvoc < 325)    setColor(40, 100, 0);

  // ===== YELLOW =====
  else if(tvoc < 450)    setColor(80, 100, 0);
  else if(tvoc < 575)    setColor(120, 120, 0);
  else if(tvoc < 700)    setColor(150, 130, 0);
  else if(tvoc < 825)    setColor(180, 140, 0);
  else if(tvoc < 950)    setColor(220, 160, 0);

  // ===== ORANGE =====
  else if(tvoc < 1150)   setColor(255, 120, 0);
  else if(tvoc < 1350)   setColor(255, 100, 0);
  else if(tvoc < 1550)   setColor(255, 80, 0);
  else if(tvoc < 1750)   setColor(255, 60, 0);
  else if(tvoc < 1950)   setColor(255, 40, 0);

  // ===== RED =====
  else if(tvoc < 2300)   setColor(255, 0, 0);
  else if(tvoc < 2800)   setColor(220, 0, 0);
  else if(tvoc < 3500)   setColor(180, 0, 0);
  else if(tvoc < 4500)   setColor(120, 0, 0);
  else                   setColor(80, 0, 20);
}
void setup()
{
  TinyWireM.begin();
  pixel.begin();
  pixel.clear();
  pixel.show();
}

void loop()
{
  if(millis() - mainTimer > 5000){
    mainTimer += 5000;

    tvoc = readAGS10();
    showTVOC(tvoc);
  }
  

  
}