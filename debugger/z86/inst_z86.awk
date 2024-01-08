#!/usr/bin/gawk -f
# -*- mode: awk; -*-

BEGIN {
    PRETTY_PRINT = 0;
    GENERATE_TABLE = 1;
}

function print_number(n, len, num, rem) {
    len = match(n, /^[0-9]+/);
    if (n == "-") {
        printf(" -    ");
    } else if (len == 0) {
        printf("%-6s", n);
    } else if (length(n) == RLENGTH) {
        printf("%2d    ", n);
    } else {
        num = substr(n, RSTART, RLENGTH);
        rem = length(n) - RLENGTH;
        printf("%2d%s", num, substr(n, RLENGTH+1));
        for (num = rem; num <= 3; ++num)
            printf(" ");
    }
}

function pretty_print(opc, mne, opr, clk, len, ext, stk) {
    if (PRETTY_PRINT == 0)
        return;
    printf("%-2s  %-4s  %-6s  ", opc, mne, opr);
    print_number(clk);
    printf("  %s  %2s  %2s\n", len, ext, stk);
}

function generate_TABLE(op, mne, opr, clk, len, ext, stk) {
    if (GENERATE_TABLE == 0)
        return;
    if (len == "-") {
        printf("        0,              // %2s:\n", opc);
    } else {
        printf("        E(%d, %d, %d),  // %2s: %-4s %s\n", len, ext, stk, opc, mne, opr);
    }
}

BEGIN {
    pretty_print("op", "meno", "oper", "clock", "#", "bus", "stack");
    pretty_print("--", "----", "------", "------", "-", "---", "-----");
    if (GENERATE_TABLE)
        printf("constexpr uint8_t INST_TABLE[] = {\n");
}
END {
    if (GENERATE_TABLE)
        printf("};\n");
}

$1 !~ /[0-9A-F][0-9A-F]/ { next; }
$1 ~ /[0-9A-F][0-9A-F]/ {
    opc = $1;
    mne = $2;
    opr = $3;
    clk = $4;
    len = $5;
    ext = $6;
    stk = $7;

    pretty_print(opc, mne, opr, clk, len, ext, stk);
    generate_TABLE(opc, mne, opr, clk, len, ext, stk);
}
