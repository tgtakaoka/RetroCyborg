#ifndef __INS8070_SCI_HANDLER_H__
#define __INS8070_SCI_HANDLER_H__

#include <Arduino.h>
#include <stdint.h>

#include "config.h"

template<const uint8_t RXD_PIN, const uint8_t TXD_PIN>
class Ins8070SciHandler {
public:
    Ins8070SciHandler(Stream &stream)
        : _stream(stream), _enabled(false) {
        pinMode(RXD_PIN, INPUT_PULLUP);
        pinMode(TXD_PIN, INPUT_PULLUP);
        reset();
    }

    /** @param baudrate in BCD */
    void reset(uint16_t baudrate = 0x110) {
        // INS8070 bitbang speed: assuming XTAL is 4MHz
        switch (baudrate) {
        case 0x4800:
            _baudrate = 4800;
            _pre_divider = 74;
            _divider = 14;
            break;
        default: // 110 bps
            _baudrate = 110;
            _pre_divider = 2561;
            _divider = 18;
            break;
        }
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

    /** @param baudrate in BCD */
    void enable(bool enabled, uint16_t baudrate) {
        _enabled = enabled;
        reset(baudrate);
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
                digitalWriteFast(RXD_PIN, LOW);  // start bit
                pinMode(RXD_PIN, OUTPUT);
            }
        } else {
            if (--tx.delay == 0) {
                digitalWriteFast(RXD_PIN, tx.data & 1);
                tx.data >>= 1;
                tx.data |= 0x80; // stop and mark bits
                tx.delay = _divider;
                if (--tx.bit == 0) {
                    pinMode(RXD_PIN, INPUT_PULLUP);  // output mark/idle
                }
            }
        }
    }

    void rxloop(Receiver &rx) {
        // TXD (F1) is inverted.
        if (rx.bit == 0) {
            if (digitalReadFast(TXD_PIN) == HIGH) {
                rx.bit = 9;
                rx.data = 0;
                rx.delay = _divider + (_divider >> 1);
            }
        } else {
            if (--rx.delay == 0) {
                rx.data >>= 1;
                if (digitalReadFast(TXD_PIN) == LOW)
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
