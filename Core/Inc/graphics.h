#pragma once

#include <stdint.h>

#define __________ 0b00000
#define HH________ 0b10000
#define __HH______ 0b01000
#define HHHH______ 0b11000
#define ____HH____ 0b00100
#define HH__HH____ 0b10100
#define __HHHH____ 0b01100
#define HHHHHH____ 0b11100
#define ______HH__ 0b00010
#define HH____HH__ 0b10010
#define __HH__HH__ 0b01010
#define HHHH__HH__ 0b11010
#define ____HHHH__ 0b00110
#define HH__HHHH__ 0b10110
#define __HHHHHH__ 0b01110
#define HHHHHHHH__ 0b11110
#define ________HH 0b00001
#define HH______HH 0b10001
#define __HH____HH 0b01001
#define HHHH____HH 0b11001
#define ____HH__HH 0b00101
#define HH__HH__HH 0b10101
#define __HHHH__HH 0b01101
#define HHHHHH__HH 0b11101
#define ______HHHH 0b00011
#define HH____HHHH 0b10011
#define __HH__HHHH 0b01011
#define HHHH__HHHH 0b11011
#define ____HHHHHH 0b00111
#define HH__HHHHHH 0b10111
#define __HHHHHHHH 0b01111
#define HHHHHHHHHH 0b11111

static const uint8_t sprite_blank[8] = {
    __________,
    __________,
    __________,
    __________,
    __________,
    __________,
    __________,
    __________,
};
static const uint8_t sprite_celsius[8] = {
    ______HH__,
    ____HH__HH,
    ______HH__,
    __________,
    __________,
    __________,
    __________,
    __________,
};

static const uint8_t sprite_alert[8] = {
    __HHHHHH__,
    __HHHHHH__,
    __HHHHHH__,
    __HHHHHH__,
    ____HH____,
    __________,
    __HHHHHH__,
    __HHHHHH__,
};

static const uint8_t sprite_anim_alert[8][8] = {
    {
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        ____HH____,
        __________,
        __HHHHHH__,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        ____HH____,
        __________,
        __HHHHHH__,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        ____HH____,
        __________,
        __HHHHHH__,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        ____HH____,
        __________,
        __HHHHHH__,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        ____HH____,
        __________,
        __HHHHHH__,
        __HHHHHH__,
    },
    {
        ____HH____,
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        ____HH____,
        __________,
        __HHHHHH__,
        __HHHHHH__,
    },
    // {
    //     __________,
    //     __HHHHHH__,
    //     __HHHHHH__,
    //     __HHHHHH__,
    //     ____HH____,
    //     __________,
    //     ____HH____,
    //     __HHHHHH__,
    // },
    {
        __________,
        __________,
        ____HH____,
        __HHHHHH__,
        __HHHHHH__,
        ____HH____,
        __________,
        __HHHHHH__,
    },
    // {
    //     __HHHHHH__,
    //     __HHHHHH__,
    //     __HHHHHH__,
    //     ____HH____,
    //     ____HH____,
    //     __________,
    //     ____HH____,
    //     ____HH____,
    // },
    {
        __HHHHHH__,
        __HHHHHH__,
        __HHHHHH__,
        ____HH____,
        ____HH____,
        __________,
        ____HH____,
        __HHHHHH__,
    },
};

static const uint8_t sprite_hot[8] = {
    HH____HH__,
    __HH____HH,
    __HH____HH,
    HH____HH__,
    HH____HH__,
    __HH____HH,
    __________,
    HHHHHHHHHH,
};

static const uint8_t sprite_anim_hot[8][8] = {
    {
        HH____HH__,
        __HH____HH,
        __HH____HH,
        HH____HH__,
        HH____HH__,
        __HH____HH,
        __________,
        HHHHHHHHHH,
    },
    {
        HH____HH__,
        __HH____HH,
        __HH____HH,
        HH____HH__,
        HH____HH__,
        __HH____HH,
        __________,
        HHHHHHHHHH,
    },
    {
        HH____HH__,
        __HH____HH,
        __HH____HH,
        HH____HH__,
        HH____HH__,
        __HH____HH,
        __________,
        HHHHHHHHHH,
    },
    {
        HH______HH,
        HH______HH,
        __HH__HH__,
        HH____HH__,
        HH______HH,
        HH______HH,
        __________,
        HHHHHHHHHH,
    },
    {
        __HH__HH__,
        HH______HH,
        HH______HH,
        __HH__HH__,
        HH______HH,
        __HH____HH,
        __________,
        HHHHHHHHHH,
    },
    {
        __HH__HH__,
        HH______HH,
        __HH____HH,
        __HH__HH__,
        HH____HH__,
        __HH____HH,
        __________,
        HHHHHHHHHH,
    },
    {
        __HH__HH__,
        HH____HH__,
        __HH____HH,
        __HH__HH__,
        HH______HH,
        HH______HH,
        __________,
        HHHHHHHHHH,
    },
    {
        HH____HH__,
        __HH____HH,
        __HH____HH,
        __HH__HH__,
        HH____HH__,
        __HH____HH,
        __________,
        HHHHHHHHHH,
    },
};

static const uint8_t sprite_voltage[8] = {
    ________HH,
    ____HHHH__,
    HHHHHH____,
    __HHHHHHHH,
    ____HHHH__,
    __HHHH____,
    __HH______,
    HH________,
};

// TODO: Substitute with 0b11011011?
// static const uint8_t sprite_unselected[8] = {
//     __________,
//     __HHHHHH__,
//     HH______HH,
//     HH______HH,
//     HH______HH,
//     HH______HH,
//     __HHHHHH__,
//     __________,
// };
static const uint8_t sprite_unselected[8] = {
    __HHHHHH__,
    HH______HH,
    HH______HH,
    HH______HH,
    HH______HH,
    HH______HH,
    HH______HH,
    __HHHHHH__,
};

// static const uint8_t sprite_selected[8] = {
//     __________,
//     __HHHHHH__,
//     HH______HH,
//     HH__HH__HH,
//     HH__HH__HH,
//     HH______HH,
//     __HHHHHH__,
//     __________,
// };

static const uint8_t sprite_anim_selected[8][8] = {
    {
        __HHHHHH__,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        HH______HH,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        __HHHHHH__,
    },
    {
        __HHHHHH__,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        HHHHHHHHHH,
        __HHHHHH__,
    },
};

static const uint8_t sprite_up[8] = {
    __________,
    ____HH____,
    __HH__HH__,
    HH______HH,
    __________,
    ____HH____,
    __HH__HH__,
    HH______HH,
};

static const uint8_t sprite_down[8] = {
    __________,
    HH______HH,
    __HH__HH__,
    ____HH____,
    __________,
    HH______HH,
    __HH__HH__,
    ____HH____,
};

static const uint8_t sprite_anim_up[8][8] = {
    {
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
    },
    {
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
    },
    {
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
    },
    {
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
    },
    {
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
    },
    {
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
    },
    {
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
    },
    {
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
        __________,
        ____HH____,
        __HH__HH__,
        HH______HH,
    },
};

static const uint8_t sprite_anim_down[8][8] = {
    {
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
    },
    {
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
    },
    {
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
    },
    {
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
    },
    {
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
    },
    {
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
    },
    {
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
    },
    {
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
        HH______HH,
        __HH__HH__,
        ____HH____,
        __________,
    },
};

static const uint8_t sprite_anim_loading[8][8] = {
    {
        __________,
        __________,
        HH________,
        HH________,
        HH________,
        HH________,
        __HH______,
        __________,
    },
    {
        __________,
        __HH______,
        HH________,
        HH________,
        __________,
        HH________,
        __________,
        __________,
    },
    {
        __________,
        __HHHHHH__,
        HH________,
        HH________,
        HH________,
        __________,
        __________,
        __________,
    },
    {
        __________,
        __HHHHHH__,
        ________HH,
        ________HH,
        __________,
        __________,
        __________,
        __________,
    },
    {
        __________,
        __HH__HH__,
        ________HH,
        ________HH,
        ________HH,
        ________HH,
        __________,
        __________,
    },
    {
        __________,
        ____HHHH__,
        __________,
        ________HH,
        ________HH,
        ________HH,
        ______HH__,
        __________,
    },
    {
        __________,
        __________,
        __________,
        __________,
        ________HH,
        ________HH,
        __HHHHHH__,
        __________,
    },
    {
        __________,
        __________,
        __________,
        HH________,
        HH________,
        HH______HH,
        __HHHH____,
        __________,
    },
};

/**
 * __________ __________ __________ __________
 * __________ __________ __________ __________
 * __________ __________ __________ __________
 * ____HHHHHH ______HHHH HH________ HHHHHHHH__
 * __HH______ ______HH__ __HH______ HH______HH
 * HH________ HH____HH__ ____HH____ HH______HH
 * HH________ HH____HH__ ____HH____ HH______HH
 * HH________ HH____HH__ ____HH____ HHHHHHHH__
 * __________ __________ __________ __________
 * HH________ HH____HH__ ____HH____ HH________
 * __HH______ ______HH__ __HH______ HH________
 * ____HHHHHH ______HHHH HH________ HH________
 * __________ __________ __________ __________
 * __________ __________ __________ __________
 * __________ __________ __________ __________
 * __________ __________ __________ __________
 */

static const uint8_t graphic_odp[8][8] = {
    {
        __________,
        __________,
        __________,
        ____HHHHHH,
        __HH______,
        HH________,
        HH________,
        HH________,
    },
    {
        __________,
        __________,
        __________,
        ______HHHH,
        ______HH__,
        HH____HH__,
        HH____HH__,
        HH____HH__,
    },
    {
        __________,
        __________,
        __________,
        HH________,
        __HH______,
        ____HH____,
        ____HH____,
        ____HH____,
    },
    {
        __________,
        __________,
        __________,
        HHHHHHHH__,
        HH______HH,
        HH______HH,
        HH______HH,
        HHHHHHHH__,
    },
    {
        __________,
        HH________,
        __HH______,
        ____HHHHHH,
        __________,
        __________,
        __________,
        __________,
    },
    {
        __________,
        HH____HH__,
        ______HH__,
        ______HHHH,
        __________,
        __________,
        __________,
        __________,
    },
    {
        __________,
        ____HH____,
        __HH______,
        HH________,
        __________,
        __________,
        __________,
        __________,
    },
    {
        __________,
        HH________,
        HH________,
        HH________,
        __________,
        __________,
        __________,
        __________,
    },
};

#undef __________
#undef HH________
#undef __HH______
#undef HHHH______
#undef ____HH____
#undef HH__HH____
#undef __HHHH____
#undef HHHHHH____
#undef ______HH__
#undef HH____HH__
#undef __HH__HH__
#undef HHHH__HH__
#undef ____HHHH__
#undef HH__HHHH__
#undef __HHHHHH__
#undef HHHHHHHH__
#undef ________HH
#undef HH______HH
#undef __HH____HH
#undef HHHH____HH
#undef ____HH__HH
#undef HH__HH__HH
#undef __HHHH__HH
#undef HHHHHH__HH
#undef ______HHHH
#undef HH____HHHH
#undef __HH__HHHH
#undef HHHH__HHHH
#undef ____HHHHHH
#undef HH__HHHHHH
#undef __HHHHHHHH
#undef HHHHHHHHHH