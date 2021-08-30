#include <Arduino.h>
#include <FastLED.h>
#include <FlipskyPPM.h>

#define lowPWM 92 //set your low measurement
#define highPWM 875 //set your high measurement
#define zeroPoint 490 // set your zeropoint
#define deviation 5 //5% deviation of zero point
#define repetitionDetectionAfterMS 600 //change mode when double clicked in timeframe

#define LED_COUNT 5 //number of leds on each side

#define LEFT_LED_PIN 7
#define RIGHT_LED_PIN 5

CRGB leftLEDs[LED_COUNT];
CRGB rightLEDs[LED_COUNT];

unsigned char BRIGHTNESS = 255;
unsigned char SATURATION = 255;

int previousState = 0;
unsigned long lastSwitched = 0;

enum MODES {off = 0, solidRainbow, waveRainbow, circleRainbow, circleRainbowSparkle, speedControl, solidBloodOrange, solidWhite, NUMBER_OF_ITEMS};
void solidRainbowMode(int currentInputValue);
void waveRainbowMode(int currentInputValue);
void circleRainbowMode(int currentInputValue);
void circleRainbowSparkleMode(int currentInputValue);
void speedControlMode(int currentAccel);

//UTILS
uint8_t hueUtil(int currentInputValue);

class Mode {
private:
  MODES currentMode;
public:
  Mode();
  void changeMode();
  MODES getMode();
};

Mode::Mode(){
  this->currentMode = (MODES) 0;
}

void Mode::changeMode(){
  if(this->currentMode < NUMBER_OF_ITEMS - 1){
    this->currentMode = (MODES) ((int) this->currentMode + 1);
  }else{
    this->currentMode = (MODES) 0;
  }
}

MODES Mode::getMode(){
  return this->currentMode;
}

void checkForModeChange(int currentInputValue);

// Initialize a reciever on digital pin 3
FlipskyPPM reciever(3);

void setup() {
  Serial.begin(115200);
  Serial.println("Beginning Setup ..");
  //pinMode(A0, INPUT_PULLUP);
  FastLED.addLeds<NEOPIXEL, LEFT_LED_PIN>(leftLEDs, LED_COUNT).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<NEOPIXEL, RIGHT_LED_PIN>(rightLEDs, LED_COUNT).setCorrection(TypicalLEDStrip);
  Serial.println("Setup complete.");
  for(int i = 0; i < LED_COUNT; i++){
    leftLEDs[i].setRGB(0, 255, 10);
    rightLEDs[i].setRGB(0, 255, 10);
  }
  FastLED.show();
  delay(500);
  for(int i = 0; i < LED_COUNT; i++){
    leftLEDs[i].setRGB(0,0,0);
    rightLEDs[i].setRGB(0,0,0);
  }
  FastLED.show();
  delay(500);
  for(int i = 0; i < LED_COUNT; i++){
    leftLEDs[i].setRGB(0, 255, 10);
    rightLEDs[i].setRGB(0, 255, 10);
  }
  FastLED.show();
  delay(500);
  for(int i = 0; i < LED_COUNT; i++){
    leftLEDs[i].setRGB(0,0,0);
    rightLEDs[i].setRGB(0,0,0);
  }
  FastLED.show();
}

Mode currentLEDMode = Mode();


void loop() {
  int currentInputValue = reciever.getNewestValue();
  //check if mode has been changed => change mode accordingly
  checkForModeChange(currentInputValue);
  //calculate new colors depending on mode
  switch(currentLEDMode.getMode()){
    case solidRainbow: 
      solidRainbowMode(currentInputValue);
    break;
    case waveRainbow: 
      waveRainbowMode(currentInputValue);
    break;
    case circleRainbow:
      circleRainbowMode(currentInputValue);
    break;
    case circleRainbowSparkle:
      circleRainbowSparkleMode(currentInputValue);
    break;
    case speedControl: 
      speedControlMode(currentInputValue);
    break;
    case solidBloodOrange: 
      for(int i = 0; i < LED_COUNT; i++){
        rightLEDs[i].setRGB(255, 17, 0);
        leftLEDs[i].setRGB(255, 17, 0);
      }
    break;
    case solidWhite:
      for(int i = 0; i < LED_COUNT; i++){
        rightLEDs[i] = CRGB::White;
        leftLEDs[i] = CRGB::White;
      }
    break;
    case off: 
      for(int i = 0; i < LED_COUNT; i++){
        rightLEDs[i] = CRGB::Black;
        leftLEDs[i] = CRGB::Black;
      }
    break;
    default: break;
  }

  //show fps
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  static unsigned long counter = 0;
  if(currentMillis - lastMillis >= 10000){
    Serial.print("FPS: ");
    Serial.println(counter / 10);
    lastMillis = currentMillis;
    counter = 0;
  }
  counter++;

  //display new colors
  FastLED.show();
}

void checkForModeChange(int currentInputValue){
  if(currentInputValue <= lowPWM + 0.01 * deviation * zeroPoint){
    if(previousState == 1){
      //currentState below threshold and previously high
      //=> switched from high to low
      unsigned long currentTime = millis();
      if(currentTime - lastSwitched < repetitionDetectionAfterMS){
        //has been switched in the last repetitionDetectionAfterMS => change mode
        Serial.println("Switching Mode");
        currentLEDMode.changeMode();
        Serial.println("Current Mode is: (solidRainbow = 0, waveRainbow, circleRainbow, circleRainbowSparkle, speedControl, solidBloodOrange, solidWhite, off)");
        Serial.println((int) currentLEDMode.getMode());
      }
      lastSwitched = currentTime;
      previousState = 0;
    }
  }else if(currentInputValue >= zeroPoint - 0.01 * deviation * zeroPoint && previousState == 0){
    previousState = 1;
  }
}

//MODES
void solidRainbowMode(int currentAccel){
  unsigned char rainbow_hue = hueUtil(currentAccel);

  for(int i = 0; i < LED_COUNT; i++){
    CRGB value = CHSV(rainbow_hue, SATURATION, BRIGHTNESS);
    leftLEDs[i] = value;
    rightLEDs[i] = value;
  }
}

void waveRainbowMode(int currentAccel){
  unsigned char rainbow_hue = hueUtil(currentAccel);
  unsigned char incrementor = 255 / LED_COUNT;

  for(int i = 0; i < LED_COUNT; i++){
    CRGB value = CHSV(((int) rainbow_hue + (incrementor * i)) % 255, SATURATION, BRIGHTNESS);
    rightLEDs[i] = value;
    leftLEDs[i] = value;
  }
}

void circleRainbowMode(int currentAccel){
  unsigned char rainbow_hue = hueUtil(currentAccel);
  unsigned char incrementor = 255 / (LED_COUNT << 1); //fast multiply by 2

  for(int i = 0; i < LED_COUNT; i++){
    rightLEDs[i] = CHSV(((int) rainbow_hue + (incrementor * i)) % 255, SATURATION, BRIGHTNESS);
    leftLEDs[LED_COUNT - i - 1] = CHSV(((int) rainbow_hue + (incrementor * (i + LED_COUNT))) % 255, SATURATION, BRIGHTNESS);
  }
}

void circleRainbowSparkleMode(int currentInputValue){
  unsigned char rainbow_hue = hueUtil(currentInputValue);
  unsigned char incrementor = 255 / (LED_COUNT << 1); //fast multiply by 2
  const unsigned char glitterAmount = LED_COUNT * 2 / 5;
  const unsigned char glitterTime = 50;
  static int glitterArray[glitterAmount];
  static unsigned long lastGlitterChange = 0;
  unsigned long currentTime = millis();

  if(currentTime - lastGlitterChange >= glitterTime){
    for(int i = 0; i < glitterAmount; i++){
      glitterArray[i] = random16(LED_COUNT * 2);
      if(random16(100) > 50){
        glitterArray[i] = -1;
      }
    }
    lastGlitterChange = currentTime;
  }

  for(int i = 0; i < LED_COUNT; i++){
    rightLEDs[i] = CHSV(((int) rainbow_hue + (incrementor * i)) % 255, SATURATION, BRIGHTNESS);
    leftLEDs[LED_COUNT - i - 1] = CHSV(((int) rainbow_hue + (incrementor * (i + LED_COUNT))) % 255, SATURATION, BRIGHTNESS);
  }
  for(int i = 0; i < glitterAmount; i++) {
    if(glitterArray[i] >= 0){
      if(glitterArray[i] <= LED_COUNT - 1){
        leftLEDs[glitterArray[i]] += CRGB::White;
      }else{
        rightLEDs[glitterArray[i] - LED_COUNT] += CRGB::White;
      }
    }
  }
}

void speedControlMode(int currentAccel){
  static unsigned long lastDotChange = 0;
  static unsigned char dotPosition = 0;
  unsigned long currentTime = millis();
  static int lastAccels[100];
  unsigned long averageAccel = 0;
  for(int i = 0; i < 100 - 1; i++){
    lastAccels[i] = lastAccels[i + 1];
    averageAccel += lastAccels[i];
  }
  lastAccels[100 - 1] = currentAccel;
  averageAccel += currentAccel;
  averageAccel = averageAccel / 100;

  int dotspeed = 0;
  if(currentAccel < zeroPoint){
    dotspeed = map(currentAccel, lowPWM, zeroPoint, 200, 500);
  }else{
    dotspeed = map(currentAccel, zeroPoint, highPWM, 500, 150);
  } 

  if(averageAccel <= zeroPoint - (int) (0.02 * deviation * zeroPoint)){
    //slowing down => show brake lights
    if(currentTime - lastDotChange >= (unsigned long) dotspeed){
      if(dotPosition <= (LED_COUNT * 0.2) + 1){
        dotPosition = LED_COUNT - 1;
      }else{
        dotPosition--;
      }
      lastDotChange = currentTime;
    }
    for(int i = 0; i < LED_COUNT; i++){
      if(i <= (LED_COUNT * 0.2)){
        rightLEDs[i] = CRGB::Red;
        leftLEDs[i] = CRGB::Red;
      }else if(i == dotPosition){
        rightLEDs[dotPosition] = CRGB::White;
        leftLEDs[dotPosition] = CRGB::White;
      }else{
        rightLEDs[i] = CRGB::Black;
        leftLEDs[i] = CRGB::Black;
      }
    }
  }else if(averageAccel >= zeroPoint + (int) (0.02 * deviation * zeroPoint)){
    //accelerating => show dots
    if(currentTime - lastDotChange >= (unsigned long) dotspeed){
      if(dotPosition >= LED_COUNT - 1){
        dotPosition = 0;
      }else{
        dotPosition++;
      }
      lastDotChange = currentTime;
    }
    for(int i = 0; i < LED_COUNT; i++){
      if(i == dotPosition){
        rightLEDs[dotPosition] = CRGB::White;
        leftLEDs[dotPosition] = CRGB::White;
      }else{
        rightLEDs[i] = CRGB::Black;
        leftLEDs[i] = CRGB::Black;
      }
    }
  }else{
    for(int i = 0; i < LED_COUNT; i++){
      if(i == dotPosition){
        rightLEDs[i] = CRGB::White;
        leftLEDs[i] = CRGB::White;
      }else{
        rightLEDs[i] = CRGB::Black;
        leftLEDs[i] = CRGB::Black;
      }
    }
  }
}

//UTILS
uint8_t hueUtil(int currentAccel){
  static uint8_t rainbow_hue = 0;
  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  static int lastAccels[100];
  unsigned long averageAccel = 0;
  for(int i = 0; i < 100 - 1; i++){
    lastAccels[i] = lastAccels[i + 1];
    averageAccel += lastAccels[i];
  }
  lastAccels[100 - 1] = currentAccel;
  averageAccel += currentAccel;
  averageAccel = averageAccel / 100;

  int rainbowSpeed = 0;
  if(averageAccel < zeroPoint){
    rainbowSpeed = map(averageAccel, lowPWM, zeroPoint, 2, 50);
  }else{
    rainbowSpeed = map(averageAccel, zeroPoint, highPWM, 40, 2);
  }
  if(currentMillis - lastMillis >= (unsigned long) rainbowSpeed){
    if(averageAccel < zeroPoint){
      rainbow_hue++;
    }else{
      rainbow_hue--;
    }
    lastMillis = currentMillis;
  }

  return rainbow_hue;
}