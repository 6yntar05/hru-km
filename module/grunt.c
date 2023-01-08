#include "grunt.h"

// Symbols ranges:
    // Lat: 1-byte
    #define LAT_U_BEGIN 0x41
    #define LAT_U_END 0x5A
    #define LAT_L_BEGIN 0x61
    #define LAT_L_END 0x7A
    // Cyr: 2-byte (Later)
    #define CYR_PREFIX_UL_1 0xD0
    #define CYR_U_BEGIN 0x90
    #define CYR_U_END 0xAF
    #define CYR_L1_BEGIN 0xB0
    #define CYR_L1_END 0xBF
    #define CYR_PREFIX_LL_2 0xD1
    #define CYR_L2_BEGIN 0x80
    #define CYR_L2_END 0x8F

// From hru.o
extern char* fill_with;
extern int buffer_size;
int fill_size;

int walk_for_end(char* buffer) {
    int string_size = 0;
    for (;;) {
        if ((!buffer[string_size]) || (string_size >= buffer_size))
            break;
        string_size++;
    }
    return string_size;
}

void HRUNEFY_BUFFER(char *buffer, int *len) {
    int local_counter = 0;

    for (int i = 0; i < (*len)-1; i++) {
        if (buffer[i] == ' ') {
            local_counter = 0;
        } else {
            buffer[i] = fill_with[local_counter];
            if (local_counter < (fill_size - 1))
                local_counter++;
        }
    }
}