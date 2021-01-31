/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "mc6850.h"

#include <libcli.h>
#include "pins.h"

//#define DEBUG_IRQ
//#define DEBUG_STATUS
//#define DEBUG_CONTROL
//#define DEBUG_READ
//#define DEBUG_WRITE

void Mc6850::assertIrq(uint8_t intMask) {
    _status |= IRQF_bm;
    Pins.assertIrq(intMask);
#ifdef DEBUG_IRQ
    if (intMask & _rxInt)
        Cli.println(F("@@ Assert RX INT"));
    if (intMask & _txInt)
        Cli.println(F("@@ Assert TX INT"));
#endif
}

void Mc6850::negateIrq(uint8_t intMask) {
    Pins.negateIrq(intMask);
    _status &= ~IRQF_bm;
#ifdef DEBUG_IRQ
    if (intMask & _rxInt)
        Cli.println(F("@@ Negate RX IRQ"));
    if (intMask & _txInt)
        Cli.println(F("@@ Negate TX IRQ"));
#endif
}

void Mc6850::loop() {
    if (_serial.available() > 0) {
        _rxData = _serial.read();
        if (rxRegFull())
            _nextFlags |= OVRN_bm;
        _status |= RDRF_bm;
        if (rxIntEnabled())
            assertIrq(_rxInt);
    }
    // TODO: Implement flow control
    if (_serial.availableForWrite() > 0) {
        if (!txRegEmpty()) {
            _serial.write(_txData);
            _status |= TDRE_bm;
            if (txIntEnabled())
                assertIrq(_txInt);
        }
    }
}

void Mc6850::write(uint8_t data, uint16_t addr) {
    if (addr == _baseAddr) {
#ifdef DEBUG_CONTROL
        Cli.print(F("@@ Control 0x"));
        Cli.println(data, HEX);
#endif
        const uint8_t delta = _control ^ data;
        _control = data;
        if (cds(delta) && masterReset()) {
            negateIrq(_txInt | _rxInt);
            _status = TDRE_bm;
            _readFlags = _nextFlags = 0;
        }
        if (tcb(delta)) {
            // TODO: flow control
            if (txIntEnabled() && txRegEmpty()) {
                assertIrq(_txInt);
            } else {
                negateIrq(_txInt);
            }
        }
        if (rieb(delta)) {
            if (rxIntEnabled() && rxRegFull()) {
                assertIrq(_rxInt);
            } else {
                negateIrq(_rxInt);
            }
        }
    } else {
        _txData = data;
        _status &= ~TDRE_bm;
        if (txIntEnabled())
            negateIrq(_txInt);
#ifdef DEBUG_WRITE
        Cli.print(F("@@ Write 0x"));
        Cli.print(data, HEX);
        Cli.print(F(" 0x"));
        Cli.println(_status, HEX);
#endif
    }
}

uint8_t Mc6850::read(uint16_t addr) {
    if (addr == _baseAddr) {
        _readFlags = _status & (DCD_bm | OVRN_bm);
#ifdef DEBUG_STATUS
        if (_readFlags) {
            Cli.print(F("@@ Status 0x"));
            Cli.println(_status, HEX);
        }
#endif
        return _status;
    } else {
#ifdef DEBUG_READ
        const uint8_t prev_status = _status;
#endif
        _status &= ~(RDRF_bm | _readFlags);
        _status |= _nextFlags;
        _readFlags = _nextFlags = 0;
#ifdef DEBUG_READ
        Cli.print(F("@@ Read 0x"));
        Cli.print(_rxData, HEX);
        Cli.print(F(" 0x"));
        Cli.print(prev_status, HEX);
        Cli.print(F("->0x"));
        Cli.println(_status, HEX);
#endif
        if (rxIntEnabled()) {
            if (_status & (RDRF_bm | OVRN_bm)) {
                assertIrq(_rxInt);
            } else {
                negateIrq(_rxInt);
            }
        }
        return _rxData;
    }
}
