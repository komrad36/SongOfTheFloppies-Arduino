// By Kareem Omar (komrad36)
// Update I/O pins in the setup() function
// and the constants below as you see fit.

#define MAX_STEPS                  (80)
#define NUM_DRIVES                 (15)
#define FREQ_MULTIPLIER            (10000.0)
#define BAUD_RATE                  (250000)
#define MESSAGE_SIZE_BYTES         (4)
#define DELAY_MS                   (3)
#define CALIBRATE_STEPS            (MAX_STEPS+4)
#define MICROSECONDS_PER_SECOND    (1000000.0)
#define LED_ON_US                  (1000)

byte          directionPin    [NUM_DRIVES];
byte          wavePin         [NUM_DRIVES];
unsigned long nextPulseTime   [NUM_DRIVES];
unsigned long nextLEDoffTime  [NUM_DRIVES];
byte          steps           [NUM_DRIVES];
bool          dir             [NUM_DRIVES];
float         freq            [NUM_DRIVES];

byte          curDrive;
unsigned long convertedFreq;

void pulse(byte floppy) {
  // if at end of travel...
  if (steps[floppy] > MAX_STEPS) {
    // ...reverse direction and reset
    // step counter
    dir[floppy] = !dir[floppy];
    digitalWrite(directionPin[floppy], dir[floppy]);
    steps[floppy] = 0;
  }

  if (nextLEDoffTime[floppy])
    digitalWrite(wavePin[floppy], HIGH);

  // floppy stepper motors advance 1 step
  // on a rising edge.
  // leave it low to keep shiny lights on
  digitalWrite(wavePin[floppy], LOW);
  ++steps[floppy];
}

void setup() {
  Serial.begin(BAUD_RATE);

  directionPin[7]   = 4;
  directionPin[1]   = 10;
  directionPin[6]   = 35;
  directionPin[12]  = 45;
  directionPin[11]  = 33;
  directionPin[10]  = 47;
  directionPin[0]   = 31;
  directionPin[4]   = 49;
  directionPin[8]   = 29;
  directionPin[14]  = 6;
  directionPin[5]   = 27;
  directionPin[3]   = 8;
  directionPin[13]  = 23;
  directionPin[9]   = 12;
  directionPin[2]   = 43;
  wavePin[7]        = 3;
  wavePin[1]        = 9;
  wavePin[6]        = 34;
  wavePin[12]       = 44;
  wavePin[11]       = 32;
  wavePin[10]       = 46;
  wavePin[0]        = 30;
  wavePin[4]        = 48;
  wavePin[8]        = 28;
  wavePin[14]       = 5;
  wavePin[5]        = 26;
  wavePin[3]        = 7;
  wavePin[13]       = 22;
  wavePin[9]        = 11;
  wavePin[2]        = 42;

  byte i, j;

  for (i = 0; i < NUM_DRIVES; ++i) {
    steps[i] = 0;
    nextLEDoffTime[i] = 0UL;
    pinMode(wavePin[i], OUTPUT);
    pinMode(directionPin[i], OUTPUT);
    digitalWrite(directionPin[i], HIGH);
  }

  for (i = 0; i < CALIBRATE_STEPS; ++i) {
    for (j = 0; j < NUM_DRIVES; ++j) {
      digitalWrite(wavePin[j], HIGH);
      digitalWrite(wavePin[j], LOW);
    }
    delay(DELAY_MS);
  }

  for (i = 0; i < NUM_DRIVES; ++i) {
    digitalWrite(directionPin[i], LOW);
  }
  for (i = 0; i < CALIBRATE_STEPS; ++i) {
    for (j = 0; j < NUM_DRIVES; ++j) {
      digitalWrite(wavePin[j], HIGH);
      digitalWrite(wavePin[j], LOW);
    }
    delay(DELAY_MS);
  }

  for (i = 0; i < NUM_DRIVES; ++i) {
    dir[i] = HIGH;
    digitalWrite(wavePin[i], HIGH);
    digitalWrite(directionPin[i], HIGH);
  }

  // signal ready
  Serial.write(0);
}

void loop() {
  if (Serial.available() >= MESSAGE_SIZE_BYTES) {
    // byte format: first byte is drive, 0-indexed
    // next 3 bytes are requested frequency of note,
    // times 10000 so it can be sent as integer type
    curDrive = Serial.read();
    convertedFreq = (unsigned long)Serial.read() + ((unsigned long)Serial.read() << 8) + ((unsigned long)Serial.read() << 16);

    // a requested freq of 0
    // is used to mean "Note OFF"
    if (convertedFreq == 0) {
      nextPulseTime[curDrive] = 0;
    }
    else {
      freq[curDrive] = (float)(convertedFreq) / FREQ_MULTIPLIER;
      if (!nextPulseTime[curDrive]) nextPulseTime[curDrive] = micros();
      nextLEDoffTime[curDrive] = nextPulseTime[curDrive] + LED_ON_US;
    }
  }

  // for each drive, check if it's at or past time
  // to produce the next pulse in the square wave
  for (byte i = 0; i < NUM_DRIVES; ++i) {
    if (nextPulseTime[i] && micros() >= nextPulseTime[i]) {
      // if it is, update the next pulse time accordingly
      pulse(i);
      nextPulseTime[i] += MICROSECONDS_PER_SECOND / freq[i];
      nextLEDoffTime[i] = nextPulseTime[i] + LED_ON_US;
    }
    
    if (nextLEDoffTime[i] && micros() >= nextLEDoffTime[i]) {
      // if it's time to turn an LED off, do so by writing high
      // and disable the next LED off time
      digitalWrite(wavePin[i], HIGH);
      nextLEDoffTime[i] = 0;
    }
  }
}
