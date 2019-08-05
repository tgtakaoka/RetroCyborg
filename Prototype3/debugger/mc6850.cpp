#include "mc6850.h"

#include "pins.h"

void Mc6850::loop() {
  if (Serial.available() > 0) {
    if ((_status & RDRF) != 0)
      _status |= OVRN;
    _receivedData = Serial.read();
    _status |= RDRF;
  }
  if (Serial.availableForWrite() > 0) {
    _status |= TDRE;
  }
}

void Mc6850::write(uint8_t data, uint16_t addr) {
  if (addr == _baseAddr) {
    const uint8_t tcb = (data & (TCB0 | TCB1));
    if (tcb == TCB1) ;// set RTS high
    if (tcb == TCB0 || tcb == 0) ; //set RTS low.
    _control = data;
  } else {
    Serial.write((char)data);
    loop();
  }
}

uint8_t Mc6850::read(uint16_t addr) {
  if (addr == _baseAddr) {
    return _status;
  } else {
    if ((_status & RDRF) != 0)
      _status &= ~RDRF;
    return _receivedData;
  }
}
