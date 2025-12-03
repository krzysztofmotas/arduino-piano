#include <Wire.h>

#define HT_ADDR 0x70

// Segments for letters - mapped to 7-segment display
uint8_t segBlank = 0x00;
uint8_t segC = 0b00111001;  // C
uint8_t segD = 0b01011110;  // D (lowercase)
uint8_t segE = 0b01111001;  // E
uint8_t segF = 0b01110001;  // F
uint8_t segG = 0b01111101;  // G

// Initialize HT16K33
void initDisplay() {
  Wire.begin();

  Wire.beginTransmission(HT_ADDR);
  Wire.write(0x21);     // enable oscillator
  Wire.endTransmission();

  Wire.beginTransmission(HT_ADDR);
  Wire.write(0x81);     // enable display
  Wire.endTransmission();

  Wire.beginTransmission(HT_ADDR);
  Wire.write(0xEF);     // max brightness
  Wire.endTransmission();
}

// Send segments to 4 digits
void sendSegments(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
  Wire.beginTransmission(HT_ADDR);
  Wire.write(0x00); // RAM address

  Wire.write(d0); Wire.write(0x00);
  Wire.write(d1); Wire.write(0x00);
  Wire.write(d2); Wire.write(0x00);
  Wire.write(d3); Wire.write(0x00);

  Wire.endTransmission();
}

// Show note letter
void showNoteChar(char n) {
  uint8_t s;

  switch(n) {
    case 'C': s = segC; break;
    case 'D': s = segD; break;
    case 'E': s = segE; break;
    case 'F': s = segF; break;
    case 'G': s = segG; break;
    default:  s = segBlank;
  }

  sendSegments(s, segBlank, segBlank, segBlank);
}

// Note button pins
int cNote = 9;  // C
int dNote = 8;  // D
int eNote = 7;  // E
int fNote = 6;  // F
int gNote = 5;  // G

// Control pins
int Piezo = 2;
int recordButton = A3;  // RECORD button
int playButton = A2;    // PLAY button
int redLED = 10;        // Red LED - recording
int greenLED = 11;      // Green LED - playback

// Note frequencies
double c = 261.63;
double d = 293.66;
double e = 329.63;
double f = 349.23;
double g = 392;
double a = 440;

// Structure for storing melody
#define MAX_NOTES 100
struct Note {
  double frequency;
  unsigned long duration;
};

Note melody[MAX_NOTES];
int noteCount = 0;
bool isRecording = false;
bool recordButtonPressed = false;
bool playButtonPressed = false;
unsigned long noteStartTime = 0;
double currentFrequency = 0;

void setup() {
  // Note buttons
  pinMode(cNote, INPUT);
  pinMode(dNote, INPUT);
  pinMode(eNote, INPUT);
  pinMode(fNote, INPUT);
  pinMode(gNote, INPUT);
  
  // Control buttons
  pinMode(recordButton, INPUT_PULLUP);
  pinMode(playButton, INPUT_PULLUP);

  // LEDs
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  pinMode(Piezo, OUTPUT);

  Serial.begin(9600);

  initDisplay();
  showNoteChar(' '); // blank on start

  Serial.println("Piano ready!");
}

void loop() {
  // Handle RECORD button
  if (digitalRead(recordButton) == LOW && !recordButtonPressed) {
    recordButtonPressed = true;
    isRecording = !isRecording;
    
    if (isRecording) {
      noteCount = 0;
      digitalWrite(redLED, HIGH);  // Turn on red LED
      Serial.println("*** RECORDING STARTED ***");
    } else {
      digitalWrite(redLED, LOW);   // Turn off red LED
      Serial.print("*** RECORDING FINISHED - saved ");
      Serial.print(noteCount);
      Serial.println(" notes ***");
    }
    delay(200);
  }
  if (digitalRead(recordButton) == HIGH) recordButtonPressed = false;

  // ---------------- PLAY ----------------
  if (digitalRead(playButton) == LOW && !playButtonPressed) {
    playButtonPressed = true;
    playMelody();
    delay(200);
  }
  if (digitalRead(playButton) == HIGH) playButtonPressed = false;

  // ---------------- NOTES ----------------
  double freq = 0;
  char noteChar = ' ';

  if (digitalRead(cNote) == 1) {
    freq = c; noteChar = 'C';
    Serial.println("Button pressed: C");
  } else if (digitalRead(dNote) == 1) {
    freq = d; noteChar = 'D';
    Serial.println("Button pressed: D");
  } else if (digitalRead(eNote) == 1) {
    freq = e; noteChar = 'E';
    Serial.println("Button pressed: E");
  } else if (digitalRead(fNote) == 1) {
    freq = f; noteChar = 'F';
    Serial.println("Button pressed: F");
  } else if (digitalRead(gNote) == 1) {
    freq = g; noteChar = 'G';
    Serial.println("Button pressed: G");
  }

  // Show note on display
  showNoteChar(noteChar);

  // Sound
  if (freq > 0) {
    tone(Piezo, freq);
    
    // Recording - start new note
    if (isRecording && freq != currentFrequency) {
      if (currentFrequency > 0 && noteCount < MAX_NOTES) {
        // Save previous note
        melody[noteCount].frequency = currentFrequency;
        melody[noteCount].duration = millis() - noteStartTime;
        noteCount++;
      }
      currentFrequency = freq;
      noteStartTime = millis();
    }
  } else {
    noTone(Piezo);
    
    // Recording - end note
    if (isRecording && currentFrequency > 0 && noteCount < MAX_NOTES) {
      melody[noteCount].frequency = currentFrequency;
      melody[noteCount].duration = millis() - noteStartTime;
      noteCount++;
      currentFrequency = 0;
    }
  }
  
  delay(10);
}

void playMelody() {
  if (noteCount == 0) {
    Serial.println("No recorded melody!");
    // Audio signal
    tone(Piezo, 200, 100);
    delay(150);
    tone(Piezo, 200, 100);
    return;
  }
  
  Serial.println("*** PLAYBACK (Tempo 0.5s) ***");
  digitalWrite(greenLED, HIGH);  // Turn on green LED
  noTone(Piezo);
  delay(500);
  
  for (int i = 0; i < noteCount; i++) {
    char noteChar = ' ';

    if (melody[i].frequency == c) noteChar = 'C';
    if (melody[i].frequency == d) noteChar = 'D';
    if (melody[i].frequency == e) noteChar = 'E';
    if (melody[i].frequency == f) noteChar = 'F';
    if (melody[i].frequency == g) noteChar = 'G';

    showNoteChar(noteChar);

    tone(Piezo, melody[i].frequency);
    delay(400);
    noTone(Piezo);
    delay(100);
  }

  digitalWrite(greenLED, LOW);
  showNoteChar(' ');
}
