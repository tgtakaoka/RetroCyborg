#ifndef __SCI_HANDLER_H__
#define __SCI_HANDLER_H__

#include <Arduino.h>
#include <stdint.h>

enum SciDivider : uint16_t {
    DIV16 = 16,
    DIV128 = 128,
    DIV1024 = 1024,
    DIV4096 = 4096,
};

template <uint8_t RXD_PIN, uint8_t TXD_PIN>
class SciHandler {
public:
    SciHandler(Stream &stream, SciDivider divider)
        : _stream(stream), _divider(divider) {
        pinMode(RXD_PIN, INPUT_PULLUP);  // output mark/idle
        pinMode(TXD_PIN, INPUT_PULLUP);
        reset();
    }

    void reset() {
        _rx.bit = 0;
        _tx.bit = 0;
    }

    void loop() {
        txloop(_tx);
        rxloop(_rx);
    }

private:
    Stream &_stream;
    SciDivider _divider;
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
                tx.delay = uint16_t(_divider);
                digitalWriteFast(RXD_PIN, LOW);  // start bit
                pinMode(RXD_PIN, OUTPUT);
            }
        } else {
            if (--tx.delay == 0) {
                digitalWriteFast(RXD_PIN, tx.data & 1);
                tx.data >>= 1;
                tx.delay = uint16_t(_divider);
                if (--tx.bit == 2) {
                    tx.data = 3;  // stop bit
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
                rx.delay = uint16_t(_divider) * 3 / 2;
            }
        } else {
            if (--rx.delay == 0) {
                rx.data >>= 1;
                if (digitalReadFast(TXD_PIN) == HIGH)
                    rx.data |= 0x80;
                rx.delay = uint16_t(_divider);
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
