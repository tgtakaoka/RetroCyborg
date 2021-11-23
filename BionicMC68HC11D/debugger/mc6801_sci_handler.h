#ifndef __MC6801_SCI_HANDLER_H__
#define __MC6801_SCI_HANDLER_H__

#include <Arduino.h>
#include <stdint.h>

template <const uint8_t RXD_PIN, const uint8_t TXD_PIN>
class Mc6801SciHandler {
public:
    Mc6801SciHandler(Stream &stream)
        : _stream(stream), _enabled(false) {
        pinMode(RXD_PIN, INPUT_PULLUP);  // output mark/idle
        pinMode(TXD_PIN, INPUT_PULLUP);
        reset();
    }

    bool isSelected(uint16_t addr) const { return _enabled && addr == 0x10; }
    void write(uint8_t data, uint16_t addr) {
        static const uint16_t scaler[] = {16, 128, 1024, 4096};
        reset();
        _divider = scaler[data & 3];
    }
    uint8_t read(uint16_t addr) { return 0; }

    void reset() {
        _divider = 16;
        _rx.bit = 0;
        _tx.bit = 0;
    }

    void loop() {
        if (_enabled) {
            txloop(_tx);
            rxloop(_rx);
        }
    }

    void enable(bool enabled) {
        _enabled = enabled;
        reset();
    }

private:
    Stream &_stream;
    bool _enabled;
    uint16_t _divider;
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
                tx.delay = _divider;
                if (--tx.bit == 2) {
                    tx.data = 3;  // stop bit and mark
                } else if (tx.bit == 0) {
                    pinMode(RXD_PIN, INPUT_PULLUP);  // output mark/idle
                }
            }
        }
    }

    void rxloop(Receiver &rx) {
        if (rx.bit == 0) {
            if (digitalReadFast(TXD_PIN) == LOW) {
                rx.bit = 9;
                rx.data = 0;
                rx.delay = _divider + (_divider >> 1);
            }
        } else {
            if (--rx.delay == 0) {
                rx.data >>= 1;
                if (digitalReadFast(TXD_PIN) == HIGH)
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
