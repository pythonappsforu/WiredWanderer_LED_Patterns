/*
 * Wired Wanderer - Light-Activated Dancing LED Lights with Multiple Bicolor LEDs
 *
 * Copyright © 2025 Wired Wanderer
 * YouTube Channel: https://www.youtube.com/@TheWiredWander
 *
 * This code is provided for educational and personal use.
 * Please attribute 'Wired Wanderer' if used in public projects or derivatives.
 *
 * Project Description:
 * This sketch controls 12 single-color LEDs and 2 bicolor LEDs based on ambient light
 * detected by an LDR. When it's dark, a sequence of various exciting and dramatic
 * light patterns runs. The patterns cycle automatically after a set duration.
 * Includes debouncing logic for the LDR and memory optimization using PROGMEM.
 */
#include <avr/pgmspace.h> // Required for PROGMEM

// --- Configuration for LEDs ---
const int ledPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]); // This will be 12

const int patternStepSpeed = 70; // General speed for pattern steps (in milliseconds)
const int patternDuration = 4000; // How long each pattern attempts to run (approx.)

// --- Configuration for LDR ---
const int ldrPin = A0;
int lightThreshold = 650; // Using your confirmed working threshold
const bool INVERT_LDR_LOGIC = true;
const unsigned long lightDebounceDelay = 500;

// --- Bicolor LED Pin Configurations ---
const int bicolor1RedPin = A4;
const int bicolor1GreenPin = A5;

const int bicolor2RedPin = A1;
const int bicolor2GreenPin = A2;

const int bicolorCycleSpeed = 500; // Speed of bicolor state changes

// --- Global Timing and State Variables ---
unsigned long currentPatternStartTime = 0;
int currentPatternIndex = 0;
bool lightsOn = false;

unsigned long lastLightStateChangeTime = 0;
bool lastDetectedLightState = false;


// Enum for different patterns
enum PatternType {
  KNIGHT_RIDER,
  BLINK_ALL,
  CHASE_OUT_IN,
  RANDOM_SPARKLE,
  ALTERNATE_BLINK,
  CYLON_EYE,
  WAVE_PATTERN,
  TWINKLE,
  MARQUEE_FILL_CLEAR,
  PING_PONG,
  RANDOM_TWINKLE_BURST,
  BICOLOR_SINGLE_CYCLE,
  BICOLOR_DUO_ALTERNATE,
  BICOLOR_DUO_SYNCHRONOUS,
  STROBE_FLASH_BICOLOR_PULSE,
  RANDOM_FIREFLY_SWARM,
  HEARTBEAT_PULSE,
  ASCENDING_DESCENDING_CASCADE,
  BICOLOR_TRAIL_AND_BLINK,
  CENTER_PULSE_BICOLOR_SHIFT,
  RANDOM_CHASE_COLOR_SHIFT,
  BREATHING_DUO_BICOLOR,
  SPARKLE_BURST_BICOLOR_FLASH,
  ALTERNATING_WAVES_BICOLOR_HOLD,
  CENTER_IMPLOSION_BICOLOR_SWIRL,
  RAINDROP_DRIP_EFFECT,
  NUM_PATTERNS // This will be the count of all patterns
};

// Array of pattern names for serial output (MUST match enum order!)
// MOVED TO PROGMEM TO SAVE SRAM!
const char* const patternNames[] PROGMEM = {
  "Knight Rider",
  "Blink All",
  "Chase Out-In",
  "Random Sparkle",
  "Alternate Blink",
  "Cylon Eye",
  "Wave Pattern",
  "Twinkle",
  "Marquee Fill & Clear",
  "Ping Pong",
  "Random Twinkle Burst",
  "Bicolor Single Cycle",
  "Bicolor Duo Alternate",
  "Bicolor Duo Synchronous",
  "Strobe Flash & Bicolor Pulse",
  "Random Firefly Swarm",
  "Heartbeat Pulse",
  "Ascending/Descending Cascade",
  "Bicolor Trail & Blink",
  "Center Pulse with Bicolor Shift",
  "Random Chase with Color Shift",
  "Breathing Duo Bicolor",
  "Sparkle Burst & Bicolor Flash",
  "Alternating Waves with Bicolor Hold",
  "Center Implosion with Bicolor Swirl",
  "Raindrop/Drip Effect"
};

// Function prototypes
void allLedsOff();
void setBicolor(int bicolorIndex, bool redOn, bool greenOn);
void allBicolorOff();
void runPattern(PatternType pattern);

// Existing individual LED pattern prototypes
void knightRider(unsigned long currentMillis);
void blinkAll(unsigned long currentMillis);
void chaseOutIn(unsigned long currentMillis);
void randomSparkle(unsigned long currentMillis);
void alternateBlink(unsigned long currentMillis);
void cylonEye(unsigned long currentMillis);
void wavePattern(unsigned long currentMillis);
void twinkle(unsigned long currentMillis);
void marqueeFillClear(unsigned long currentMillis);
void pingPong(unsigned long currentMillis);
void randomTwinkleBurst(unsigned long currentMillis);

// Bicolor LED pattern functions
void bicolorSingleCycle(unsigned long currentMillis);
void bicolorDuoAlternate(unsigned long currentMillis);
void bicolorDuoSynchronous(unsigned long currentMillis);

// DRAMATIC PATTERN PROTOTYPES
void strobeFlashBicolorPulse(unsigned long currentMillis);
void randomFireflySwarm(unsigned long currentMillis);
void heartbeatPulse(unsigned long currentMillis);
void ascendingDescendingCascade(unsigned long currentMillis);
void bicolorTrailAndBlink(unsigned long currentMillis);
void centerPulseBicolorShift(unsigned long currentMillis);
void randomChaseColorShift(unsigned long currentMillis);
void breathingDuoBicolor(unsigned long currentMillis);
void sparkleBurstBicolorFlash(unsigned long currentMillis);
void alternatingWavesBicolorHold(unsigned long currentMillis);
void centerImplosionBicolorSwirl(unsigned long currentMillis);
void raindropDripEffect(unsigned long currentMillis);


void setup() {
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }
  pinMode(ldrPin, INPUT);

  pinMode(bicolor1RedPin, OUTPUT);
  pinMode(bicolor1GreenPin, OUTPUT);
  pinMode(bicolor2RedPin, OUTPUT);
  pinMode(bicolor2GreenPin, OUTPUT);

  allBicolorOff();

  Serial.begin(9600);
  Serial.println(F("--- Responsive Light-Activated Dancing LED Lights with Multiple Bicolor LEDs ---")); // F() macro for String literals
  Serial.println(F("Monitoring ambient light..."));
  Serial.println(F("---------------------------------------------------"));
  Serial.println(F("CALIBRATION GUIDE (INVERT_LDR_LOGIC = true):"));
  Serial.println(F("1. In BRIGHT light, note 'LDR' reading (This value is higher)."));
  Serial.println(F("2. In DARK light, note 'LDR' reading (This value is lower)."));
  Serial.println(F("3. Set 'lightThreshold' to a value BETWEEN these two."));
  Serial.println(F("   Example: Bright=700, Dark=100 -> Threshold=400-500. Lights ON when LDR < Threshold."));
  Serial.println(F("4. Observe 'DebouncedDarkEnough' and 'LightsOn' for stable behavior."));
  Serial.println(F("---------------------------------------------------"));
  Serial.println(F("Memory optimizations applied!"));


  randomSeed(analogRead(A0)); // Seed random number generator
  allLedsOff();
  allBicolorOff();
}

void loop() {
  unsigned long currentMillis = millis();
  int lightValue = analogRead(ldrPin);

  bool currentDetectedLightState;
  if (INVERT_LDR_LOGIC) {
    currentDetectedLightState = (lightValue < lightThreshold);
  } else {
    currentDetectedLightState = (lightValue > lightThreshold);
  }

  // --- Debounce Logic ---
  if (currentDetectedLightState != lastDetectedLightState) {
    lastLightStateChangeTime = currentMillis;
    lastDetectedLightState = currentDetectedLightState;
  }

  bool debouncedDarkEnough = false;
  if ((currentMillis - lastLightStateChangeTime) >= lightDebounceDelay) {
    debouncedDarkEnough = lastDetectedLightState;
  } else {
    debouncedDarkEnough = lightsOn;
  }


  // Print debug info (comment out if not needed for cleaner output)
  Serial.print(F("LDR:")); Serial.print(lightValue);
  Serial.print(F(" Thr:")); Serial.print(lightThreshold);
  Serial.print(F(" Inv:")); Serial.print(INVERT_LDR_LOGIC ? F("T") : F("F"));
  Serial.print(F(" RawDark:")); Serial.print(currentDetectedLightState ? F("T") : F("F"));
  Serial.print(F(" DebouncedDark:")); Serial.print(debouncedDarkEnough ? F("T") : F("F"));
  Serial.print(F(" LightsOn:")); Serial.print(lightsOn ? F("T") : F("F"));
  Serial.print(F(" PatIdx:")); Serial.print(currentPatternIndex);
  Serial.print(F(" -> "));


  // --- Main Light Sensing and Pattern Logic ---
  if (debouncedDarkEnough) {
    if (!lightsOn) {
      Serial.println(F("IT'S DARK! Turning lights ON. ***RESETTING PATTERN TO 0***"));
      lightsOn = true;
      currentPatternIndex = 0;
      currentPatternStartTime = currentMillis;
      allLedsOff();
      allBicolorOff();
      randomSeed(analogRead(A0) + analogRead(A1) + currentMillis);
      Serial.print(F("--- Starting first pattern: "));
      // Read pattern name from PROGMEM
      Serial.println((const __FlashStringHelper*)pgm_read_word_near(&(patternNames[currentPatternIndex])));
    }

    runPattern(static_cast<PatternType>(currentPatternIndex));

    if (currentMillis - currentPatternStartTime >= patternDuration) {
      currentPatternIndex = (currentPatternIndex + 1) % NUM_PATTERNS;
      currentPatternStartTime = currentMillis;
      allLedsOff();
      allBicolorOff();
      Serial.print(F("--- Switching to pattern: "));
      // Read pattern name from PROGMEM
      Serial.println((const __FlashStringHelper*)pgm_read_word_near(&(patternNames[currentPatternIndex])));
      randomSeed(analogRead(A0) + analogRead(A1) + currentMillis);
    }

  } else {
    if (lightsOn) {
      Serial.println(F("IT'S BRIGHT. Turning lights OFF. LightsOn=FALSE"));
      lightsOn = false;
      allLedsOff();
      allBicolorOff();
    }
    delay(10);
  }
}

// --- Helper Functions ---
void allLedsOff() {
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void setBicolor(int bicolorIndex, bool redOn, bool greenOn) {
  // Common Cathode assumption: HIGH turns on LED.
  switch (bicolorIndex) {
    case 1: // Bicolor LED 1 (A4, A5)
      digitalWrite(bicolor1RedPin, redOn ? HIGH : LOW);
      digitalWrite(bicolor1GreenPin, greenOn ? HIGH : LOW);
      break;
    case 2: // Bicolor LED 2 (A1, A2)
      digitalWrite(bicolor2RedPin, redOn ? HIGH : LOW);
      digitalWrite(bicolor2GreenPin, greenOn ? HIGH : LOW);
      break;
  }
}

void allBicolorOff() {
  setBicolor(1, false, false);
  setBicolor(2, false, false);
}


// --- Pattern Runner Function ---
void runPattern(PatternType pattern) {
  unsigned long currentMillis = millis();

  switch (pattern) {
    case STROBE_FLASH_BICOLOR_PULSE:
    case RANDOM_FIREFLY_SWARM:
    case HEARTBEAT_PULSE:
    case ASCENDING_DESCENDING_CASCADE:
    case BICOLOR_TRAIL_AND_BLINK:
    case CENTER_PULSE_BICOLOR_SHIFT:
    case RANDOM_CHASE_COLOR_SHIFT:
    case BREATHING_DUO_BICOLOR:
    case SPARKLE_BURST_BICOLOR_FLASH:
    case ALTERNATING_WAVES_BICOLOR_HOLD:
    case CENTER_IMPLOSION_BICOLOR_SWIRL:
    case RAINDROP_DRIP_EFFECT:
      // For these dramatic patterns, we'll let them control all LEDs
      // and bicolors within their own function, without blanket turning off
      // at the start of runPattern.
      if (pattern == STROBE_FLASH_BICOLOR_PULSE) strobeFlashBicolorPulse(currentMillis);
      else if (pattern == RANDOM_FIREFLY_SWARM) randomFireflySwarm(currentMillis);
      else if (pattern == HEARTBEAT_PULSE) heartbeatPulse(currentMillis);
      else if (pattern == ASCENDING_DESCENDING_CASCADE) ascendingDescendingCascade(currentMillis);
      else if (pattern == BICOLOR_TRAIL_AND_BLINK) bicolorTrailAndBlink(currentMillis);
      else if (pattern == CENTER_PULSE_BICOLOR_SHIFT) centerPulseBicolorShift(currentMillis);
      else if (pattern == RANDOM_CHASE_COLOR_SHIFT) randomChaseColorShift(currentMillis);
      else if (pattern == BREATHING_DUO_BICOLOR) breathingDuoBicolor(currentMillis);
      else if (pattern == SPARKLE_BURST_BICOLOR_FLASH) sparkleBurstBicolorFlash(currentMillis);
      else if (pattern == ALTERNATING_WAVES_BICOLOR_HOLD) alternatingWavesBicolorHold(currentMillis);
      else if (pattern == CENTER_IMPLOSION_BICOLOR_SWIRL) centerImplosionBicolorSwirl(currentMillis);
      else if (pattern == RAINDROP_DRIP_EFFECT) raindropDripEffect(currentMillis);
      break;

    case BICOLOR_SINGLE_CYCLE:
    case BICOLOR_DUO_ALTERNATE:
    case BICOLOR_DUO_SYNCHRONOUS:
      allLedsOff(); // Ensure 2-legged LEDs are off when these Bicolor patterns are active
      if (pattern == BICOLOR_SINGLE_CYCLE) bicolorSingleCycle(currentMillis);
      else if (pattern == BICOLOR_DUO_ALTERNATE) bicolorDuoAlternate(currentMillis);
      else bicolorDuoSynchronous(currentMillis);
      break;

    default:
      allBicolorOff(); // Ensure Bicolor LEDs are off when 2-legged patterns are active
      if (pattern == KNIGHT_RIDER) knightRider(currentMillis);
      else if (pattern == BLINK_ALL) blinkAll(currentMillis);
      else if (pattern == CHASE_OUT_IN) chaseOutIn(currentMillis);
      else if (pattern == RANDOM_SPARKLE) randomSparkle(currentMillis);
      else if (pattern == ALTERNATE_BLINK) alternateBlink(currentMillis);
      else if (pattern == CYLON_EYE) cylonEye(currentMillis);
      else if (pattern == WAVE_PATTERN) wavePattern(currentMillis);
      else if (pattern == TWINKLE) twinkle(currentMillis);
      else if (pattern == MARQUEE_FILL_CLEAR) marqueeFillClear(currentMillis);
      else if (pattern == PING_PONG) pingPong(currentMillis);
      else if (pattern == RANDOM_TWINKLE_BURST) randomTwinkleBurst(currentMillis);
      else allLedsOff(); // Should not happen
      break;
  }
}

// --- Existing Pattern Implementations (updated static array sizes and F() macro for Serial.print) ---
void knightRider(unsigned long currentMillis) {
  static int currentLed = 0; static bool forward = true; static unsigned long lastStepTime = 0;
  if (currentMillis - lastStepTime >= patternStepSpeed) { lastStepTime = currentMillis; allLedsOff();
    if (forward) { digitalWrite(ledPins[currentLed], HIGH); currentLed++; if (currentLed >= numLeds) { currentLed = numLeds - 2; forward = false; } }
    else { digitalWrite(ledPins[currentLed], HIGH); currentLed--; if (currentLed < 1) { currentLed = 0; forward = true; } } } }

void blinkAll(unsigned long currentMillis) {
  static unsigned long lastToggleTime = 0; static bool ledState = LOW;
  if (currentMillis - lastToggleTime >= (unsigned long)patternStepSpeed * 5) { lastToggleTime = currentMillis; ledState = !ledState;
    for (int i = 0; i < numLeds; i++) { digitalWrite(ledPins[i], ledState); } } }

void chaseOutIn(unsigned long currentMillis) {
  static int step = 0; static bool outward = true; static unsigned long lastStepTime = 0;
  int center1 = (numLeds - 1) / 2; int center2 = numLeds / 2;
  if (currentMillis - lastStepTime >= patternStepSpeed) { lastStepTime = currentMillis; allLedsOff();
    if (outward) { if (step <= center1) { digitalWrite(ledPins[center1 - step], HIGH); digitalWrite(ledPins[center2 + step], HIGH); step++; } else { outward = false; step--; } }
    else { if (step >= 0) { digitalWrite(ledPins[center1 - step], HIGH); digitalWrite(ledPins[center2 + step], HIGH); step--; } else { outward = true; step = 0; } } } }

void randomSparkle(unsigned long currentMillis) {
  static unsigned long lastSparkleTime = 0; static int lastSparkledLed = -1;
  if (currentMillis - lastSparkleTime >= random(10, 80)) { lastSparkleTime = currentMillis;
    if (lastSparkledLed != -1) { digitalWrite(ledPins[lastSparkledLed], LOW); }
    int randLed = random(0, numLeds); digitalWrite(ledPins[randLed], HIGH); lastSparkledLed = randLed; } }

void alternateBlink(unsigned long currentMillis) {
  static unsigned long lastToggleTime = 0; static bool oddOn = true;
  if (currentMillis - lastToggleTime >= (unsigned long)patternStepSpeed * 3) { lastToggleTime = currentMillis; oddOn = !oddOn;
    for (int i = 0; i < numLeds; i++) { if (i % 2 == 0) { digitalWrite(ledPins[i], oddOn ? LOW : HIGH); } else { digitalWrite(ledPins[i], oddOn ? HIGH : LOW); } } } }

void cylonEye(unsigned long currentMillis) {
  static int currentLed = 0; static bool forward = true; static unsigned long lastStepTime = 0;
  if (currentMillis - lastStepTime >= patternStepSpeed) { lastStepTime = currentMillis; allLedsOff(); digitalWrite(ledPins[currentLed], HIGH);
    if (forward) { currentLed++; if (currentLed >= numLeds) { currentLed = numLeds - 2; forward = false; } }
    else { currentLed--; if (currentLed < 1) { currentLed = 0; forward = true; } } } }

void wavePattern(unsigned long currentMillis) {
  static int currentLed = 0; static unsigned long lastStepTime = 0; const int waveTrailDuration = patternStepSpeed * 3;
  static unsigned long ledOnTime[numLeds] = {0}; // Sized to numLeds
  if (currentMillis - lastStepTime >= patternStepSpeed) { lastStepTime = currentMillis; digitalWrite(ledPins[currentLed], HIGH); ledOnTime[currentLed] = currentMillis; currentLed++; if (currentLed >= numLeds) { currentLed = 0; } }
  for (int i = 0; i < numLeds; i++) { if (digitalRead(ledPins[i]) == HIGH && (currentMillis - ledOnTime[i] >= waveTrailDuration)) { digitalWrite(ledPins[i], LOW); } } }

void twinkle(unsigned long currentMillis) {
  static unsigned long lastTwinkleCheckTime = 0; const int checkInterval = 20; const int twinkleDuration = 100;
  static unsigned long ledTwinkleOnTime[numLeds] = {0}; // Sized to numLeds
  if (currentMillis - lastTwinkleCheckTime >= checkInterval) { lastTwinkleCheckTime = currentMillis;
    if (random(0, 100) < 20) { int randLed = random(0, numLeds); if (digitalRead(ledPins[randLed]) == LOW) { digitalWrite(ledPins[randLed], HIGH); ledTwinkleOnTime[randLed] = currentMillis; } } }
  for (int i = 0; i < numLeds; i++) { if (digitalRead(ledPins[i]) == HIGH && (currentMillis - ledTwinkleOnTime[i] >= twinkleDuration)) { digitalWrite(ledPins[i], LOW); } } }

void marqueeFillClear(unsigned long currentMillis) {
  static int currentFillLed = 0; static bool filling = true; static unsigned long lastStepTime = 0; const int fillSpeed = patternStepSpeed * 2;
  if (currentMillis - lastStepTime >= fillSpeed) { lastStepTime = currentMillis;
    if (filling) { digitalWrite(ledPins[currentFillLed], HIGH); currentFillLed++; if (currentFillLed >= numLeds) { filling = false; currentFillLed = 0; } }
    else { digitalWrite(ledPins[currentFillLed], LOW); currentFillLed++; if (currentFillLed >= numLeds) { filling = true; currentFillLed = 0; } } } }

void pingPong(unsigned long currentMillis) {
  static int currentLed = 0; static bool forward = true; static unsigned long lastStepTime = 0; const int pingPongSpeed = patternStepSpeed;
  if (currentMillis - lastStepTime >= pingPongSpeed) { lastStepTime = currentMillis; allLedsOff(); digitalWrite(ledPins[currentLed], HIGH);
    if (forward) { currentLed++; if (currentLed >= numLeds - 1) { forward = false; } }
    else { currentLed--; if (currentLed <= 0) { forward = true; } } } }

void randomTwinkleBurst(unsigned long currentMillis) {
  static unsigned long lastTwinkleActionTime = 0; const int actionInterval = 10; const int onDuration = 50;
  static unsigned long ledOnTimers[numLeds] = {0}; // Sized to numLeds
  if (currentMillis - lastTwinkleActionTime >= actionInterval) { lastTwinkleActionTime = currentMillis;
    int numFlashes = random(1, 4); for (int i = 0; i < numFlashes; i++) { int randLed = random(0, numLeds); if (digitalRead(ledPins[randLed]) == LOW) { digitalWrite(ledPins[randLed], HIGH); ledOnTimers[randLed] = currentMillis; } } }
  for (int i = 0; i < numLeds; i++) { if (digitalRead(ledPins[i]) == HIGH && (currentMillis - ledOnTimers[i] >= onDuration)) { digitalWrite(ledPins[i], LOW); } } }


// --- Bicolor LED Pattern Implementations ---
void bicolorSingleCycle(unsigned long currentMillis) {
  static unsigned long lastCycleTime = 0; static int bicolorState = 0;
  if (currentMillis - lastCycleTime >= bicolorCycleSpeed) { lastCycleTime = currentMillis; bicolorState = (bicolorState + 1) % 3; allBicolorOff();
    switch (bicolorState) { case 0: setBicolor(1, true, false); break; case 1: setBicolor(1, false, true); break; case 2: setBicolor(1, true, true); break; } } }

void bicolorDuoAlternate(unsigned long currentMillis) {
  static unsigned long lastToggleTime = 0; static bool stateRedGreen = true;
  if (currentMillis - lastToggleTime >= bicolorCycleSpeed * 1.5) { lastToggleTime = currentMillis; stateRedGreen = !stateRedGreen;
    if (stateRedGreen) { setBicolor(1, true, false); setBicolor(2, false, true); } else { setBicolor(1, false, true); setBicolor(2, true, false); } } }

void bicolorDuoSynchronous(unsigned long currentMillis) {
  static unsigned long lastToggleTime = 0; static int colorState = 0;
  if (currentMillis - lastToggleTime >= bicolorCycleSpeed) { lastToggleTime = currentMillis; colorState = (colorState + 1) % 3;
    switch (colorState) { case 0: setBicolor(1, true, false); setBicolor(2, true, false); break; case 1: setBicolor(1, false, true); setBicolor(2, false, true); break; case 2: setBicolor(1, true, true); setBicolor(2, true, true); break; } } }


// --- DRAMATIC PATTERNS ---

void strobeFlashBicolorPulse(unsigned long currentMillis) {
  static unsigned long lastStrobeTime = 0;
  static unsigned long lastBicolorPulseTime = 0;
  static bool bicolorPulseUp = true;
  const int strobeSpeed = 30;
  const int pulseDuration = 200;

  if (currentMillis - lastStrobeTime >= strobeSpeed) {
    lastStrobeTime = currentMillis;
    allLedsOff();
    digitalWrite(ledPins[random(numLeds)], HIGH);
  }

  if (currentMillis - lastBicolorPulseTime >= pulseDuration) {
    lastBicolorPulseTime = currentMillis;
    if (bicolorPulseUp) {
      setBicolor(1, true, true);
      setBicolor(2, true, true);
    } else {
      allBicolorOff();
    }
    bicolorPulseUp = !bicolorPulseUp;
  }
}

void randomFireflySwarm(unsigned long currentMillis) {
  static unsigned long lastActionTime = 0;
  const int actionInterval = 20;
  const int onDuration = 150;
  static unsigned long ledOnTimers[numLeds] = {0}; // Sized to numLeds

  if (currentMillis - lastActionTime >= actionInterval) {
    lastActionTime = currentMillis;
    int numToTurnOn = random(1, 4);
    for (int i = 0; i < numToTurnOn; i++) {
      int randLed = random(numLeds);
      if (digitalRead(ledPins[randLed]) == LOW) {
        digitalWrite(ledPins[randLed], HIGH);
        ledOnTimers[randLed] = currentMillis;
      }
    }
  }

  for (int i = 0; i < numLeds; i++) {
    if (digitalRead(ledPins[i]) == HIGH && (currentMillis - ledOnTimers[i] >= onDuration)) {
      digitalWrite(ledPins[i], LOW);
    }
  }

  const int bicolorShiftInterval = 500;
  static unsigned long lastBicolorShiftTime = 0;
  if (currentMillis - lastBicolorShiftTime >= bicolorShiftInterval) {
    lastBicolorShiftTime = currentMillis;
    int color1 = random(3);
    if (color1 == 0) setBicolor(1, true, false); else if (color1 == 1) setBicolor(1, false, true); else setBicolor(1, true, true);
    int color2 = random(3);
    if (color2 == 0) setBicolor(2, true, false); else if (color2 == 1) setBicolor(2, false, true); else setBicolor(2, true, true);
  }
}

void heartbeatPulse(unsigned long currentMillis) {
  static unsigned long lastBeatTime = 0;
  static int beatPhase = 0;
  const int expandSpeed = 50;
  const int contractSpeed = 30;
  const int pauseDuration = 300;
  const int flashDuration = 100;

  if (beatPhase == 0 && currentMillis - lastBeatTime < expandSpeed) {
      allLedsOff();
      allBicolorOff();
  }

  switch (beatPhase) {
    case 0:
      if (currentMillis - lastBeatTime >= expandSpeed) {
        int center1 = (numLeds - 1) / 2;
        int center2 = numLeds / 2;
        int currentLedIndex = (currentMillis - lastBeatTime) / expandSpeed;

        if (currentLedIndex < numLeds / 2) {
            digitalWrite(ledPins[center1 - currentLedIndex], HIGH);
            digitalWrite(ledPins[center2 + currentLedIndex], HIGH);
        } else {
            beatPhase = 1;
            lastBeatTime = currentMillis;
        }
      }
      break;

    case 1:
      if (currentMillis - lastBeatTime >= contractSpeed) {
        int center1 = (numLeds - 1) / 2;
        int center2 = numLeds / 2;
        int currentLedIndex = (currentMillis - lastBeatTime) / contractSpeed;

        if (currentLedIndex < numLeds / 2) {
            digitalWrite(ledPins[center1 - currentLedIndex], LOW);
            digitalWrite(ledPins[center2 + currentLedIndex], LOW);
        } else {
            beatPhase = 2;
            lastBeatTime = currentMillis;
        }
      }
      break;

    case 2:
      if (currentMillis - lastBeatTime >= pauseDuration) {
        beatPhase = 3;
        lastBeatTime = currentMillis;
        setBicolor(1, true, true);
        setBicolor(2, true, true);
      }
      break;

    case 3:
      if (currentMillis - lastBeatTime >= flashDuration) {
        allBicolorOff();
        beatPhase = 0;
        lastBeatTime = currentMillis;
      }
      break;
  }
}

void ascendingDescendingCascade(unsigned long currentMillis) {
  static unsigned long lastStepTime = 0;
  static int currentStep = 0;
  static bool ascending = true;
  const int cascadeSpeed = 40;
  const int fullPassDelay = 200;

  if (currentMillis - lastStepTime >= cascadeSpeed) {
    lastStepTime = currentMillis;

    if (ascending) {
      if (currentStep > 0) digitalWrite(ledPins[currentStep - 1], LOW);
    } else {
      if (currentStep < numLeds - 1) digitalWrite(ledPins[currentStep + 1], LOW);
    }

    digitalWrite(ledPins[currentStep], HIGH);

    if (ascending) {
      setBicolor(1, true, false); // LED1 Red (for ascending)
      setBicolor(2, false, false);
    } else {
      setBicolor(1, false, false);
      setBicolor(2, true, false); // LED2 Red (for descending)
    }

    if (ascending) {
      currentStep++;
      if (currentStep >= numLeds) {
        currentStep = numLeds - 1;
        ascending = false;
        lastStepTime = currentMillis + fullPassDelay;
        allLedsOff();
        allBicolorOff();
      }
    } else {
      currentStep--;
      if (currentStep < 0) {
        currentStep = 0;
        ascending = true;
        lastStepTime = currentMillis + fullPassDelay;
        allLedsOff();
        allBicolorOff();
      }
    }
  }
}

// --- DRAMATIC PATTERNS (4 from last turn) ---

void bicolorTrailAndBlink(unsigned long currentMillis) {
  static unsigned long lastStepTime = 0;
  static int currentLedSegment = 0;
  static int phase = 0;
  const int segmentSpeed = 100;
  const int blinkDuration = 300;

  switch (phase) {
    case 0:
      if (currentMillis - lastStepTime >= segmentSpeed) {
        lastStepTime = currentMillis;
        allLedsOff();
        allBicolorOff();

        // Light up a segment of 3 LEDs, leaving the previous ones on
        // Adjusted to use numLeds in loop condition to avoid hardcoded limits
        for (int i = 0; i <= currentLedSegment; i++) {
            if (i * 3 < numLeds) {
                digitalWrite(ledPins[i * 3], HIGH);
            }
            if (i * 3 + 1 < numLeds) {
                digitalWrite(ledPins[i * 3 + 1], HIGH);
            }
            if (i * 3 + 2 < numLeds) {
                digitalWrite(ledPins[i * 3 + 2], HIGH);
            }
        }
        currentLedSegment++;
        if (currentLedSegment * 3 >= numLeds) {
          currentLedSegment = 0;
          phase = 1;
          lastStepTime = currentMillis;
          allLedsOff();
        }
      }
      break;

    case 1:
      if (currentMillis - lastStepTime >= blinkDuration) {
        lastStepTime = currentMillis;
        allBicolorOff();
        phase = 0;
      } else {
        setBicolor(1, true, true);
        setBicolor(2, true, true);
      }
      break;
  }
}

void centerPulseBicolorShift(unsigned long currentMillis) {
  static unsigned long lastPulseTime = 0;
  static unsigned long lastBicolorShiftTime = 0;
  static int pulseLedIndex = 0;
  static bool pulseExpanding = true;
  static int bicolorColorIndex = 0;

  const int pulseSpeed = 40;
  const int bicolorShiftSpeed = 300;

  int center1 = (numLeds - 1) / 2;
  int center2 = numLeds / 2;

  if (currentMillis - lastPulseTime >= pulseSpeed) {
    lastPulseTime = currentMillis;
    allLedsOff();

    if (pulseExpanding) {
      if (pulseLedIndex <= center1) {
        digitalWrite(ledPins[center1 - pulseLedIndex], HIGH);
        digitalWrite(ledPins[center2 + pulseLedIndex], HIGH);
        pulseLedIndex++;
      } else {
        pulseExpanding = false;
        pulseLedIndex = (numLeds / 2) - 1;
      }
    } else {
      if (pulseLedIndex >= 0) {
        digitalWrite(ledPins[center1 - pulseLedIndex], HIGH);
        digitalWrite(ledPins[center2 + pulseLedIndex], HIGH);
        pulseLedIndex--;
      } else {
        pulseExpanding = true;
        pulseLedIndex = 0;
      }
    }
  }

  if (currentMillis - lastBicolorShiftTime >= bicolorShiftSpeed) {
    lastBicolorShiftTime = currentMillis;
    bicolorColorIndex = (bicolorColorIndex + 1) % 3;

    switch (bicolorColorIndex) {
      case 0: setBicolor(1, true, false); setBicolor(2, true, false); break;
      case 1: setBicolor(1, false, true); setBicolor(2, false, true); break;
      case 2: setBicolor(1, true, true); setBicolor(2, true, true); break;
    }
  }
}

void randomChaseColorShift(unsigned long currentMillis) {
  static unsigned long lastChaseStepTime = 0;
  static unsigned long lastChaseResetTime = 0;
  static int startLed = -1;
  static int endLed = -1;
  static int currentChaseLed = -1;
  static int chaseSpeed = 50;
  const int chaseResetInterval = 1500;

  if (currentMillis - lastChaseResetTime >= chaseResetInterval) {
    lastChaseResetTime = currentMillis;
    allLedsOff();

    startLed = random(numLeds);
    do { endLed = random(numLeds); } while (endLed == startLed);

    currentChaseLed = startLed;
    lastChaseStepTime = currentMillis;

    int colorMode = random(3);
    if (colorMode == 0) { setBicolor(1, true, false); setBicolor(2, false, true); }
    else if (colorMode == 1) { setBicolor(1, false, true); setBicolor(2, true, false); }
    else { setBicolor(1, true, true); setBicolor(2, true, true); }
  }

  if (currentChaseLed != -1 && currentMillis - lastChaseStepTime >= chaseSpeed) {
    lastChaseStepTime = currentMillis;

    if (startLed < endLed) { if (currentChaseLed > startLed) digitalWrite(ledPins[currentChaseLed - 1], LOW); }
    else { if (currentChaseLed < startLed) digitalWrite(ledPins[currentChaseLed + 1], LOW); }

    digitalWrite(ledPins[currentChaseLed], HIGH);

    if (startLed < endLed) { if (currentChaseLed < endLed) currentChaseLed++; else { currentChaseLed = -1; allLedsOff(); } }
    else { if (currentChaseLed > endLed) currentChaseLed--; else { currentChaseLed = -1; allLedsOff(); } }
  }
}

void breathingDuoBicolor(unsigned long currentMillis) {
  static unsigned long lastFadeTime = 0;
  static int brightness = 0;
  static bool fadingUp = true;
  static int bicolorColor = 0;

  const int fadeSpeed = 8;
  const int fadeSteps = 10;
  const int colorChangeInterval = 2000;

  allLedsOff();

  if (currentMillis - lastFadeTime >= fadeSpeed) {
    lastFadeTime = currentMillis;

    if (fadingUp) {
      brightness++;
      if (brightness > fadeSteps) { brightness = fadeSteps; fadingUp = false; }
    } else {
      brightness--;
      if (brightness < 0) { brightness = 0; fadingUp = true; bicolorColor = (bicolorColor + 1) % 3; }
    }

    bool ledOn = (brightness > fadeSteps / 2);

    switch (bicolorColor) {
      case 0: setBicolor(1, ledOn, false); setBicolor(2, ledOn, false); break;
      case 1: setBicolor(1, false, ledOn); setBicolor(2, false, ledOn); break;
      case 2: setBicolor(1, ledOn, ledOn); setBicolor(2, ledOn, ledOn); break;
    }
  }
}

// --- NEW DRAMATIC PATTERNS ---

// NEW DRAMATIC PATTERN 9: Sparkle Burst & Bicolor Flash
void sparkleBurstBicolorFlash(unsigned long currentMillis) {
  static unsigned long lastActionTime = 0;
  static int phase = 0; // 0: Burst, 1: Bicolor Flash, 2: Pause
  const int burstDuration = 150; // How long LEDs are actively bursting
  const int burstFlashSpeed = 10; // Rapidly turn LEDs on/off during burst
  const int bicolorFlashDuration = 200; // How long bicolors stay on
  const int pauseDuration = 500; // Pause after sequence

  switch (phase) {
    case 0: // Burst
      if (currentMillis - lastActionTime >= burstFlashSpeed) {
        lastActionTime = currentMillis;
        // Randomly turn on/off some LEDs during burst
        for (int i = 0; i < numLeds; i++) {
          digitalWrite(ledPins[i], random(2)); // Randomly on or off
        }
      }
      if (currentMillis - currentPatternStartTime >= burstDuration) {
        allLedsOff(); // Clear all single LEDs
        phase = 1; // Move to bicolor flash
        lastActionTime = currentMillis; // Reset for next phase
      }
      break;

    case 1: // Bicolor Flash
      setBicolor(1, true, true); // Both bicolors on (orange/yellow)
      setBicolor(2, true, true);
      if (currentMillis - lastActionTime >= bicolorFlashDuration) {
        allBicolorOff(); // Turn off bicolors
        phase = 2; // Move to pause
        lastActionTime = currentMillis; // Reset for next phase
      }
      break;

    case 2: // Pause
      if (currentMillis - lastActionTime >= pauseDuration) {
        phase = 0; // Reset to burst
        currentPatternStartTime = currentMillis; // Reset overall pattern timer
      }
      break;
  }
}

// NEW DRAMATIC PATTERN 10: Alternating Waves with Bicolor Hold
void alternatingWavesBicolorHold(unsigned long currentMillis) {
  static unsigned long lastStepTime = 0;
  static int currentWavePos1 = 0; // For wave 1 (ends to center)
  static int currentWavePos2 = 0; // For wave 2 (center to ends)
  static bool wave1DirectionIn = true; // true = ends to center, false = center to ends
  static bool wave2DirectionOut = true; // true = center to ends, false = ends to center
  const int waveSpeed = 60; // Speed of LED movement
  const int waveTrail = 1; // Number of trailing LEDs to keep on (0 for no trail, 1 for 1 trailing LED)

  int center1 = (numLeds - 1) / 2;
  int center2 = numLeds / 2;

  // Set Bicolor color based on overall direction
  if (wave1DirectionIn) { // Waves moving inwards primarily
    setBicolor(1, false, true); // Bicolor 1 Green
    setBicolor(2, false, true); // Bicolor 2 Green
  } else { // Waves moving outwards primarily
    setBicolor(1, true, false); // Bicolor 1 Red
    setBicolor(2, true, false); // Bicolor 2 Red
  }


  if (currentMillis - lastStepTime >= waveSpeed) {
    lastStepTime = currentMillis;

    // Clear all LEDs (or specific ones if using a more complex trail logic)
    allLedsOff();

    // Update and draw wave 1 (ends to center / center to ends)
    if (wave1DirectionIn) { // Moving inwards
      if (currentWavePos1 < numLeds / 2) {
        digitalWrite(ledPins[currentWavePos1], HIGH);
        digitalWrite(ledPins[numLeds - 1 - currentWavePos1], HIGH);
        if (currentWavePos1 > 0) { // Keep trailing LED on
            digitalWrite(ledPins[currentWavePos1-1], HIGH);
            digitalWrite(ledPins[numLeds - currentWavePos1], HIGH);
        }
        currentWavePos1++;
      } else {
        wave1DirectionIn = false;
        currentWavePos1 = (numLeds / 2) - 1; // Start moving outwards from center-ish
      }
    } else { // Moving outwards
      if (currentWavePos1 >= 0) {
        digitalWrite(ledPins[currentWavePos1], HIGH);
        digitalWrite(ledPins[numLeds - 1 - currentWavePos1], HIGH);
        if (currentWavePos1 < numLeds / 2 - 1) { // Keep trailing LED on
            digitalWrite(ledPins[currentWavePos1+1], HIGH);
            digitalWrite(ledPins[numLeds - 2 - currentWavePos1], HIGH);
        }
        currentWavePos1--;
      } else {
        wave1DirectionIn = true;
        currentWavePos1 = 0; // Reset to start inwards
      }
    }

    // Update and draw wave 2 (center to ends / ends to center)
    if (wave2DirectionOut) { // Moving outwards
      if (currentWavePos2 < numLeds / 2) {
        digitalWrite(ledPins[center1 - currentWavePos2], HIGH);
        digitalWrite(ledPins[center2 + currentWavePos2], HIGH);
        if (currentWavePos2 > 0) { // Keep trailing LED on
            digitalWrite(ledPins[center1 - currentWavePos2 + 1], HIGH);
            digitalWrite(ledPins[center2 + currentWavePos2 - 1], HIGH);
        }
        currentWavePos2++;
      } else {
        wave2DirectionOut = false;
        currentWavePos2 = (numLeds / 2) - 1; // Start moving inwards
      }
    } else { // Moving inwards
      if (currentWavePos2 >= 0) {
        digitalWrite(ledPins[center1 - currentWavePos2], HIGH);
        digitalWrite(ledPins[center2 + currentWavePos2], HIGH);
        if (currentWavePos2 < numLeds / 2 - 1) { // Keep trailing LED on
            digitalWrite(ledPins[center1 - currentWavePos2 - 1], HIGH);
            digitalWrite(ledPins[center2 + currentWavePos2 + 1], HIGH);
        }
        currentWavePos2--;
      } else {
        wave2DirectionOut = true;
        currentWavePos2 = 0; // Reset to start outwards
      }
    }
  }
}


// NEW DRAMATIC PATTERN 11: Center Implosion with Bicolor Swirl
// NEW DRAMATIC PATTERN 11: Center Implosion with Bicolor Swirl
void centerImplosionBicolorSwirl(unsigned long currentMillis) {
  static unsigned long lastStepTime = 0;
  static int currentImplosionStep = 0;
  static int phase = 0; // 0: Implode ON, 1: Implode OFF, 2: Bicolor Swirl, 3: Pause after swirl
  static int bicolorSwirlState = 0;

  const int implodeSpeed = 40;
  const int swirlSpeed = 80;
  const int swirlDuration = 800;   // Re-added: Duration of the bicolor swirl itself
  const int pauseAfterSwirl = 500; // Pause before restarting whole pattern cycle

  int center1 = (numLeds - 1) / 2;
  int center2 = numLeds / 2;


  switch (phase) {
    case 0: // Implode (LEDs turn ON towards center)
      if (currentMillis - lastStepTime >= implodeSpeed) {
        lastStepTime = currentMillis;
        allLedsOff(); // Clear all LEDs at each step for cleaner effect

        // Light LEDs from outside in
        if (currentImplosionStep < numLeds / 2) {
            digitalWrite(ledPins[currentImplosionStep], HIGH); // From left end
            digitalWrite(ledPins[numLeds - 1 - currentImplosionStep], HIGH); // From right end
            currentImplosionStep++;
        } else {
            // All active LEDs are now lit towards center
            phase = 1; // Move to turning them off
            lastStepTime = currentMillis; // Reset timer for next phase
            currentImplosionStep = (numLeds / 2) - 1; // Start turning off from inner
        }
      }
      break;

    case 1: // Implode (LEDs turn OFF towards center)
      if (currentMillis - lastStepTime >= implodeSpeed) {
        lastStepTime = currentMillis;

        // Turn off LEDs from inside out (clearing the implosion)
        if (currentImplosionStep >= 0) {
            digitalWrite(ledPins[currentImplosionStep], LOW);
            digitalWrite(ledPins[numLeds - 1 - currentImplosionStep], LOW);
            currentImplosionStep--;
        } else {
            // All LEDs are off
            phase = 2; // Move to bicolor swirl
            lastStepTime = currentMillis; // Reset timer
            allLedsOff(); // Just to be sure
        }
      }
      break;

    case 2: // Bicolor Swirl
      if (currentMillis - lastStepTime >= swirlSpeed) {
        lastStepTime = currentMillis;
        bicolorSwirlState = (bicolorSwirlState + 1) % 4; // Cycle through 4 states for a more complex swirl

        switch (bicolorSwirlState) {
          case 0: setBicolor(1, true, false); setBicolor(2, false, true); break; // LED1 Red, LED2 Green
          case 1: setBicolor(1, false, true); setBicolor(2, true, false); break; // LED1 Green, LED2 Red
          case 2: setBicolor(1, true, true); setBicolor(2, false, false); break; // LED1 Orange, LED2 Off
          case 3: setBicolor(1, false, false); setBicolor(2, true, true); break; // LED1 Off, LED2 Orange
        }
      }
      // Check if swirl duration is complete
      if (currentMillis - (lastStepTime - swirlSpeed) >= swirlDuration) { // Check from when swirl started
        allBicolorOff();
        phase = 3; // Move to pause
        lastStepTime = currentMillis; // Reset timer for pause
      }
      break;

    case 3: // Pause after swirl
      if (currentMillis - lastStepTime >= pauseAfterSwirl) {
        phase = 0; // Reset to start new implosion
        currentPatternStartTime = currentMillis; // Reset overall pattern timer
        currentImplosionStep = 0; // Reset for next implosion cycle
      }
      break;
  }
}

// NEW DRAMATIC PATTERN 12: Raindrop/Drip Effect
void raindropDripEffect(unsigned long currentMillis) {
  static unsigned long lastDripTime = 0;
  static unsigned long lastLedTravelTime = 0;
  static int activeDripLed = -1;
  static int dripTargetLed = -1; // The LED the drip is currently moving towards
  static int dripDirection = 0; // 0: no drip, 1: right, -1: left
  static unsigned long ledTravelOnTime[numLeds] = {0}; // To manage short trails

  const int newDripChance = 50; // % chance of new drip starting when ready
  const int newDripInterval = 200; // How often a new drip can potentially start
  const int dripTravelSpeed = 70; // How fast the "drip" moves
  const int dripOnDuration = 100; // How long a drip segment stays lit

  // Clear LEDs that have been on for too long from a drip
  for (int i = 0; i < numLeds; i++) {
    if (digitalRead(ledPins[i]) == HIGH && (currentMillis - ledTravelOnTime[i] >= dripOnDuration)) {
      digitalWrite(ledPins[i], LOW);
    }
  }

  // Bicolor "ripple" effect control
  const int bicolorRippleDuration = 150;
  static unsigned long bicolorRippleStartTime = 0;
  static bool bicolorRippleActive = false;


  // If no drip is active and it's time for a new one
  if (!dripDirection && currentMillis - lastDripTime >= newDripInterval) {
    if (random(0, 100) < newDripChance) { // Chance to start a new drip
      lastDripTime = currentMillis;
      activeDripLed = random(numLeds); // Starting point

      int targetOffset = random(2, 5); // Drip 2-4 steps away
      if (random(2) == 0) { // Randomly choose left or right
          dripDirection = 1; // Right
          dripTargetLed = constrain(activeDripLed + targetOffset, 0, numLeds - 1);
      } else {
          dripDirection = -1; // Left
          dripTargetLed = constrain(activeDripLed - targetOffset, 0, numLeds - 1);
      }

      lastLedTravelTime = currentMillis; // Start drip movement timer
      allBicolorOff(); // Ensure bicolors off before ripple
      setBicolor(1, true, true); setBicolor(2, true, true); // Flash bicolors briefly (Orange)
      bicolorRippleActive = true;
      bicolorRippleStartTime = currentMillis;
    }
  }

  // Execute drip movement
  if (dripDirection && currentMillis - lastLedTravelTime >= dripTravelSpeed) {
    lastLedTravelTime = currentMillis;

    digitalWrite(ledPins[activeDripLed], HIGH);
    ledTravelOnTime[activeDripLed] = currentMillis; // Set its turn-off timer

    if (activeDripLed == dripTargetLed) { // Reached destination
      dripDirection = 0; // Drip completed
      activeDripLed = -1;
      allLedsOff(); // Ensure trail is cleared eventually
    } else {
      activeDripLed += dripDirection;
    }
  }

  // Manage bicolor ripple
  if (bicolorRippleActive && currentMillis - bicolorRippleStartTime >= bicolorRippleDuration) {
    allBicolorOff(); // Turn off bicolors after ripple
    bicolorRippleActive = false;
  }
}
