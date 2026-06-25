// ============================================================
//  PSAI v2 — Pseudo-Sentient Artificial Intelligence
//  Depphen's Depot
//  Hardware: Arduino Nano + OttoDIYLib + HC-SR04 + Buzzer
// ============================================================

#include <Otto.h>
Otto Otto;

// --- Pin Definitions ---
#define LEFT_LEG    2
#define RIGHT_LEG   3
#define LEFT_FOOT   4
#define RIGHT_FOOT  5
#define BUZZER_PIN  13
#define TRIG_PIN    9
#define ECHO_PIN    8

// ============================================================
//  MOOD SYSTEM
//  moodValue: 0–100 integer, persists and drifts over time
//  Maps to 5 named mood bands
// ============================================================
int moodValue = 50;  // Start neutral

#define MOOD_MELANCHOLY  0   // 0–20
#define MOOD_ANXIOUS     1   // 21–40
#define MOOD_NEUTRAL     2   // 41–60
#define MOOD_HAPPY       3   // 61–80
#define MOOD_EUPHORIC    4   // 81–100

int getMood() {
  if (moodValue <= 20)  return MOOD_MELANCHOLY;
  if (moodValue <= 40)  return MOOD_ANXIOUS;
  if (moodValue <= 60)  return MOOD_NEUTRAL;
  if (moodValue <= 80)  return MOOD_HAPPY;
  return MOOD_EUPHORIC;
}

// Nudge mood up or down, clamped to 0–100
void shiftMood(int delta) {
  moodValue = constrain(moodValue + delta, 0, 100);
}

// ============================================================
//  TIMING — millis()-based, no blocking loop delays
// ============================================================
unsigned long lastActionTime    = 0;
unsigned long lastProximityTime = 0;
unsigned long lastMoodDriftTime = 0;
unsigned long lastSensorTime    = 0;

// How long Otto waits between idle actions (scales with boredom)
unsigned long idleInterval      = 4000;
unsigned long boredTimer        = 0;      // How long since last interaction
bool          interactionFlag   = false;  // True when someone is near

// Boredom level 0–3 (escalates idle drama)
int boredLevel = 0;

// ============================================================
//  ULTRASONIC SENSOR
// ============================================================
long prevDistance    = 999;
long currentDistance = 999;

long readDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  if (duration == 0) return 999; // No echo = nothing detected
  return duration / 58;
}

// Approach speed: negative = getting closer, positive = moving away
long approachSpeed() {
  return currentDistance - prevDistance;
}

// ============================================================
//  BUZZER — Melodic phrases per mood
//  Each is a short sequence of {frequency, duration_ms}
// ============================================================

// Melancholy: slow descending minor phrase
void singMelancholy() {
  int notes[] = {494, 440, 415, 392, 0};
  int durs[]  = {400, 400, 400, 600, 200};
  for (int i = 0; i < 5; i++) {
    if (notes[i] > 0) tone(BUZZER_PIN, notes[i], durs[i]);
    else noTone(BUZZER_PIN);
    delay(durs[i] + 50);
  }
  noTone(BUZZER_PIN);
}

// Anxious: rapid staccato, slightly dissonant
void singAnxious() {
  int notes[] = {523, 554, 523, 466, 523, 554, 466, 0};
  int durs[]  = {80,  80,  80,  80,  80,  80,  160, 100};
  for (int i = 0; i < 8; i++) {
    if (notes[i] > 0) tone(BUZZER_PIN, notes[i], durs[i]);
    else noTone(BUZZER_PIN);
    delay(durs[i] + 20);
  }
  noTone(BUZZER_PIN);
}

// Neutral: simple curious two-note question
void singNeutral() {
  tone(BUZZER_PIN, 440, 200); delay(250);
  tone(BUZZER_PIN, 494, 300); delay(350);
  noTone(BUZZER_PIN);
}

// Happy: ascending major arpeggio
void singHappy() {
  int notes[] = {262, 330, 392, 523, 392, 523};
  int durs[]  = {150, 150, 150, 300, 150, 400};
  for (int i = 0; i < 6; i++) {
    tone(BUZZER_PIN, notes[i], durs[i]);
    delay(durs[i] + 30);
  }
  noTone(BUZZER_PIN);
}

// Euphoric: fast chaotic celebratory burst
void singEuphoric() {
  int notes[] = {523, 659, 784, 1047, 784, 659, 523, 659, 784, 1047};
  int durs[]  = {100, 100, 100, 200,  100, 100, 100, 100, 100, 400};
  for (int i = 0; i < 10; i++) {
    tone(BUZZER_PIN, notes[i], durs[i]);
    delay(durs[i] + 20);
  }
  noTone(BUZZER_PIN);
}

// Startle sting
void singStartle() {
  tone(BUZZER_PIN, 800, 60);  delay(80);
  tone(BUZZER_PIN, 400, 120); delay(140);
  noTone(BUZZER_PIN);
}

// Sing based on current mood
void singMood() {
  switch (getMood()) {
    case MOOD_MELANCHOLY: singMelancholy(); break;
    case MOOD_ANXIOUS:    singAnxious();    break;
    case MOOD_NEUTRAL:    singNeutral();    break;
    case MOOD_HAPPY:      singHappy();      break;
    case MOOD_EUPHORIC:   singEuphoric();   break;
  }
}

// ============================================================
//  PROXIMITY REACTIONS — behavior varies by mood
// ============================================================
void reactToProximity() {
  interactionFlag = true;
  boredTimer      = 0;
  boredLevel      = 0;
  idleInterval    = 4000;

  bool approaching = approachSpeed() < -5; // Getting closer fast

  switch (getMood()) {

    case MOOD_MELANCHOLY:
      // Sad Otto shies away — backs up, sad sound
      singMelancholy();
      isWalking = true;
      Otto.walk(2, 1000, -1);   // Back away
      isWalking = false;
      Otto.playGesture(OttoSad);
      Otto.home();
      shiftMood(-3);
      break;

    case MOOD_ANXIOUS:
      // Anxious Otto startles hard, spins, freezes
      singStartle();
      Otto.playGesture(OttoFretful);
      if (approaching) {
        Otto.turn(2, 800, 1);
        Otto.turn(2, 800, -1);
      }
      Otto.home();
      singAnxious();
      shiftMood(-5);
      delay(500);
      shiftMood(3);
      break;

    case MOOD_NEUTRAL:
      // Neutral Otto is curious — leans in, investigates
      singNeutral();
      Otto.bend(1, 800, 1);
      Otto.playGesture(OttoConfused);
      Otto.home();
      if (approaching) {
        isWalking = true;
        Otto.walk(1, 1000, 1);
        isWalking = false;
        Otto.home();
      }
      shiftMood(4);
      break;

    case MOOD_HAPPY:
      // Happy Otto gets excited — approaches, celebrates
      singHappy();
      Otto.playGesture(OttoHappy);
      isWalking = true;
      Otto.walk(2, 800, 1);
      isWalking = false;
      Otto.home();
      Otto.swing(1, 600, 20);
      Otto.home();
      shiftMood(6);
      break;

    case MOOD_EUPHORIC:
      // Euphoric Otto loses its mind — full dance party
      singEuphoric();
      Otto.playGesture(OttoSuperHappy);
      isWalking = true;
      Otto.walk(2, 700, 1);
      isWalking = false;
      Otto.moonwalker(2, 800, 25, 1);
      Otto.home();
      Otto.jump(1, 800);
      Otto.home();
      singEuphoric();
      shiftMood(3);
      break;
  }
}

// ============================================================
//  IDLE BEHAVIORS — scaled by boredom level and mood
// ============================================================

// Small subtle idle: sway, look around, breathe
void idleSmall() {
  int r = random(1, 5);
  switch (r) {
    case 1: Otto.swing(1, 1200, 15); break;
    case 2: Otto.bend(1, 800, 1); delay(300); Otto.bend(1, 800, -1); break;
    case 3: Otto.jitter(1, 600, 10); break;
    case 4: Otto.tiptoeSwing(1, 1000, 15); break;
  }
  Otto.home();
}

// Medium idle: explore, look around, small emotional expression
void idleMedium() {
  int r = random(1, 6);
  switch (r) {
    case 1:
      Otto.turn(random(1,3), 900, 1);
      Otto.home();
      singMood();
      break;
    case 2:
      Otto.turn(random(1,3), 900, -1);
      Otto.home();
      break;
    case 3:
      isWalking = true;
      Otto.walk(random(1,3), 900, 1);
      Otto.home();
      Otto.walk(random(1,3), 900, -1);
      Otto.home();
      isWalking = false;
      break;
    case 4:
      singMood();
      Otto.updown(1, 800, 20);
      Otto.home();
      break;
    case 5:
      Otto.shakeLeg(1, 900, random(0,1) == 0 ? 1 : -1);
      Otto.home();
      break;
  }
}

// Large idle: dramatic attention-seeking behavior
void idleLarge() {
  singMood();
  int r = random(1, 7);
  switch (r) {
    case 1:
      Otto.playGesture(OttoVictory);
      Otto.home();
      break;
    case 2:
      Otto.moonwalker(2, 900, 25, 1);
      Otto.moonwalker(2, 900, 25, -1);
      Otto.home();
      break;
    case 3:
      Otto.crusaito(2, 900, 25, 1);
      Otto.home();
      singMood();
      break;
    case 4:
      Otto.flapping(2, 800, 25, 1);
      Otto.flapping(2, 800, 25, -1);
      Otto.home();
      break;
    case 5:
      Otto.ascendingTurn(2, 900, 25);
      Otto.home();
      break;
    case 6:
      Otto.playGesture(OttoFart); // Everyone loves this one
      Otto.home();
      break;
  }
}

// Mood-tinted idle — emotional color on top of boredom scale
void doIdleAction() {
  // Mood biases which pool to draw from
  int mood = getMood();

  // Boredom level picks drama level, but mood can override
  if (boredLevel == 0) {
    idleSmall();
  } else if (boredLevel == 1) {
    // Anxious/melancholy stay small even when bored
    if (mood == MOOD_MELANCHOLY || mood == MOOD_ANXIOUS) {
      idleSmall();
    } else {
      idleMedium();
    }
  } else if (boredLevel == 2) {
    if (mood == MOOD_MELANCHOLY) {
      // Melancholy and very bored: just play a sad sound, barely move
      singMelancholy();
      Otto.bend(1, 1000, -1);
      Otto.home();
    } else {
      idleMedium();
    }
  } else {
    // boredLevel 3: maximum drama regardless (even sad Otto snaps eventually)
    idleLarge();
  }

  // Mood drift from idle — melancholy deepens without interaction
  if (mood == MOOD_MELANCHOLY) shiftMood(-1);
  // Happy Otto self-sustains a little
  if (mood == MOOD_HAPPY || mood == MOOD_EUPHORIC) shiftMood(1);
}

// ============================================================
//  MOOD DRIFT — slow natural evolution over time
//  Called every ~10 seconds regardless of interaction
// ============================================================
void driftMood() {
  int mood = getMood();

  // Isolation drift: without interaction, mood slowly sinks toward anxious
  if (!interactionFlag) {
    if (mood == MOOD_EUPHORIC) shiftMood(-2);
    else if (mood == MOOD_HAPPY) shiftMood(-1);
    else if (mood == MOOD_NEUTRAL) {
      // Coin flip — drift toward anxious or stay
      if (random(0, 2) == 0) shiftMood(-2);
    }
    // Anxious and melancholy can slowly recover on their own (resilience)
    else if (mood == MOOD_ANXIOUS) {
      if (random(0, 3) == 0) shiftMood(3);
    }
    else if (mood == MOOD_MELANCHOLY) {
      if (random(0, 5) == 0) shiftMood(4); // Rare but it happens
    }
  }

  // Random "intrusive thought" — small unpredictable nudge
  // This keeps the system from getting totally stuck
  int intrusiveThought = random(0, 10);
  if (intrusiveThought == 0) shiftMood(random(3, 8));
  if (intrusiveThought == 1) shiftMood(random(-8, -3));

  // Reset interaction flag — it resets each drift cycle
  interactionFlag = false;
}

// ============================================================
//  OBSTACLE AVOIDANCE
//  Separate from mood/social system — pure safety layer
//  Triggers only when something is within 10cm for 1 second
//  while Otto is walking
// ============================================================
bool          isWalking         = false;  // True when Otto is mid-walk
unsigned long wallDetectedTime  = 0;      // When we first saw something < 10cm
bool          wallTimerActive   = false;  // Is the wall timer running

#define WALL_DISTANCE     10    // cm — closer than this = wall
#define WALL_CONFIRM_MS   1000  // ms — must be detected this long to confirm wall

void avoidWall() {
  // Stop, back up, turn a random amount between 90 and 180 degrees
  tone(BUZZER_PIN, 300, 200); delay(250); // Low thud — "oof"
  noTone(BUZZER_PIN);
  Otto.walk(2, 900, -1);                          // Back up
  Otto.home();
  int turnSteps = random(2, 5);                   // 90–180 degrees roughly
  int turnDir   = random(0, 2) == 0 ? 1 : -1;    // Random left or right
  Otto.turn(turnSteps, 800, turnDir);
  Otto.home();
  isWalking       = false;
  wallTimerActive = false;
  wallDetectedTime = 0;
}
void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Otto.init(LEFT_LEG, RIGHT_LEG, LEFT_FOOT, RIGHT_FOOT, true, BUZZER_PIN);
  Otto.home();

  randomSeed(analogRead(0)); // True random seed from floating analog pin

  // Boot sequence — Otto wakes up
  delay(500);
  singNeutral();
  Otto.playGesture(OttoSuperHappy);
  Otto.home();
  delay(random(1000, 3000)); // Random pause before first action

  lastActionTime    = millis();
  lastMoodDriftTime = millis();
  lastSensorTime    = millis();
  boredTimer        = millis();
}

// ============================================================
//  MAIN LOOP
// ============================================================
void loop() {
  unsigned long now = millis();

  // --- Read sensor every 200ms (non-blocking) ---
  if (now - lastSensorTime >= 200) {
    prevDistance    = currentDistance;
    currentDistance = readDistance();
    lastSensorTime  = now;
  }

  // --- Wall avoidance: only triggers while walking, under 10cm for 1 second ---
  if (isWalking && currentDistance > 0 && currentDistance < WALL_DISTANCE) {
    if (!wallTimerActive) {
      wallTimerActive  = true;
      wallDetectedTime = now;
    } else if (now - wallDetectedTime >= WALL_CONFIRM_MS) {
      avoidWall();
    }
  } else {
    // Clear wall timer if obstacle gone or not walking
    wallTimerActive  = false;
    wallDetectedTime = 0;
  }

  // --- Proximity check: something within 80cm ---
  if (currentDistance < 80 && currentDistance > 0) {
    if (now - lastProximityTime >= 3000) { // Don't re-trigger for 3s
      reactToProximity();
      lastProximityTime = now;
      lastActionTime    = now; // Reset idle timer after reaction
    }
  }

  // --- Mood drift every 10 seconds ---
  if (now - lastMoodDriftTime >= 10000) {
    driftMood();
    lastMoodDriftTime = now;
  }

  // --- Boredom escalation ---
  unsigned long boredElapsed = now - boredTimer;
  if      (boredElapsed < 15000)  boredLevel = 0; // < 15s: calm
  else if (boredElapsed < 45000)  boredLevel = 1; // 15–45s: a little restless
  else if (boredElapsed < 90000)  boredLevel = 2; // 45–90s: noticeably bored
  else                            boredLevel = 3; // 90s+: dramatic

  // Boredom also affects idle interval — more bored = more frequent fidgeting
  if      (boredLevel == 0) idleInterval = random(4000, 8000);
  else if (boredLevel == 1) idleInterval = random(2500, 5000);
  else if (boredLevel == 2) idleInterval = random(1500, 3000);
  else                      idleInterval = random(800,  1800);

  // --- Idle action ---
  if (now - lastActionTime >= idleInterval) {
    doIdleAction();
    lastActionTime = now;
  }
}
