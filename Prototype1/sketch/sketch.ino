/**
 * Cyborg09 controller
 *
 * This sketch is designed for SparkFun Pro Micro 16MHz 5V and controll
 * vanilla HD63C09E MPU.
 *
 * USB Serial port is used as console. So one can connect with
 *   $ screen /dev/<tty USB> 9600
 *
 * The Controller can accept commands represented by one letter.
 *
 * r - assert #RES to low.
 * R - negate #RES to high.
 * k - stop negerating clock E and Q.
 * K - start free running clock E and Q.
 * c - one bus cycle. stopped at E=H Q=L.
 * s - print 6309 status.
 * ? - print version.
 */

const char * const VERSION = "* Cyborg09 Prototype1 0.1";

const byte RESET = 6; // D6
const byte CLK_Q = 7; // D7
const byte CLK_E = 8; // D8
const byte RD_WR = 9; // D9

const byte DBUS[] PROGMEM = {
  21, // bit 0, A3
  20, // bit 1, A2
  19, // bit 2, A1
  18, // bit 3, A0
  15, // bit 4, SCLK
  14, // bit 5, MISO
  16, // bit 6, MOSI
  10, // bit 7, A10
};

static void setDigital(const byte pin, const bool value, const char *name) {
  digitalWrite(pin, value);
  Serial.print(name);
  Serial.println(value ? "HIGH" : "LOW");
}

static void reset(const bool value) {
  setDigital(RESET, value, "#RES=");
}

static void clkE(const bool value) {
  digitalWrite(CLK_E, value);
}

static void clkQ(const bool value) {
  digitalWrite(CLK_Q, value);
}

static byte pinDataBus(const byte bit) {
  return pgm_read_byte_near(DBUS + bit);
}

static void setDataBusDirection(const byte mode) {
  if (mode == OUTPUT && digitalRead(RD_WR) == LOW) {
    Serial.println("!! RW=LOW");
    return;
  }
  for (byte bit = 0; bit < 8; bit++) {
    pinMode(pinDataBus(bit), mode);
  }
}

static void setDataBus(byte data) {
  for (byte bit = 0; bit < 8; bit++) {
    const byte pin = pinDataBus(bit);
    if (data & 0x01) {
      digitalWrite(pin, HIGH);
    } else {
      digitalWrite(pin, LOW);
    }
    data >>= 1;
  }
}

static byte getDataBus() {
  byte data = 0;
  for (byte bit = 0; bit < 8; bit++) {
    data >>= 1;
    const byte pin = pinDataBus(bit);
    if (digitalRead(pin) == HIGH) {
      data |= 0x80;
    }
  }
  return data;
}

static void printPinStatus(const byte pin, const char *title) {
  Serial.print(title);
  Serial.print(digitalRead(pin) ? "H" : "L");
}

static void printStatus() {
  printPinStatus(RESET, "#RES=");
  printPinStatus(CLK_E, " E=");
  printPinStatus(CLK_Q, " Q=");
  printPinStatus(RD_WR, " RW=");
  Serial.print(" Dn=0x");
  Serial.println(getDataBus(), HEX);
}

void setup() {
  Serial.begin(9600);

  pinMode(RESET, OUTPUT);
  reset(LOW);

  pinMode(CLK_E, OUTPUT);
  pinMode(CLK_Q, OUTPUT);
  clkE(LOW);
  clkQ(LOW);

  setDataBusDirection(INPUT);
}

static int clockDelay = 250;
static bool freeClock = false;

void loop() {
  char c = Serial.read();
  if (c == 's') {
    printStatus();
  }
  if (c == 'r' || c == 'R') {
    reset(c == 'R');
  }
  if (c == 'k' || c == 'K') {
    freeClock = (c == 'K');
    Serial.print("Clock ");
    Serial.println(c == 'K' ? "running" : "stop");
  }
  if (c == 'c' || freeClock) {
    if (c == 'c') Serial.println("Clock step");
    delay(clockDelay);
    clkE(LOW);
    delay(clockDelay);
    clkQ(HIGH);
    delay(clockDelay);
    clkE(HIGH);
    delay(clockDelay);
    clkQ(LOW);
    printStatus();
  }
  if (c == '?') {
    Serial.println(VERSION);
  }
}
