#include <Arduino.h>
#include <jled.h>
bool startProcess = false;

enum lightMode { ON = 1, OFF, BREATH, FLICKER, STROBE, RANDOM_STROBE };
char* handshakeMessage = "s";
lightMode currentMode = OFF;
const int LED_COUNT = 16;
// int pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 29, 30, 23, 22, 21, 20};

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

void startOn() {
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i].On();
    }
}

void startOff() {
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i].Off();
    }
}

void startBreath() {
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i].Breathe(5000).Forever();
    }
}

void startFlicker() {
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i].Candle(5, 255, 1000).Forever().DelayAfter(4000);
    }
}

void startStrobe() {
    for (int i = 0; i < LED_COUNT; i++) {
        leds[i].Blink(50, 50).Forever();
    }
}

void startRandomStrobe() {
    for (int i = 0; i < LED_COUNT; i++) {
        long ledOn = random(0, 2);
        Serial.println(ledOn);
        if (ledOn == 1)
            analogWrite(pins[i], HIGH);
        else
            analogWrite(pins[i], LOW);
    }
}

void setup() {
    Serial.begin(9600);
    for (int i = 0; i < LED_COUNT; i++) {
        pinMode(pins[i], OUTPUT);
    }
}

void loop() {
    char incomingByte = Serial.read();
    Serial.println(incomingByte);

    if (incomingByte == handshakeMessage) {
        Serial.write("s");
    }

    switch (incomingByte) {
        case 1:
            currentMode = ON;
            Serial.write("on");
            startOn();
            break;
        case 2:
            currentMode = OFF;
            Serial.write("off");
            startOff();
            break;
        case BREATH:
            currentMode = BREATH;
            Serial.write("breathe");
            startBreath();
            break;
        case FLICKER:
            currentMode = FLICKER;
            Serial.write("flicker");
            startFlicker();
            break;
        case STROBE:
            currentMode = STROBE;
            Serial.write("strobe");
            startStrobe();
            break;
        case RANDOM_STROBE:
            currentMode = RANDOM_STROBE;
            Serial.write("random strobe");
            startRandomStrobe();
            break;
    }

    if (currentMode == RANDOM_STROBE) {
        startRandomStrobe();
    } else {
        for (int i = 0; i < LED_COUNT; i++) {
            leds[i].Update();
        }
    }
}