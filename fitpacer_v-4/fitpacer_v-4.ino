#include <Adafruit_NeoPixel.h>
#define PIN1 5
#define PIN2 6
#define PIN3 7
#define BUZZER 11
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
bool isCooldown = false;
int currentMode = 0;
unsigned long previousMillis = 0;
unsigned long buttonPreviousMillis = 0;
unsigned long buttonDebounce = 400;
unsigned long cooldownStart = 0;
const unsigned long cooldownDuration = 5000;
int step = 0;

bool isValidInteger(String str) {
    if (str.length() == 0) return false;  // Empty string is invalid
    for (unsigned int i = 0; i < str.length(); i++) {
        if (!isDigit(str[i])) {  
            return false;  // If any character is not a digit, return false
        }
    }
    return true;
}

int reps = 0;
int sets = 0;
int pace = 0;

struct TrainingMode {
  int reps;
  int sets;
  unsigned long interval;
};

TrainingMode modes[] = {
  {8, 3, 50},
  {10, 4, 100},
  {12, 5, 200},
  {15, 6, 300},
  {1, 1, 1}
};
const int totalModes = sizeof(modes) / sizeof(modes[0]);

void setup() {
  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_TASK_CHANGE, INPUT_PULLUP);
  pinMode(BUTTON_NEXT_SET, INPUT_PULLUP);
  
  Serial.begin(9600);
  
  mainStrip.begin();
  counterStrip.begin();
  setsStrip.begin();

  mainStrip.show();
  counterStrip.show();
  setsStrip.show();

  displayModePreview();
}

void loop() {
  if (isConfig) {
    handleModeSelection();
    handleButtons();
  } else if (isCooldown) {
    runCooldown();
  } else {
    handleButtons();
    if (isRunning) {
      runTraining();
    }
  }
  handleBluetooth();
}

void handleModeSelection() {
  unsigned long currentMillis = millis();
  if (digitalRead(BUTTON_TASK_CHANGE) == LOW && currentMillis - buttonPreviousMillis > buttonDebounce) {
    buttonPreviousMillis = currentMillis;
    currentMode = (currentMode + 1) % totalModes;
    displayModePreview();
  }
}

void displayModePreview() {
  counterStrip.clear();
  setsStrip.clear();
  
  for (int i = 0; i < modes[currentMode].reps; i++) {
    counterStrip.setPixelColor(i, counterStrip.Color(100, 100, 0));
  }
  
  for (int i = 0; i < modes[currentMode].sets; i++) {
    setsStrip.setPixelColor(i, setsStrip.Color(100, 0, 100));
  }
  
  counterStrip.show();
  setsStrip.show();
}

void handleButtons() {
  unsigned long currentMillis = millis();
  if (digitalRead(BUTTON_START) == LOW && currentMillis - buttonPreviousMillis > buttonDebounce) {
    buttonPreviousMillis = currentMillis;
    if (isConfig) {
      isConfig = false;
      isRunning = true;
      counterStrip.clear();
      setsStrip.clear();
      counterStrip.show();
      setsStrip.show();
      displayActiveMarkers();
      startCooldown();
    }
    else{
    if (completedSets >= modes[currentMode].sets) {
      isConfig = true;
      completedSets = 0;
      completedCycles = 0;
      counterStrip.clear();
      setsStrip.clear();
      counterStrip.show();
      setsStrip.show();
      displayModePreview();
    }
    else if (isRunning == false && completedSets < modes[currentMode].sets){
      completedCycles = 0;
      counterStrip.clear();
      counterStrip.show();
      displayActiveMarkers();
      startCooldown();
    }
    else if(isRunning == true && completedSets < modes[currentMode].sets) {
      isRunning = false;
      completedCycles = 0;
      step = 0;
      counterStrip.clear();
      counterStrip.show();
      mainStrip.clear();
      mainStrip.show();
      displayActiveMarkers();
      }
    }
  }
}
void startCooldown() {
  isCooldown = true;
  cooldownStart = millis();
}

void runCooldown() {
  unsigned long currentMillis = millis();
  if (currentMillis - cooldownStart >= cooldownDuration) {
    isCooldown = false;
    isRunning = true;
  } else {
    if ((currentMillis - cooldownStart) % 1000 < 100) {
      tone(BUZZER, 1000, 100);
    }
  }
}

void displayActiveMarkers() {
  counterStrip.setPixelColor(modes[currentMode].reps - 1, counterStrip.Color(0, 255, 255));
  setsStrip.setPixelColor(modes[currentMode].sets - 1, setsStrip.Color(255, 255, 0));
  counterStrip.show();
  setsStrip.show();
}

void runTraining() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= modes[currentMode].interval) {
    previousMillis = currentMillis;

    if (step < MAIN_STRIP_PIXELS) {
      mainStrip.setPixelColor(step, mainStrip.Color(255, 0, 0));
      mainStrip.show();
      step++;
    } else if (step < 2 * MAIN_STRIP_PIXELS) {
      mainStrip.setPixelColor(2 * MAIN_STRIP_PIXELS - step - 1, 0);
      mainStrip.show();
      step++;
    } else {
      step = 0;
      tone(BUZZER, 100, 100);
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
      if (completedSets >= modes[currentMode].sets) {
        tone(BUZZER, 100, 500);
      }
    }
  }
}

void handleBluetooth(){
  if (Serial.available()) {
    String receivedData = Serial.readStringUntil('\n');
    receivedData.trim();
    if (isConfig == true && receivedData!="start" && receivedData!="reset"){
      int firstSpace = receivedData.indexOf(' ');
      int secondSpace = receivedData.indexOf(' ', firstSpace + 1);
  
      String repString = receivedData.substring(0, firstSpace);
      String setString = receivedData.substring(firstSpace + 1, secondSpace);
      String paceString = receivedData.substring(secondSpace + 1);
      if (isValidInteger(repString) && isValidInteger(setString) && isValidInteger(paceString)) {
      reps = repString.toInt();
      sets = setString.toInt();
      pace = paceString.toInt();
      pace = pace/32;
      if (pace<10) pace=50;
      if (reps>=17 || reps<1) reps=8;
      if (sets>=17 || sets<1) sets=3;
      }
      else {
        reps=8;
        sets=3;
        pace=50;
        }
      modes[4] = {reps, sets, pace};
      currentMode = 4;
      displayModePreview();
    }
    if (receivedData=="reset"){
      isConfig = true;
      isRunning = false;
      isCooldown = false;
      completedSets = 0;
      completedCycles = 0;
      step = 0;
      counterStrip.clear();
      setsStrip.clear();
      mainStrip.clear();
      counterStrip.show();
      setsStrip.show();
      mainStrip.show();
      displayModePreview();
      }
    if (receivedData=="start"){
      if (isConfig) {
      isConfig = false;
      isRunning = true;
      counterStrip.clear();
      setsStrip.clear();
      counterStrip.show();
      setsStrip.show();
      displayActiveMarkers();
      startCooldown();
    }
    else{
    if (completedSets >= modes[currentMode].sets) {
      isConfig = true;
      completedSets = 0;
      completedCycles = 0;
      counterStrip.clear();
      setsStrip.clear();
      counterStrip.show();
      setsStrip.show();
      displayModePreview();
    }
    else if (isRunning == false && completedSets < modes[currentMode].sets && isCooldown==false){
      completedCycles = 0;
      counterStrip.clear();
      counterStrip.show();
      displayActiveMarkers();
      startCooldown();
    }
    else if(isRunning == true && completedSets < modes[currentMode].sets) {
      isRunning = false;
      completedCycles = 0;
      step = 0;
      counterStrip.clear();
      counterStrip.show();
      mainStrip.clear();
      mainStrip.show();
      displayActiveMarkers();
      }
    }
      }
  }
}
