#include <Adafruit_NeoPixel.h>

#define PIN1 5
#define PIN2 6
#define PIN3 7
#define BUTTON_START 2
#define BUTTON_TASK_CHANGE 8
#define BUTTON_NEXT_SET 9

int MAIN_STRIP_PIXELS = 16;
int COUNTER_STRIP_PIXELS = 16;
int SETS_STRIP_PIXELS = 16;

Adafruit_NeoPixel mainStrip = Adafruit_NeoPixel(MAIN_STRIP_PIXELS, PIN1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel counterStrip = Adafruit_NeoPixel(COUNTER_STRIP_PIXELS, PIN2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel setsStrip = Adafruit_NeoPixel(SETS_STRIP_PIXELS, PIN3, NEO_GRB + NEO_KHZ800);

int completedCycles = 0;
int completedSets = 0;
bool afterStart = false;

bool isRunning = false;
bool isConfig = true;

void setup() {
  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_TASK_CHANGE, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_SET, INPUT_PULLUP);

  mainStrip.begin();
  counterStrip.begin();
  setsStrip.begin();

  mainStrip.show();
  counterStrip.show();
  setsStrip.show();

  COUNTER_STRIP_PIXELS = 0;
  SETS_STRIP_PIXELS = 0;  
}

void loop() {
  configur();
  if (isRunning == false) handleButtons();

  if (isRunning) {
    int fadeDelay = 50;
    counterStrip.setPixelColor(COUNTER_STRIP_PIXELS-1, counterStrip.Color(0, 255, 255));
    counterStrip.show();
    setsStrip.setPixelColor(SETS_STRIP_PIXELS-1, setsStrip.Color(255, 255, 0));
    setsStrip.show();

    for (int i = 0; i < MAIN_STRIP_PIXELS; i++) {
      mainStrip.setPixelColor(i, mainStrip.Color(255, 0, 0));
      mainStrip.show();
      delay(fadeDelay);
    }

    for (int i = MAIN_STRIP_PIXELS - 1; i >= 0; i--) {
      mainStrip.setPixelColor(i, 0);
      mainStrip.show();
      delay(fadeDelay);
    }

    if (completedCycles < COUNTER_STRIP_PIXELS) {
      counterStrip.setPixelColor(completedCycles, counterStrip.Color(0, 255, 0));
      counterStrip.show();
      completedCycles++;
    }

    if (completedCycles >= COUNTER_STRIP_PIXELS) {
      isRunning = false;
    }
  }
}

void handleButtons() {
  if (digitalRead(BUTTON_START) == LOW) {
    if ((completedCycles >= COUNTER_STRIP_PIXELS || completedCycles == 0) && completedSets <= SETS_STRIP_PIXELS-1) {
      delay(5000);
      isRunning = !isRunning;
      completedSets++;
      updateSetsStrip();
      resetCycle();
    }
    else {
      resetCycle();
      completedSets = 0;
      isConfig = true;
      setsStrip.setPixelColor(SETS_STRIP_PIXELS-1, 0);
      COUNTER_STRIP_PIXELS = 0;
      SETS_STRIP_PIXELS = 0;
      afterStart = true;
      configur();
      }
  }
}

void resetCycle() {
  completedCycles = 0;
  for (int i = 0; i < COUNTER_STRIP_PIXELS; i++) {
    counterStrip.setPixelColor(i, 0);
  }
  counterStrip.show();
}

void updateSetsStrip() {
  setsStrip.clear();
  if (completedSets > 0 && completedSets <= SETS_STRIP_PIXELS) {
    setsStrip.setPixelColor(completedSets-1, setsStrip.Color(0, 0, 255)); // Кількість підходів
    setsStrip.show();
  }
  setsStrip.setPixelColor(SETS_STRIP_PIXELS - 1, setsStrip.Color(255, 255, 0));
  setsStrip.show();
}

void configur (){
  while (isConfig == true){
    if (afterStart == true) delay(200);
    afterStart = false;
    if (digitalRead(BUTTON_TASK_CHANGE) == LOW) {
      delay(200);
      if(COUNTER_STRIP_PIXELS <= setsStrip.numPixels()-1){
        COUNTER_STRIP_PIXELS++;
      }
      else{ 
      COUNTER_STRIP_PIXELS = 0;
      counterStrip.clear();
      }
      counterStrip.setPixelColor(COUNTER_STRIP_PIXELS-1, counterStrip.Color(0, 255, 0));
      counterStrip.show();
    }
    if (digitalRead(BUTTON_NEXT_SET) == LOW) {
      delay(200);
      if(SETS_STRIP_PIXELS <= setsStrip.numPixels()-1){
        SETS_STRIP_PIXELS++;
      }
      else{ 
      SETS_STRIP_PIXELS = 0;
      setsStrip.clear();
      }
      setsStrip.setPixelColor(SETS_STRIP_PIXELS-1, setsStrip.Color(0, 0, 255));
      setsStrip.show();
    }
    if (digitalRead(BUTTON_START) == LOW) {
    delay(200);
    isConfig = false;
    setsStrip.clear();
    counterStrip.clear();
    break;
  }
  }
}
