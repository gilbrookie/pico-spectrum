#include "debug_usb.h"
#include <stdio.h>
#include <stdbool.h>

void debug_print_bands(const float *b) {
    printf("B:");
    for (int i = 0; i < 16; i++) printf(" %.2f", b[i]);
    printf("\n");
}

void debug_handle_cmd(int c, float *mix, bool *bypass) {
    if (c == '+') *mix += 0.05f;
    if (c == '-') *mix -= 0.05f;
    if (c == 'b') *bypass = !*bypass;
    if (*mix < 0) *mix = 0;
    if (*mix > 1) *mix = 1;
}
