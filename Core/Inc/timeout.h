#pragma once

#include <stdint.h>

typedef struct timeout_t {
    uint32_t timeout;
    uint32_t last;
    uint32_t value;
} timeout_t;

uint8_t check_timeout(timeout_t *timeout);
void reset_timeout(timeout_t *timeout);