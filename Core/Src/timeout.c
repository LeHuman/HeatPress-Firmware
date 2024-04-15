#include "timeout.h"

#include "main.h"

uint8_t check_timeout(timeout_t *timeout) {
    uint32_t ticks = HAL_GetTick();
    if (timeout->last != 0)
        timeout->value += ticks - timeout->last;
    else
        timeout->value = 0;
    timeout->last = ticks;

    return timeout->value > timeout->timeout;
}

void reset_timeout(timeout_t *timeout) {
    timeout->last = HAL_GetTick();
    timeout->value = 0;
}