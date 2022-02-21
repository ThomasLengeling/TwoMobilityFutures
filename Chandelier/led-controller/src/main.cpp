#include <Arduino.h>
#include <jled.h>

enum lightMode {
  ON = 1,
  OFF = 2,
  BREATH = 3,
  FLICKER = 4,
  STROBE = 5,
  RANDOM_STROBE = 6,
  FADE_ON = 7,
  FADE_OFF = 8,
  FADE_ON_SEQ = 9,
  FADE_OFF_SEQ = 10
};

// serial communication
char* handshakeMessage = "s";
const size_t BUFFER_SIZE = 8;
char line[BUFFER_SIZE];
int baudRate = 115200;

// leds
lightMode currentMode = OFF;
const int LED_COUNT = 16;

int teensyPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 29, 30, 23, 22, 21, 20, 17};

// re-order pins based on led orientation
int pins[] = {teensyPins[8], teensyPins[4],  teensyPins[1],  teensyPins[14], teensyPins[9],
              teensyPins[6], teensyPins[3],  teensyPins[12], teensyPins[10], teensyPins[7],
              teensyPins[0], teensyPins[13], teensyPins[11], teensyPins[15], teensyPins[5], teensyPins[2]
             };

// example orientation pins
int northPins[] = {1, 2, 3, 4};
// etc, ...

// jled config
JLed led1 = JLed(pins[0]);
JLed led2 = JLed(pins[1]);
JLed led3 = JLed(pins[2]);
JLed led4 = JLed(pins[3]);
JLed led5 = JLed(pins[4]);
JLed led6 = JLed(pins[5]);
JLed led7 = JLed(pins[6]);
JLed led8 = JLed(pins[7]);
JLed led9 = JLed(pins[8]);
JLed led10 = JLed(pins[9]);
JLed led11 = JLed(pins[10]);
JLed led12 = JLed(pins[11]);
JLed led13 = JLed(pins[12]);
JLed led14 = JLed(pins[13]);
JLed led15 = JLed(pins[14]);
JLed led16 = JLed(pins[15]);

JLed leds[] = {led1, led2, led3, led4, led5, led6, led7, led8, led9, led10, led11, led12, led13, led14, led15, led16};
auto sequence = JLedSequence(JLedSequence::eMode::PARALLEL, leds);

/*
  LED Effects
*/

void startOn(bool isSequential = false) {
  if (isSequential == true) {
    int delay = 500;
    for (int i = 0; i < LED_COUNT; i++) {
      int ledDelay = delay * i;
      leds[i].On().DelayBefore(ledDelay);
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      leds[i].On();
    }
  }
}

void startOff(bool isSequential = false) {
  if (isSequential == true) {
    int delay = 1000;
    for (int i = 0; i < LED_COUNT; i++) {
      int ledDelay = delay * i;
      leds[i].Off().DelayBefore(ledDelay);
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      leds[i].Off();
    }
  }
}

void startBreath(int speed = 5000) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].Breathe(speed).Forever();
  }
}

void startFlicker(int speed = 1000) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].Candle(5, 255, speed).Forever();
  }
}

void startStrobe(int speed = 25) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].Blink(speed, speed).Forever();
  }
}

void startRandomStrobe() {
  for (int i = 0; i < LED_COUNT; i++) {
    long ledOn = random(0, 2);
    Serial.println(ledOn);
    if (ledOn == 1)
      leds[i].On().Forever();
    else
      leds[i].Off().Forever();
  }
}

void startFadeOn(bool isSequential = false, int speed = 1000) {
  if (isSequential == true) {
    int delay = 200;
    for (int i = 0; i < LED_COUNT; i++) {
      int ledDelay = delay * i;
      leds[i].FadeOn(speed).DelayBefore(ledDelay);
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      leds[i].FadeOn(speed);
    }
  }
}

void startFadeOff(bool isSequential = false, int speed = 1000) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].On();
    leds[i].Update();
  }
  if (isSequential == true) {
    int delay = 200;
    for (int i = LED_COUNT; i > 0; i--) {
      int idx = LED_COUNT - i;
      int ledDelay = delay * i;
      leds[idx].FadeOff(speed).DelayBefore(ledDelay);
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      leds[i].FadeOff(speed);
    }
  }
}

void setup() {
  Serial.begin(baudRate);

  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void reset() {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].DelayBefore(0).DelayAfter(0).Repeat(1).Stop();
  }
}

void loop() {
  if (Serial.available()) {
    int inputLength = Serial.readBytes(line, BUFFER_SIZE);

    // parse command (approach https://forum.arduino.cc/t/parsing-comma-separated-string-values/514977/7)
    int effect, speed;
    char* field;
    field = strtok(line, ",");
    effect = atof(field);
    field = strtok(nullptr, ",");
    speed = atof(field);
    //  Serial.print("effect: ");
    //  Serial.print(effect);
    //  Serial.print("speed: ");
    //  Serial.print(speed);

    // send handshake response
    if (line[0] == 's') {
      Serial.println("s");
    }

    // check effect trigger
    switch (effect) {
      case ON:
        reset();
        currentMode = ON;
        startOn();
        break;
      case OFF:
        reset();
        currentMode = OFF;
        startOff();
        break;
      case BREATH:
        reset();
        currentMode = BREATH;
        if (speed == 0)
          startBreath();
        else
          startBreath(speed);
        break;

      case FLICKER:
        reset();
        currentMode = FLICKER;
        if (speed == 0)
          startFlicker();
        else
          startFlicker(speed);
        break;
      case STROBE:
        reset();
        currentMode = STROBE;
        if (speed == 0)
          startStrobe();
        else
          startStrobe();
         break;
      case RANDOM_STROBE:
        reset();
        currentMode = RANDOM_STROBE;
        startRandomStrobe();
        break;
      case FADE_ON:
        reset();
        currentMode = FADE_ON;
        if (speed == 0)
          startFadeOn(false);
        else
          startFadeOn(false, speed);
        break;
      case FADE_OFF:
        reset();
        currentMode = FADE_OFF;
        if (speed == 0)
          startFadeOff(false);
        else
          startFadeOff(false, speed);
        break;
      case FADE_ON_SEQ:
        reset();
        currentMode = FADE_ON_SEQ;
        if (speed == 0)
          startFadeOn(true);
        else
          startFadeOn(true, speed);
        break;
      case FADE_OFF_SEQ:
        reset();
        currentMode = FADE_OFF_SEQ;
        if (speed == 0)
          startFadeOff(true);
        else
          startFadeOff(true, speed);
    }
  }
  
  // update led effects
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].Update();
  }
}