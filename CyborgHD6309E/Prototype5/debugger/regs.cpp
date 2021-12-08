#include "regs.h"

#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;

struct Regs Regs;
struct Memory Memory;

static const char *cpu_type = nullptr;
static const char MC6809[] = "6809";
static const char HD6309[] = "6309";
static const uint8_t clrb[] = {0x5F};
static const uint8_t comd[] = {0x10, 0x43};
static const uint8_t stb[] = {0xD7, 0x00};

static const char *checkCpu() {
    Pins.execInst(clrb, sizeof(clrb));
    // 6309:COMD, 6809:NOP+COMA
    Pins.execInst(comd, sizeof(comd));
    // 6309:B=0xFF, 6809:B=0
    uint8_t b;
    Pins.captureWrites(stb, sizeof(stb), &b, 1);
    return b ? HD6309 : MC6809;
}

bool Regs::is6309() const {
    return cpu_type == HD6309;
}

const char *Regs::cpu() const {
    return cpu_type ? cpu_type : MC6809;
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // clang-format off
    static char buffer[] = {
        'D', 'P', '=', 0, 0, ' ',  // dp=3
        'P', 'C', '=', 0, 0, 0, 0, // pc=9
        ' ', 'S', '=', 0, 0, 0, 0, // s=16
        ' ', 'U', '=', 0, 0, 0, 0, // u=23
        ' ', 'Y', '=', 0, 0, 0, 0, // y=30
        ' ', 'X', '=', 0, 0, 0, 0, // x=37
        ' ', 'A', '=', 0, 0,       // A=44
        ' ', 'B', '=', 0, 0,       // B=49
        ' ',                       // 6309=51
        'W', '=', 0, 0, 0, 0,      // w=54
        ' ', 'V', '=', 0, 0, 0, 0, // v=61
        0,                         // EOS
    };
    // clang-format on
    outHex8(buffer + 3, dp);
    outHex16(buffer + 9, pc);
    outHex16(buffer + 16, s);
    outHex16(buffer + 23, u);
    outHex16(buffer + 30, y);
    outHex16(buffer + 37, x);
    outHex8(buffer + 44, a);
    outHex8(buffer + 49, b);
    if (is6309()) {
        buffer[51] = ' ';
        outHex16(buffer + 54, getW());
        outHex16(buffer + 61, v);
    } else {
        buffer[51] = 0;
    }
    // clang-format off
    static char cc_bits[] = {
        ' ', 'C', 'C', '=',
        0, 0, 0, 0, 0, 0, 0, 0,
        0,
    };
    // clang-format on
    char *p = cc_bits + 4;
    *p++ = bit1(cc & 0x80, 'E');
    *p++ = bit1(cc & 0x40, 'F');
    *p++ = bit1(cc & 0x20, 'H');
    *p++ = bit1(cc & 0x10, 'I');
    *p++ = bit1(cc & 0x08, 'N');
    *p++ = bit1(cc & 0x04, 'Z');
    *p++ = bit1(cc & 0x02, 'V');
    *p = bit1(cc & 0x01, 'C');
    cli.print(buffer);
    cli.println(cc_bits);
}

void Regs::get(bool show) {
    save();
    restore();
    if (show)
        print();
}

static constexpr uint16_t le16(const uint8_t *p) {
    return *p | (static_cast<uint16_t>(p[1]) << 8);
}
static const uint8_t pshs_all[] = {0x34, 0xFF};  // PSHS PC,U,Y,X,DP,B,A,CC
static const uint8_t pshu_s[] = {0x36, 0x40};    // PSHU S
static const uint8_t tfr_wy[] = {0x1F, 0x62};    // TFR W,Y
static const uint8_t tfr_vx[] = {0x1F, 0x71};    // TFR V,X
static const uint8_t pshu_yx[] = {0x36, 0x30};   // PSHU Y,X
void Regs::save() {
    uint8_t bytes[12];
    Pins.captureWrites(pshs_all, sizeof(pshs_all), bytes, 12);
    // capture writes to stack in reverse order.
    pc = le16(bytes + 0) - 2;  // offset PSHS instruction.
    u = le16(bytes + 2);
    y = le16(bytes + 4);
    x = le16(bytes + 6);
    dp = bytes[8];
    b = bytes[9];
    a = bytes[10];
    cc = bytes[11];
    Pins.captureWrites(pshu_s, sizeof(pshu_s), bytes, 2);
    s = le16(bytes + 0) + 12;  // offset PSHS stack frame.
    if (cpu_type == nullptr)
        cpu_type = checkCpu();
    if (is6309()) {
        Pins.execInst(tfr_wy, sizeof(tfr_wy));
        Pins.execInst(tfr_vx, sizeof(tfr_vx));
        Pins.captureWrites(pshu_yx, sizeof(pshu_yx), bytes, 4);
        setW(le16(bytes + 0));
        v = le16(bytes + 2);
    }
}

static constexpr uint8_t hi(const uint16_t v) {
    return static_cast<uint8_t>(v >> 8);
}
static constexpr uint8_t lo(const uint16_t v) {
    return static_cast<uint8_t>(v);
}
static const uint8_t tfr_dv[] = {0x1F, 0x07};  // TFR D,V
void Regs::restore() {
    if (is6309()) {
        const uint8_t ldd[] = {0xCC, hi(v), lo(v)};  // LDD #v
        Pins.execInst(ldd, sizeof(ldd));
        Pins.execInst(tfr_dv, sizeof(tfr_dv));
        const uint16_t w = getW();
        const uint8_t ldw[] = {0x10, 0x86, hi(w), lo(w)};  // LDW #w
        Pins.execInst(ldw, sizeof(ldw));
    }
    const uint16_t sp = s - 12;
    const uint8_t lds[] = {0x10, 0xCE, hi(sp), lo(sp)};  // LDS #(s-12)
    Pins.execInst(lds, sizeof(lds));
    const uint8_t puls_all[] = {0x35, 0xFF, cc, a, b, dp, hi(x), lo(x), hi(y),
            lo(y), hi(u), lo(u), hi(pc), lo(pc)};
    Pins.execInst(puls_all, sizeof(puls_all));
}

void Regs::printRegList() const {
    cli.print(F("?Reg: pc s u x y a b d"));
    if (is6309())
        cli.print(F(" w e f v"));
    cli.println(F(" DP cc"));
}

bool Regs::validUint8Reg(char reg) const {
    if (reg == 'D' || reg == 'a' || reg == 'b' || reg == 'c' ||
            (is6309() && (reg == 'e' || reg == 'f'))) {
        cli.print(reg);
        if (reg == 'D')
            cli.print('P');
        if (reg == 'c')
            cli.print('c');
        return true;
    }
    return false;
}

bool Regs::validUint16Reg(char reg) const {
    if (reg == 'p' || reg == 's' || reg == 'u' || reg == 'y' || reg == 'x' ||
            reg == 'd' || (is6309() && (reg == 'w' || reg == 'v'))) {
        cli.print(reg);
        if (reg == 'p')
            cli.print('c');
        return true;
    }
    return false;
}

bool Regs::setRegValue(char reg, uint32_t value, State state) {
    if (state == State::CLI_CANCEL)
        return true;
    if (state == State::CLI_DELETE) {
        cli.backspace(reg == 'p' || reg == 'D' || reg == 'c' ? 3 : 2);
        return false;
    }
    switch (reg) {
    case 'p':
        pc = value;
        break;
    case 's':
        s = value;
        break;
    case 'u':
        u = value;
        break;
    case 'y':
        y = value;
        break;
    case 'x':
        x = value;
        break;
    case 'd':
        setD(value);
        break;
    case 'w':
        setW(value);
        break;
    case 'v':
        v = value;
        break;
    case 'a':
        a = value;
        break;
    case 'b':
        b = value;
        break;
    case 'e':
        e = value;
        break;
    case 'f':
        f = value;
        break;
    case 'D':
        dp = value;
        break;
    case 'c':
        cc = value;
        break;
    }
    restore();
    print();
    return true;
}

uint8_t Memory::read(uint16_t addr) const {
    static uint8_t LDA[] = {0xB6, 0, 0}; // LDA $addr
    LDA[1] = hi(addr);
    LDA[2] = lo(addr);
    uint8_t data;
    Pins.captureReads(LDA, sizeof(LDA), &data, 1);
    return data;
}

void Memory::write(uint16_t addr, uint8_t data) {
    static uint8_t LDA[] = {0x86, 0};    // LDA #data
    static uint8_t STA[] = {0xB7, 0, 0}; // STA $addr
    LDA[1] = data;
    STA[1] = hi(addr);
    STA[2] = lo(addr);
    Pins.execInst(LDA, sizeof(LDA));
    Pins.execInst(STA, sizeof(STA));
}

uint8_t Memory::nextByte() {
    Regs.save();
    const uint8_t data = read(address());
    Regs.restore();
    return data;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
