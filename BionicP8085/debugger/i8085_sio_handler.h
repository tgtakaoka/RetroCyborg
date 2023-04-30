#ifndef __I8085_SCI_HANDLER_H__
#define __I8085_SCI_HANDLER_H__

#include <Arduino.h>
#include <stdint.h>

#include "config.h"

class I8085SioHandler {
public:
    I8085SioHandler(Stream &stream)
        : _stream(stream), _enabled(false) {
        pinMode(PIN_SID, INPUT_PULLUP);
        pinMode(PIN_SOD, INPUT_PULLUP);
        reset();
    }

    /** Baudrate is 9,600bps */
    void reset() {
        // I8085 bitbang speed: assuming CLK is 3MHz
        _baudrate = 9600;
        _pre_divider = 24;
        _divider = 13;
        _prescaler = _pre_divider;
        _rx.bit = 0;
        _tx.bit = 0;
    }

    void loop() {
        if (_enabled && --_prescaler == 0) {
            _prescaler = _pre_divider;
            txloop(_tx);
            rxloop(_rx);
        }
    }

    void enable(bool enabled) {
        _enabled = enabled;
        reset();
    }

    /** @return baudrate in decimal */
    uint16_t baudrate() const { return _baudrate; }

private:
    Stream &_stream;
    bool _enabled;
    uint16_t _baudrate;
    uint16_t _divider;
    uint16_t _pre_divider;
    uint16_t _prescaler;
    struct Transmitter {
        uint8_t bit;
        uint8_t data;
        uint16_t delay;
    } _tx;
    struct Receiver {
        uint8_t bit;
        uint8_t data;
        uint16_t delay;
    } _rx;

    void txloop(Transmitter &tx) {
        if (tx.bit == 0) {
            if (_stream.available()) {
                tx.bit = 10;  // start bit + data bits + stop bit
                tx.data = _stream.read();
                tx.delay = _divider;
                digitalWriteFast(PIN_SID, LOW);  // start bit
                pinMode(PIN_SID, OUTPUT);
            }
        } else {
            if (--tx.delay == 0) {
                digitalWriteFast(PIN_SID, tx.data & 1);
                tx.data >>= 1;
                tx.data |= 0x80; // stop and mark bits
                tx.delay = _divider;
                if (--tx.bit == 0) {
                    pinMode(PIN_SID, INPUT_PULLUP);  // output mark/idle
                }
            }
        }
    }

    void rxloop(Receiver &rx) {
        if (rx.bit == 0) {
            if (digitalReadFast(PIN_SOD) == LOW) {
                rx.bit = 9;
                rx.data = 0;
                rx.delay = _divider + (_divider >> 1);
            }
        } else {
            if (--rx.delay == 0) {
                rx.data >>= 1;
                if (digitalReadFast(PIN_SOD) != LOW)
                    rx.data |= 0x80;
                rx.delay = _divider;
                if (--rx.bit == 1) {
                    _stream.write(rx.data);
                    // stop bit will be ignored.
                }
            }
        }
    }
};

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
