#include <Arduino.h>
#include <jled.h>
bool startProcess = false;

enum lightMode { ON = 1, OFF, BREATH, FLICKER, STROBE, RANDOM_STROBE, FADE_ON, FADE_OFF, FADE_ON_SEQ, FADE_OFF_SEQ };

char* handshakeMessage = "s";
lightMode currentMode = OFF;
const int LED_COUNT = 16;
int teensyPins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 29, 30, 23, 22, 21, 20, 17};

// re-order pins based on led orientation
int pins[] = {teensyPins[8], teensyPins[4],  teensyPins[1],  teensyPins[14], teensyPins[9],
              teensyPins[6], teensyPins[3],  teensyPins[12], teensyPins[10], teensyPins[7],
              teensyPins[2], teensyPins[13], teensyPins[11], teensyPins[15], teensyPins[5], teensyPins[10]
             };

// example orientation pins
int northPins[] = {1, 2, 3, 4};
// etc, ...

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

void startFlicker() {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].Candle(5, 255, 1000).Forever();
  }
}

void startStrobe() {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].Blink(25, 25).Forever();
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
    int delay = 1000;
    for (int i = 0; i < LED_COUNT; i++) {
      int ledDelay = delay * i;
      leds[i].FadeOn(speed).DelayBefore(ledDelay);
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      leds[i].FadeOn(1000);
    }
  }
}

void startFadeOff(bool isSequential = false, int speed = 1000) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].On();
  }
  if (isSequential == true) {
    int delay = 1000;
    for (int i = 0; i < LED_COUNT; i++) {
      int ledDelay = delay * i;
      leds[i].FadeOff(speed).DelayBefore(ledDelay);
    }
  } else {
    for (int i = 0; i < LED_COUNT; i++) {
      leds[i].FadeOff(1000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < LED_COUNT; i++) {
    pinMode(pins[i], OUTPUT);
  }
}

void reset() {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].DelayBefore(0).DelayAfter(0).Repeat(1).Stop();
  }
}
/*
  void loop() {
  char incomingByte = Serial.read();

  Serial.println(incomingByte);

  if (incomingByte == handshakeMessage) {
    Serial.write("s");
  }

  switch (incomingByte) {
    case ON:
      reset();
      currentMode = ON;
      Serial.write("on");
      startOn(false);
      break;
    case OFF:
      reset();
      currentMode = OFF;
      Serial.write("off");
      startOff(false);
      break;
    case BREATH:
      reset();
      currentMode = BREATH;
      Serial.write("breathe");
      startBreath();
      break;
    case FLICKER:
      reset();
      currentMode = FLICKER;
      Serial.write("flicker");
      startFlicker();
      break;
    case STROBE:
      reset();
      currentMode = STROBE;
      Serial.write("strobe");
      startStrobe();
      break;
    case RANDOM_STROBE:
      reset();
      currentMode = RANDOM_STROBE;
      Serial.write("random strobe");
      startRandomStrobe();
      break;
    case FADE_ON:
      reset();
      currentMode = FADE_ON;
      Serial.write("fade on");
      startFadeOn();
      break;
    case FADE_OFF:
      reset();
      currentMode = FADE_OFF;
      Serial.write("fade off");
      startFadeOff();
      break;
    case FADE_ON_SEQ:
      reset();
      currentMode = FADE_ON_SEQ;
      Serial.write("fade on seq");
      startFadeOn(true);
      break;
    case FADE_OFF_SEQ:
      reset();
      currentMode = FADE_OFF_SEQ;
      Serial.write("fade off seq");
      startFadeOff(true);
      break;
  }

  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].Update();
  }
  }
*/

const int BUFFER_SIZE = 24;
char buf[BUFFER_SIZE];


void loop() {
  int inputLength = Serial.readBytes(buf, BUFFER_SIZE);
  Serial.println(inputLength);
  if (inputLength == 0) {
    for (int i = 0; i < LED_COUNT; i++) {
      leds[i].Update();
    }
    return;
  }
  //   //https://arduino.stackexchange.com/questions/77081/arduino-split-comma-separated-serial-stream
  int command = atoi(strtok(buf, ","));
  int speed = atoi(strtok(NULL, ",")) * 1000;

  memset(buf, 0, BUFFER_SIZE);

  Serial.print("command: ");
  Serial.println(command);
  Serial.print("speed: ");
  Serial.println(speed);

  if (command == 1) {
    reset();
    currentMode = ON;
    Serial.println("on");
    startOn();
  }
  if (command == 2) {
    reset();
    currentMode = OFF;
    Serial.write("off");
    startOff();
  }
  if (command == 3) {
    reset();
    currentMode = BREATH;
    Serial.write("breathe");
    startBreath(speed);
  }
  if (command == FLICKER) {
    reset();
    currentMode = FLICKER;
    Serial.write("flicker");
    startFlicker();
  }
  if (command == STROBE) {
    reset();
    currentMode = STROBE;
    Serial.write("strobe");
    startStrobe();
  }
  if (command == RANDOM_STROBE) {
    reset();
    currentMode = RANDOM_STROBE;
    Serial.write("random strobe");
    startRandomStrobe();
  }

  // send interupt on chnage
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i].Update();
  }
}


// void loop() {
//  String command = Serial.readString();
//
//
//    if (command.toInt() == 1) {
//        reset();
//        currentMode = ON;
//        Serial.println("on");
//        startFadeOn(false);
//    }
//    if (command == 2) {
//        reset();
//        currentMode = OFF;
//      Serial.write("off");
//      startOff(false);
//    }
//    if (command == 3) {
//        currentMode = BREATH;
//        Serial.write("breathe");
//        startBreath();
//    }
////    if (command == FLICKER) {
////        currentMode = FLICKER;
////        Serial.write("flicker");
////        startFlicker();
////    }
////    if (command == STROBE) {
////        currentMode = STROBE;
////        Serial.write("strobe");
////        startStrobe();
////    }
////    if (command == RANDOM_STROBE) {
////        currentMode = RANDOM_STROBE;
////        Serial.write("random strobe");
////        startRandomStrobe();
////    }
////
////    // send interupt on chnage
//    for (int i = 0; i < LED_COUNT; i++) {
//        leds[i].Update();
//    }
//}