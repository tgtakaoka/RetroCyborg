/* -*- mode: c++; -*- */
#ifndef __DUMP_TIMER_H__
#define __DUMP_TIMER_H__

#include <Arduino.h>
#include <stdint.h>

class DumpTimer {
public:
    DumpTimer(Stream &console) : _console(console) {}
    void dumpTCA(uint8_t n);
    void dumpTCB(uint8_t n);

private:
    Stream &_console;

    void newline() { _console.println(); }
    void printText(const char *text, const char *next = nullptr);
    void printDec(uint16_t val) { _console.print(val, DEC); }
    void printHex(uint16_t val, uint8_t digits);
    void printHex(uint16_t val) { printHex(val, sizeof(val) * 2); }
    void printHex(uint8_t val) { printHex(val, sizeof(val) * 2); }
    void dumpTCA_CMP(
            const char *name, uint8_t n, bool CMPEN, bool CMPOV, uint16_t CMP);
};

#endif
