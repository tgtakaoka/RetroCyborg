#!/usr/bin/gawk -f
# -*- mode: awk; -*-

# 1: new instruction address
# 2-9: part of instruction
# 0: repeat current instruction
# N: prefetch next instruction
# n: dummy prefetch next instruction
# B: 8-bit relative branch
# P: poped from stack address
# d: dummy cycle, no read nor write
# R: read 1 byte
# W: write 1 byte, the same address if R is preceeded
# r: read 1 byte at address R+1
# w: write 1 byte at address W+1
# E: read 1 byte from direct page (FFxx)
# e: read 1 byte at address E+1
# F: write 1 byte to direct page (FFxx), the same address if E is preseeded
# f: write 1 byte at address F+1
# S: read 1 byte at R+2
# s: read 1 byte at R+3
# X: write 1 byte at W+2
# x: write 1 byte at W+3

BEGIN {
    PRETTY_PRINT = 0;
    GENERATE_CYCLES = 0;
    GENERATE_INST = 1;

    # GENERATE_CYCLES=1
    CYCLES[1]="N"
    CYCLES[2]="1"
    CYCLES[3]="wW1"
    CYCLES[4]="nRr1"
    CYCLES[5]="nRrSs1"
    CYCLES[6]="EN"
    CYCLES[7]="NF"
    CYCLES[8]="FN"
    CYCLES[9]="NFf"
    CYCLES[10]="EeN"
    CYCLES[11]="NwW"
    CYCLES[12]="nRrN"
    CYCLES[13]="ENF"
    CYCLES[14]="EeNFf"
    CYCLES[15]="nB"
    CYCLES[16]="n1xXwW1"
    CYCLES[17]="nN"
    CYCLES[18]="nRWN"
    CYCLES[19]="nRN"
    CYCLES[20]="RNW"
    CYCLES[21]="RN"
    CYCLES[22]="RrN"
    CYCLES[23]="RrWwN"
    CYCLES[24]="RrNWw"
    CYCLES[25]="NW"
    CYCLES[26]="WN"
    CYCLES[27]="NWw"
    CYCLES[28]="n1"
    CYCLES[29]="nwW1"
    # GENERATE_CYCLES=2
    CYCLES[30]="ENF@"
    CYCLES[31]="N"
    CYCLES[32]="B@"
    CYCLES[33]="N"
    CYCLES[34]="nB@"
    CYCLES[35]="N"
    CYCLES[36]="nRW0@"
    CYCLES[37]="nRWN"
    CYCLES[38]="nR0@"
    CYCLES[39]="nRN"
    CYCLES[40]="nRrP@"
    CYCLES[41]="N"
    CYCLES[42]="n1@"
    CYCLES[43]="N"
    CYCLES[44]="nwW1@"
    CYCLES[45]="N"

    if (GENERATE_CYCLES == 1)
        delete CYCLES;
}

function bus_cycles(sequence, tmp, c, cycles) {
    if (sequence == "-")
        return "-";
    delete tmp;
    split(sequence, tmp, ":");
    cycles = "";
    for (c in tmp) {
        if (tmp[c] ~ /[d2-9]/)
            continue;
        cycles = cycles tmp[c];
    }
    return cycles;
}

function bus_length(cycles) {
    if (cycles == "-")
        return 0;
    return length(cycles);
}

function find_prefetch(cycles, c, prefetch, idx, len) {
    prefetch = 0;
    len = bus_length(cycles);
    for (idx = 1; idx <= len; ++idx) {
        if (substr(cycles, idx, 1) ~ /[1N]/)
            prefetch = idx;
    }
    return prefetch;
}

function CYCLES_index(sequence, s) {
    for (s in CYCLES) {
        if (CYCLES[s] == sequence)
            return s;
    }
    return 0;
}

function pretty_print(op, nemo, opr, len,
                      true_bus, false_bus, true_seq, false_seq) {
    if (PRETTY_PRINT == 0)
        return;
    printf("%-2s %-4s %-7s %s ", op, nemo, opr, len);
    printf("%-7s %-7s ", true_bus, false_bus);
    printf("%-20s%s\n", true_seq, false_seq);
}

function add_CYCLES(bus) {
    CYCLES[length(CYCLES)+1] = bus;
    printf("    CYCLES[%d]=\"%s\"\n", length(CYCLES), bus);
}

function append_CYCLES(bus) {
    if (bus_length(bus) != 0 && CYCLES_index(bus) == 0) {
        add_CYCLES(bus);
        return 1;
    }
    return 0;
}

function generate_CYCLES(true_bus, false_bus) {
    if (GENERATE_CYCLES == 0)
        return 0;
    if (GENERATE_CYCLES == 1) {
        if (false_bus == "-" || true_bus == false_bus)
            append_CYCLES(true_bus);
    }
    if (GENERATE_CYCLES == 2) {
        if (false_bus != "-" && true_bus != false_bus) {
            if (append_CYCLES(true_bus "@"))
                add_CYCLES(false_bus);
        }
    }
}

function generate_INST(op, nemo, opr, len,
                       true_bus, false_bus, true_seq, false_seq, cycle) {
    if (GENERATE_INST == 0)
        return;
    if (false_bus == "-" || true_bus == false_bus) {
        cycle = CYCLES_index(true_bus);
    } else {
        cycle = CYCLES_index(true_bus "@");
    }
    printf("        E(%d, %d),  // %s: %-4s %-7s; %-20s%s\n",
           len, cycle, op, nemo, opr, true_seq, false_seq);
}

BEGIN {
    pretty_print("op", "nemo", "opr", "#",
                 "tbus", "fbus",
                 "true_seq", "false_seq");
    pretty_print("--", "---", "----", "-",
                 "---", "---", 
                 "--------", "---------");
}
$1 !~ /[0-9A-F][0-9A-F]/ { next; }
$1 ~ /[0-9A-F][0-9A-F]/ {
    op = $1;
    nemo = $2;
    opr = $3;
    len = $4;
    true_seq = $7;
    false_seq = $8;

    true_bus = bus_cycles(true_seq);
    true_pre = find_prefetch(true_bus);

    false_bus = bus_cycles(false_seq);
    false_pre = find_prefetch(false_bus);

    pretty_print(op, nemo, opr, len, true_bus, false_bus, true_seq, false_seq);
    generate_CYCLES(true_bus, false_bus);
    generate_INST(op, nemo, opr, len, true_bus, false_bus, true_seq, false_seq);
}
