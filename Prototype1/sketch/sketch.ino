/**
   Cyborg09 controller

   This sketch is designed for SparkFun Pro Micro 16MHz 5V and controll
   vanilla HD63C09E MPU.

   USB Serial port is used as console. So one can connect with
     $ screen /dev/<tty USB> 9600

   The Controller can accept commands represented by one letter.

   r - assert #RES to LOW.
   R - negate #RES to HIGH.
   h - assert #HALT to LOW.
   H - negate #HALT to HIGH.
   k - stop negerating clock E and Q.
   K - start free running clock E and Q.
   c - one bus cycle. stopped at E=HIGH Q=LOW.
   s - print 6309 status.
   d - set data bus, input 2 digit hex number.
   ? - print version.
*/

const char * const VERSION = "* Cyborg09 Prototype1 0.3";

#if defined(ARDUINO_AVR_ITSYBITSY32U4_5V)
/* Adafruit ItsyBitsy 32u4 5V */
const uint8_t HALT  = 18; // A0
const uint8_t BS    = 19; // A1
const uint8_t BA    = 20; // A2
const uint8_t LIC   = 21; // A3
const uint8_t AVMA  = 22; // A4
const uint8_t BUSY  = 23; // A5
const uint8_t RESET = 15; // SCK
const uint8_t CLK_Q = 16; // MOSI
const uint8_t CLK_E = 14; // MISO
const uint8_t RD_WR = 8;  // D8

const uint8_t DBUS[] PROGMEM = {
  10, // bit 0, D10
  9,  // bit 1, D9
  7,  // bit 2, D7
  5,  // bit 3, D5
  3,  // bit 4, SCL
  2,  // bit 5, SDA
  1,  // bit 6, TX
  0,  // bit 7, RX
};
#elif defined(ARDUINO_AVR_PROMICRO)
/* SparkFun Pro Micro 32u4 5V */
const uint8_t HALT  = 1; // TX/D1
const uint8_t BS    = 0; // RX/D0
const uint8_t BA    = 2; // D2/SDA
const uint8_t LIC   = 3; // D3/SCL
const uint8_t AVMA  = 4; // D4/D24/A6
const uint8_t BUSY  = 5; // D5
const uint8_t RESET = 6; // D6/D25/A7
const uint8_t CLK_Q = 7; // D7
const uint8_t CLK_E = 8; // D8/D26/A8
const uint8_t RD_WR = 9; // D9/D27/A9

const uint8_t DBUS[] PROGMEM = {
  21, // bit 0, A3/D21
  20, // bit 1, A2/D20
  19, // bit 2, A1/D19
  18, // bit 3, A0/D18
  15, // bit 4, D15/SCK
  14, // bit 5, D14/MISO
  16, // bit 6, D16/MOSI
  10, // bit 7, D10/A10
};
#endif

static void setDigital(const uint8_t pin, const uint8_t value, const char *name) {
  digitalWrite(pin, value);
  Serial.print(name);
  Serial.println(value ? "HIGH" : "LOW");
}

static void reset(const uint8_t value) {
  setDigital(RESET, value, "#RES=");
}

static void halt(const uint8_t value) {
  setDigital(HALT, value, "#HALT=");
}

static void clockE(const uint8_t value) {
  digitalWrite(CLK_E, value);
}

static void clockQ(const uint8_t value) {
  digitalWrite(CLK_Q, value);
}

static uint8_t pinDataBus(const uint8_t bit) {
  return pgm_read_byte_near(DBUS + bit);
}

static bool setDataBusDirection(const uint8_t mode) {
  if (mode == OUTPUT && digitalRead(RD_WR) == LOW) {
    Serial.println(" !! RW=LOW");
    return false;
  }
  for (uint8_t bit = 0; bit < 8; bit++) {
    pinMode(pinDataBus(bit), mode);
  }
  return true;
}

static void setDataBus(uint8_t data) {
  for (uint8_t bit = 0; bit < 8; bit++) {
    const uint8_t pin = pinDataBus(bit);
    if (data & 0x01) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
    data >>= 1;
  }
}

static uint8_t getDataBus() {
  uint8_t data = 0;
  for (uint8_t bit = 0; bit < 8; bit++) {
    data >>= 1;
    const uint8_t pin = pinDataBus(bit);
    if (digitalRead(pin) == HIGH) {
      data |= 0x80;
    }
  }
  return data;
}

static void printPinStatus(const uint8_t pin, const char *title) {
  Serial.print(title);
  Serial.print(digitalRead(pin) ? "H" : "L");
}

static void printStatus() {
  printPinStatus(RESET, "#RES=");
  printPinStatus(HALT,  " HALT=");
  printPinStatus(BA,    " BA=");
  printPinStatus(BS,    " BS=");
  printPinStatus(LIC,   " LIC=");
  printPinStatus(AVMA,  " AVMA=");
  printPinStatus(BUSY,  " BUSY=");
  printPinStatus(RD_WR, " RW=");
  Serial.print(" Dn=0x");
  Serial.println(getDataBus(), HEX);
}

void setup() {
  Serial.begin(9600);

  pinMode(RESET, OUTPUT);
  reset(LOW);
  pinMode(HALT,  OUTPUT);
  halt(LOW);

  pinMode(CLK_E, OUTPUT);
  pinMode(CLK_Q, OUTPUT);
  clockE(LOW);
  clockQ(LOW);

  pinMode(BS,    INPUT);
  pinMode(BA,    INPUT);
  pinMode(LIC,   INPUT);
  pinMode(AVMA,  INPUT);
  pinMode(BUSY,  INPUT);
  pinMode(RD_WR, INPUT);

  setDataBusDirection(INPUT);
}

static bool clockRunning = false;

static void clock(const bool value, const char *message) {
  clockRunning = value;
  Serial.println(message);
}

static void clockCycle(int8_t ms) {
  delay(ms);
  clockE(LOW);
  setDataBusDirection(INPUT);
  delay(1);
  clockQ(HIGH);
  delay(1);
  clockE(HIGH);
  delay(1);
  clockQ(LOW);
}

#define COMMAND    0
#define HEX_NUMBER 1

static char inCommand;
static uint16_t hexNumber;

static int readHexNumber(char command) {
  inCommand = command;
  Serial.print(command);
  Serial.print('?');
  hexNumber = 0;
  return HEX_NUMBER;
}

static int handleHexNumber() {
  if (inCommand == 'd') {
    if (setDataBusDirection(OUTPUT)) {
      setDataBus((uint8_t)hexNumber);
      Serial.print(" Dn=0x");
      Serial.println(getDataBus(), HEX);
    }
    printStatus();
  }
  return COMMAND;
}

static int cancelHexNumber() {
  Serial.println(" cancel");
  return COMMAND;
}

static int processHexNumber() {
  char c = Serial.read();
  if (isDigit(c)) {
    hexNumber *= 16;
    hexNumber += (c - '0');
  } else if (isHexadecimalDigit(c)) {
    hexNumber *= 16;
    hexNumber += (c - (isUpperCase(c) ? 'A' : 'a')) + 10;
  }
  if (c == '\b' || c == 0x7f) {
    c = '\b';
    hexNumber /= 16;
  }
  if (c == 0x1b) {
    return cancelHexNumber();
  }
  if (c == '\r' || c == '\n') {
    return handleHexNumber();
  }

  if (c != -1) {
    Serial.print(c);
  }
  return HEX_NUMBER;
}

static int processCommand() {
  char c = Serial.read();
  if (c == 's') printStatus();
  if (c == 'r') reset(LOW);
  if (c == 'R') reset(HIGH);
  if (c == 'h') halt(LOW);
  if (c == 'H') halt(HIGH);
  if (c == 'k') clock(false, "Clock stop");
  if (c == 'K') clock(true,  "Clock running");
  if (c == 'c' || clockRunning) {
    bool step = (c == 'c');
    clockCycle(step ? 1 : 1000);
    printStatus();
  }
  if (c == 'd') return readHexNumber(c);
  if (c == '?') Serial.println(VERSION);
  return COMMAND;
}

static int inputMode = COMMAND;

void loop() {
  switch (inputMode) {
  case COMMAND:
    inputMode = processCommand();
    break;
  case HEX_NUMBER:
    inputMode = processHexNumber();
    break;
  }
}
