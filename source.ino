#include <Wire.h>

#define HT_ADDR 0x70

// Segmenty liter - dopasowane do 7-seg
uint8_t segBlank = 0x00;
uint8_t segA = 0b01110111;  // A
uint8_t segC = 0b00111001;  // C
uint8_t segD = 0b01011110;  // D (d małe)
uint8_t segE = 0b01111001;  // E
uint8_t segG = 0b01111101;  // G

// Inicjalizacja HT16K33
void initDisplay() {
  Wire.begin();

  Wire.beginTransmission(HT_ADDR);
  Wire.write(0x21);     // włącz oscylator
  Wire.endTransmission();

  Wire.beginTransmission(HT_ADDR);
  Wire.write(0x81);     // włącz wyświetlacz
  Wire.endTransmission();

  Wire.beginTransmission(HT_ADDR);
  Wire.write(0xEF);     // jasność max
  Wire.endTransmission();
}

// Wyslanie segmentów na 4 cyfry
void sendSegments(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
  Wire.beginTransmission(HT_ADDR);
  Wire.write(0x00); // adres RAM

  Wire.write(d0); Wire.write(0x00);
  Wire.write(d1); Wire.write(0x00);
  Wire.write(d2); Wire.write(0x00);
  Wire.write(d3); Wire.write(0x00);

  Wire.endTransmission();
}

// pokazanie litery nuty
void showNoteChar(char n) {
  uint8_t s;

  switch(n) {
    case 'A': s = segA; break;
    case 'C': s = segC; break;
    case 'D': s = segD; break;
    case 'E': s = segE; break;
    case 'G': s = segG; break;
    default:  s = segBlank;
  }

  sendSegments(s, segBlank, segBlank, segBlank);
}

// Piny przycisków nut
int cNote = 9;  // C
int dNote = 8;  // D
int eNote = 7;  // E
int gNote = 6;  // G
int aNote = 5;  // A

// Piny kontrolne
int Piezo = 2;
int recordButton = A3;  // Przycisk RECORD
int playButton = A2;    // Przycisk PLAY
int redLED = 10;        // Czerwona dioda - nagrywanie
int greenLED = 11;      // Zielona dioda - odtwarzanie

// Częstotliwości nut
double c = 261.63;
double d = 293.66;
double e = 329.63;
double g = 392;
double a = 440;

// Struktura do przechowywania melodii
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
  // Przyciski nut
  pinMode(cNote, INPUT);
  pinMode(dNote, INPUT);
  pinMode(eNote, INPUT);
  pinMode(gNote, INPUT);
  pinMode(aNote, INPUT);
  
  // Przyciski kontrolne
  pinMode(recordButton, INPUT_PULLUP);
  pinMode(playButton, INPUT_PULLUP);

  // LEDy
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  pinMode(Piezo, OUTPUT);

  Serial.begin(9600);

  initDisplay();
  showNoteChar(' '); // na start pusto

  Serial.println("Piano gotowe!");
}

void loop() {
  // Obsługa przycisku RECORD
  if (digitalRead(recordButton) == LOW && !recordButtonPressed) {
    recordButtonPressed = true;
    isRecording = !isRecording;
    
    if (isRecording) {
      noteCount = 0;
      digitalWrite(redLED, HIGH);  // Zapal czerwoną diodę
      Serial.println("*** NAGRYWANIE ROZPOCZETE ***");
    } else {
      digitalWrite(redLED, LOW);   // Zgaś czerwoną diodę
      Serial.print("*** NAGRYWANIE ZAKONCZONE - zapisano ");
      Serial.print(noteCount);
      Serial.println(" nut ***");
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

  // ---------------- NUTY ----------------
  double freq = 0;
  char noteChar = ' ';

  if (digitalRead(cNote) == 1) {
    freq = c; noteChar = 'C';
    Serial.println("Nacisnieto przycisk: C");
  } else if (digitalRead(dNote) == 1) {
    freq = d; noteChar = 'D';
    Serial.println("Nacisnieto przycisk: D");
  } else if (digitalRead(eNote) == 1) {
    freq = e; noteChar = 'E';
    Serial.println("Nacisnieto przycisk: E");
  } else if (digitalRead(gNote) == 1) {
    freq = g; noteChar = 'G';
    Serial.println("Nacisnieto przycisk: G");
  } else if (digitalRead(aNote) == 1) {
    freq = a; noteChar = 'A';
    Serial.println("Nacisnieto przycisk: A");
  }

  // pokaz nutę na wyświetlaczu
  showNoteChar(noteChar);

  // dźwięk
  if (freq > 0) {
    tone(Piezo, freq);
    
    // Nagrywanie - rozpoczęcie nowej nuty
    if (isRecording && freq != currentFrequency) {
      if (currentFrequency > 0 && noteCount < MAX_NOTES) {
        // Zapisz poprzednią nutę
        melody[noteCount].frequency = currentFrequency;
        melody[noteCount].duration = millis() - noteStartTime;
        noteCount++;
      }
      currentFrequency = freq;
      noteStartTime = millis();
    }
  } else {
    noTone(Piezo);
    
    // Nagrywanie - zakończenie nuty
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
    Serial.println("Brak nagranej melodii!");
    // Sygnał dźwiękowy
    tone(Piezo, 200, 100);
    delay(150);
    tone(Piezo, 200, 100);
    return;
  }
  
  Serial.println("*** ODTWARZANIE (Tempo 0.5s) ***");
  digitalWrite(greenLED, HIGH);  // Zapal zieloną diodę
  noTone(Piezo);
  delay(500);
  
  for (int i = 0; i < noteCount; i++) {
    char noteChar = ' ';

    if (melody[i].frequency == c) noteChar = 'C';
    if (melody[i].frequency == d) noteChar = 'D';
    if (melody[i].frequency == e) noteChar = 'E';
    if (melody[i].frequency == g) noteChar = 'G';
    if (melody[i].frequency == a) noteChar = 'A';

    showNoteChar(noteChar);

    tone(Piezo, melody[i].frequency);
    delay(400);
    noTone(Piezo);
    delay(100);
  }

  digitalWrite(greenLED, LOW);
  showNoteChar(' ');
}
