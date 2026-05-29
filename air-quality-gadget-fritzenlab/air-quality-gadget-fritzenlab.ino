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
unsigned long blinkTimer = 0;
uint32_t smoothtvoc = 0;
int blink = 0;
int blinkLED = 0;

class MovingAverage {
  private:
    int _numReadings;
    uint32_t *_readings;     
    int _readIndex = 0;
    uint32_t _total = 0;   

  public:
    MovingAverage(int size) {
      _numReadings = size;
      _readings = new uint32_t[_numReadings];
      for (int i = 0; i < _numReadings; i++) _readings[i] = 0.0;
    }

    ~MovingAverage() {  // free memory
      delete[] _readings;
    }

    uint32_t update(uint32_t newValue) {
      _total -= _readings[_readIndex];
      _readings[_readIndex] = newValue;
      _total += newValue;

      _readIndex++;
      if (_readIndex >= _numReadings) _readIndex = 0;

      return _total / _numReadings; 
    }
};

MovingAverage tvocSensorAvg(25);

uint32_t readAGS10()
{
  uint8_t data[5] = {0};

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
  /*
    Log-like perception mapping:
    
    GREEN   :   0 - 500
    YELLOW  : 500 - 1200
    ORANGE  : 1200 - 2500
    RED     : >2500

    The steps are intentionally compressed at higher values
    so the display behaves more naturally to human perception.
  */

  // ===== DEEP GREEN TO LIME =====
  if(tvoc < 80)           setColor(0, 18, 0);
  else if(tvoc < 160)     setColor(0, 35, 0);
  else if(tvoc < 260)     setColor(0, 55, 0);
  else if(tvoc < 380)     setColor(15, 75, 0);
  else if(tvoc < 500)     setColor(40, 100, 0);

  // ===== YELLOW ZONE =====
  else if(tvoc < 700)     setColor(90, 110, 0);
  else if(tvoc < 900)     setColor(140, 125, 0);
  else if(tvoc < 1050)    setColor(190, 140, 0);
  else if(tvoc < 1200)    setColor(240, 170, 0);

  // ===== ORANGE ZONE =====
  else if(tvoc < 1500)    setColor(255, 130, 0);
  else if(tvoc < 1800)    setColor(255, 100, 0);
  else if(tvoc < 2100)    setColor(255, 75, 0);
  else if(tvoc < 2500)    setColor(255, 45, 0);

  // ===== RED ZONE =====
  else if(tvoc < 3000)    setColor(255, 0, 0);
  else if(tvoc < 4000)    setColor(180, 0, 0);
  else                    setColor(120, 0, 10);
}
void blink_LED(int color){
  if(millis() - blinkTimer > 300){
    blinkTimer += 300;

    if(blink == 0){
      blink= 1;
      if(color == 1){
        setColor(0, 0, 254);
      }else{
        setColor(254, 0, 0);
      }
    }else{
      blink= 0;
      setColor(0, 0, 0);
    }

  }
    
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

    tvoc = readAGS10(); // make an AGS10 reading

    if(tvoc < 1){ // maybe we lost connection to the sensor
      blinkLED= 1; // blinks in blue
    }else if(tvoc > 5500){ // very likely a bad situation
      blinkLED= 2; // blinks in red

    }else{ // this is the normal, just light the Neopixel with the corresponding color
      blinkLED= 0;
      smoothtvoc = tvocSensorAvg.update(tvoc);
      showTVOC(smoothtvoc);
    }
    
  }
  // effectively execute the blinking, if and when necessary
  if(blinkLED == 1){ 
    blink_LED(1);
  }else if(blinkLED == 2){
    blink_LED(2);
  }else{

  }
  
}