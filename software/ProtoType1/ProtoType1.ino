
const byte CLK_E = 9; // D9
const byte CLK_Q = 8; // D8
const byte RESET = 7; // D7

const byte DATA[] PROGMEM = {
  10, // bit 0, D10
  16, // bit 1, D16
  14, // bit 2, D14
  15, // bit 3, D15
  18, // bit 4, A0
  19, // bit 5, A1
  20, // bit 6, A2
  21, // bit 7, A3
};

static void setDigital(byte pin, bool value, const char *name) {
  digitalWrite(pin, value);
  Serial.print(name);
  Serial.println(value ? "HIGH" : "LOW");
}

static void reset(bool value) {
  setDigital(RESET, value, "#RES=");
}

static void clkE(bool value) {
  digitalWrite(CLK_E, value);
}

static void clkQ(bool value) {
  digitalWrite(CLK_Q, value);
}

static void setDataDirection(byte mode) {
  for (byte bit = 0; bit < 8; bit++) {
    pinMode(DATA[bit], mode);
  }
}

static void setData(byte value) {
  for (byte bit = 0; bit < 8; bit++) {
    digitalWrite(DATA[bit], (value & 1) != 0);
    value >>= 1;
  }
}

static byte getData() {
  byte value = 0;
  for (byte bit = 0; bit < 8; bit++) {
    value >>= 1;
    if (digitalRead(DATA[bit])) value |= 0x80;
  }
  return value;
}

static void initReset() {
  pinMode(RESET, OUTPUT);
  reset(LOW);
}

static void initClocks() {
  pinMode(CLK_E, OUTPUT);
  pinMode(CLK_Q, OUTPUT);
  clkE(LOW);
  clkQ(LOW);
}

static void initData() {
  setDataDirection(INPUT_PULLUP);
  setData(0xff); // pullup
}

void setup() {
  Serial.begin(9600);
  initReset();
  initClocks();
  initData();
}

int clockDelay = 250;
bool freeClock = false;

void loop() {
  char c = Serial.read();
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
    clkQ(HIGH);
    delay(clockDelay);
    clkE(HIGH);
    delay(clockDelay);
    clkQ(LOW);
    delay(clockDelay);
    clkE(LOW);
  }
}
