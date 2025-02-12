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
bool isRunning = false;
bool isConfig = true;
int currentMode = 0;

struct TrainingMode {
  int reps;
  int sets;
};

TrainingMode modes[] = {
  {8, 3},
  {10, 4},
  {12, 5},
  {15, 6}
};
const int totalModes = sizeof(modes) / sizeof(modes[0]);

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

  Serial.begin(9600);

  displayModePreview();
}

void loop() {
  if (isConfig) {
    handleModeSelection();
    handleButtons();
  } else {
    handleButtons();
    if (isRunning) {
      runTraining();
    }
  }
}

void handleModeSelection() {
  if (digitalRead(BUTTON_TASK_CHANGE) == LOW) {
    delay(200);
    currentMode = (currentMode + 1) % totalModes;
    displayModePreview();
  }
}

void displayModePreview() {
  counterStrip.clear();
  setsStrip.clear();
  
  for (int i = 0; i < modes[currentMode].reps; i++) {
    counterStrip.setPixelColor(i, counterStrip.Color(0, 255, 0));
  }
  
  for (int i = 0; i < modes[currentMode].sets; i++) {
    setsStrip.setPixelColor(i, setsStrip.Color(0, 0, 255));
  }
  
  counterStrip.show();
  setsStrip.show();
}

void handleButtons() {
  if (digitalRead(BUTTON_START) == LOW) {
    delay(400);
    if (isConfig){
      isConfig = false;
      isRunning = true;
      counterStrip.clear();
      setsStrip.clear();
      counterStrip.show();
      setsStrip.show();
      }
    if (completedSets >= modes[currentMode].sets) {
      delay(400);
      isConfig = true;
      completedSets = 0;
      completedCycles = 0;
      counterStrip.clear();
      setsStrip.clear();
      counterStrip.show();
      setsStrip.show();
      displayModePreview();
    } else {
      isRunning = true;
      completedCycles = 0;
      counterStrip.clear();
      counterStrip.show();
      displayActiveMarkers();
    }
  }
}


void displayActiveMarkers() {
  counterStrip.setPixelColor(modes[currentMode].reps - 1, counterStrip.Color(0, 255, 255)); // Кінцева мітка повторів
  setsStrip.setPixelColor(modes[currentMode].sets - 1, setsStrip.Color(255, 255, 0)); // Кінцева мітка підходів
  counterStrip.show();
  setsStrip.show();
}

void runTraining() {
  int fadeDelay = 50;

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

  if (completedCycles < modes[currentMode].reps) {
    counterStrip.setPixelColor(completedCycles, counterStrip.Color(0, 255, 0));
    counterStrip.show();
    completedCycles++;
  }

  if (completedCycles >= modes[currentMode].reps) {
    completedCycles = 0;
    completedSets++;
    setsStrip.setPixelColor(completedSets - 1, setsStrip.Color(0, 0, 255));
    setsStrip.show();
    isRunning = false;
  }
}
