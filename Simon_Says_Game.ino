
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <avr/wdt.h>

// LCD: common I2C address is 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Buzzer
const int buzzerPin = 4;

// LEDs
const int leds[] = {A3, A2, A1, A0};

// Buttons: white, yellow, blue, red
const int buttons[] = {7, 8, 9, 10};

// Notes for each color
const int notes[] = {262, 330, 392, 523};

const int NUM_COLORS = 4;
const int MAX_LEVEL = 15;

int flashDelay = 350;
int winDelay = 5000;
int counting = 1000;

int sequence[MAX_LEVEL];
int level = 1;

bool cheat = false;

void playVictoryMusic() {
  // Melody notes: C5, E5, G5, C6, G5, C6
  int victoryNotes[] = {523, 659, 784, 1047, 784, 1047};
  int noteDurations[] = {150, 150, 150, 300, 150, 600};

  for (int i = 0; i < 6; i++) {
    // Flash all LEDs along with the notes
    for (int j = 0; j < NUM_COLORS; j++) {
      digitalWrite(leds[j], HIGH);
    }
    
    tone(buzzerPin, victoryNotes[i]);
    delay(noteDurations[i]);
    noTone(buzzerPin);
    
    for (int j = 0; j < NUM_COLORS; j++) {
      digitalWrite(leds[j], LOW);
    }
    delay(50); // Small gap between notes
  }
}

void playColor(int color) {
  digitalWrite(leds[color], HIGH);
  tone(buzzerPin, notes[color]);
  delay(flashDelay);
  digitalWrite(leds[color], LOW);
  noTone(buzzerPin);
  delay(150);
  flashDelay = 350;
  flashDelay = flashDelay - (level * 20);
  if (flashDelay < 50) {
    flashDelay = 50;
  }
}

void reset() {
  wdt_enable(WDTO_15MS); // Enable watchdog timer with a 15ms timeout
  while (1);
}

void waitForRelease(int buttonPin) {
  while (digitalRead(buttonPin) == LOW) {
    delay(10);
  }
  delay(30);
}

int getButtonPress() {
  while (true) {
    for (int i = 0; i < NUM_COLORS; i++) {
      if (digitalRead(buttons[i]) == LOW) {
        delay(25); // debounce
        if (digitalRead(buttons[i]) == LOW) {
          playColor(i);
          waitForRelease(buttons[i]);
          return i;
        }
      }
    }
  }
}

void gameOver() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GAME OVER!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(level - 1);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < NUM_COLORS; j++) {
      digitalWrite(leds[j], HIGH);
    }
    tone(buzzerPin, 180);
    delay(250);

    for (int j = 0; j < NUM_COLORS; j++) {
      digitalWrite(leds[j], LOW);
    }
    noTone(buzzerPin);
    delay(200);
  }

  int wButton = digitalRead(buttons[0]);
  int yButton = digitalRead(buttons[1]);
  int bButton = digitalRead(buttons[2]);
  int rButton = digitalRead(buttons[3]);
  if (wButton == LOW && yButton == LOW && bButton == LOW && rButton == LOW) {
    lcd.clear();
    lcd.home();
    lcd.print("CHEAT MODE   ");
    lcd.setCursor(6, 1); 
    lcd.print("ACTIVATED");
    cheat = true;
  }
  delay(1800);
}

void setup() {
  for (int i = 0; i < NUM_COLORS; i++) {
    pinMode(leds[i], OUTPUT);
    pinMode(buttons[i], INPUT_PULLUP);
  }

  pinMode(buzzerPin, OUTPUT);

  lcd.init();
  lcd.backlight();

  randomSeed(analogRead(A5));

  lcd.setCursor(0, 0);
  lcd.print("SIMON SAYS!");
  lcd.setCursor(0, 1);
  lcd.print("Get ready...");
  delay(1500);
}

void loop() {
   if (cheat) {
    level = MAX_LEVEL;
    // Generate valid random moves for the ENTIRE sequence history so it's playable
    for (int i = 0; i < MAX_LEVEL; i++) {
      sequence[i] = random(NUM_COLORS);
    }
    cheat = false;
  } else {
    // Standard single-step progression
    sequence[level - 1] = random(NUM_COLORS);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Level: ");
  lcd.print(level);
  lcd.setCursor(0, 1);
  lcd.print("Watch carefully");

  delay(800);

  // Show the entire sequence so far
  for (int i = 0; i < level; i++) {
    playColor(sequence[i]);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Your turn!");
  lcd.setCursor(0, 1);
  lcd.print("Level: ");
  lcd.print(level);

  // Check the player's sequence
  for (int i = 0; i < level; i++) {
    int pressed = getButtonPress();

    if (pressed != sequence[i]) {
      gameOver();
      level = 1;
      delay(500);
      return;
    }
  }

  // Successful round!
  tone(buzzerPin, 880);
  delay(100);
  noTone(buzzerPin);

  level++;
  if (level > MAX_LEVEL) {
    level = 1;
    lcd.clear();
    lcd.home();
    lcd.print("Wow... You beat  ");
    lcd.setCursor(2, 1);
    lcd.print("THE GAME!!!");
    for (int p = 0; p < 5; p++) {
      playVictoryMusic();
    }
    delay(winDelay);
    lcd.clear();
    lcd.home();
    lcd.print("Resetting in... ");
    for (int k = 5; k > 0; k--) {
      lcd.setCursor(0, 1);
      lcd.print(k);
      delay(counting);
    }
    reset();
  }

  delay(400);
}
