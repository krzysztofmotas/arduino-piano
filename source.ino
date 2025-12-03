// Piny przycisków nut
int cNote = 9;  // C
int dNote = 8;  // D
int eNote = 7;  // E
int gNote = 6;  // G
int aNote = 5;  // A

// Piny kontrolne
int Piezo = 2;
int recordButton = A3;  // Przycisk RECORD
int playButton = A4;    // Przycisk PLAY
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

void setup()
{
  // Przyciski nut
  pinMode(cNote, INPUT);
  pinMode(dNote, INPUT);
  pinMode(eNote, INPUT);
  pinMode(gNote, INPUT);
  pinMode(aNote, INPUT);
  
  // Przyciski kontrolne
  pinMode(recordButton, INPUT_PULLUP);
  pinMode(playButton, INPUT_PULLUP);
  
  // Diody LED
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  digitalWrite(redLED, LOW);    // Wyłącz na start
  digitalWrite(greenLED, LOW);  // Wyłącz na start
  
  pinMode(Piezo, OUTPUT);
  Serial.begin(9600);
  Serial.println("Piano (5 nut) gotowe!");
  Serial.println("A3 = RECORD, A4 = PLAY");
}

void loop()
{
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
    delay(200); // debounce
  }
  if (digitalRead(recordButton) == HIGH) {
    recordButtonPressed = false;
  }
  
  // Obsługa przycisku PLAY
  if (digitalRead(playButton) == LOW && !playButtonPressed) {
    playButtonPressed = true;
    playMelody();
    delay(200); // debounce
  }
  if (digitalRead(playButton) == HIGH) {
    playButtonPressed = false;
  }
  
  // Odczyt aktualnie granej nuty
  double freq = 0;
  
  if (digitalRead(cNote) == 1) {
    freq = c;
  } else if (digitalRead(dNote) == 1) {
    freq = d;
  } else if (digitalRead(eNote) == 1) {
    freq = e;
  } else if (digitalRead(gNote) == 1) {
    freq = g;
  } else if (digitalRead(aNote) == 1) {
    freq = a;
  }
  
  // Odtwarzanie dźwięku
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
    if (melody[i].frequency > 0) {
      tone(Piezo, melody[i].frequency);
      // Sztywne tempo 0.5s na nutę (400ms grania + 100ms przerwy)
      delay(400);
      noTone(Piezo);
      delay(100);
    }
  }
  
  noTone(Piezo);
  digitalWrite(greenLED, LOW);   // Zgaś zieloną diodę
  Serial.println("*** KONIEC ODTWARZANIA ***");
}