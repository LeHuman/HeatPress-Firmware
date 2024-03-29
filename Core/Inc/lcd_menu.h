#pragma once

#include <stdint.h>

#include "lcd_gfx.h"
#include "main.h"

#define LMS_PAGE_LAYERS 5

#define __LMS_SET_DIR(lms_object, next_lms_object, __dir) lms_object->base->__dir = next_lms_object->base
#define LMS_SET_NEXT(lms_object, next_lms_object) __LMS_SET_DIR(lms_object, next_lms_object, next)
#define LMS_SET_BACK(lms_object, next_lms_object) __LMS_SET_DIR(lms_object, next_lms_object, back)
#define LMS_SET_UP(lms_object, next_lms_object) __LMS_SET_DIR(lms_object, next_lms_object, up)
#define LMS_SET_DOWN(lms_object, next_lms_object) __LMS_SET_DIR(lms_object, next_lms_object, down)
#define LMS_SET_POS(lms_object, _x, _y) \
    lms_object->base->x = _x;           \
    lms_object->base->y = _y

/**
 * @brief LMS Object types
 *
 * Each object has a set graphic area to display
 *
 * Each object has the option to be selectable [Enabled], when this is enabled, depending on the type, only certain triggers can be defined.
 *
 * LMS can trigger objects with a [Enter], [Up], [Down], [Back], [Next], or [Cancel] Signal.
 * An example of a trigger can be a GPIO interrupt from a button to go [Next].
 *
 * Objects that don't define a trigger but are signaled when selected will relay the signal to their parent object.
 * Objects also have the option to relay the signal after processing it.
 * Objects hold references to other objects that up, down, before (back), or next to it.
 * Objects, by default for directional signals (up, down, back, next), move focus to the next object referenced, if available.
 *
 * Page objects should define all triggers. Pages are the only ones that can take other objects as children.
 * Pages cannot be children of other pages. Whenever a page is set to be viewed it replaces the current page.
 * Pages are always enabled.
 *
 * Be weary of soft-locking yourself into a menu, unless you want to.
 */
typedef enum LMSObjectType {
    // LMSObj_t,    // The base LMS object
    LMSTxt_t,    // Basic line of text, has [Enter], [Back], and [Next]
    LMSBtn_t,    // Button that can be selected and pressed, has [Enter], [Back], and [Next]
    LMSNumSel_t, // Selector wheel for a single digit, has [Up], [Down], [Back], and [Next]
    LMSPage_t,   // Pages define a space for other objects, can take children objects, has all triggers
} LMSObjectType;

typedef enum LMSSignal {
    ENTER,
    UP,
    DOWN,
    BACK,
    NEXT,
    CANCEL,
    LMS_SIGNAL_COUNT,
} LMSSignal;

/**
 * @brief LMS Modules
 *
 * Modules are higher level implementations of the base objects
 *
 * Modules add additional functionality and provide an easier entry point to creating a menu
 *
 * They can be treated as a single object when placing onto a Page
 */
typedef enum LMSModule {
    LMSTxtFrame_t, // Block of text, text wraps until it fills up it's set space
    LMSTxtBtn_t,   // Button that has text overlayed on it. Size is made to fit text.
    LMSList_t,     // LMSPage_t that automatically sets [Up] and [Down] to select children. Auto scrolls up and down
} LMSModule;

typedef struct LMSContext LMSContext;
typedef struct LMSObj LMSObj;
typedef struct LMSPage LMSPage;
typedef struct LMSTxt LMSTxt;
typedef struct LMSBtn LMSBtn;
typedef struct LMSNumSel LMSNumSel;
typedef struct LMSContext LMSContext;

typedef void (*lms_trigger)(LMSObj *object);
typedef void (*lms_page_callback)(LMSPage *page); // Called when a page gains focus, either through a child object or itself
typedef uint8_t (*lms_btn_callback)(LMSBtn *btn); // Returns whether the [Next] object should be focused
typedef int8_t coord;

/**
 * @brief Call this function whenever an [Enter] signal should be sent.
 */
// void lms_trigger_enter() {
//     if (ot == LMSText_t) {
//         LMSTxt *txt = (LMSTxt *)obj;
//         if (txt->id == id) {
//         }
//     }
// }

typedef struct LMSObj {
    uint16_t id;      // unique id
    coord x, y;       // Position on page, origin on top left
    uint8_t z;        // layer height 0-7
    coord w, h;       // Size

    uint8_t enabled;  // Enabled state
    const char *name; // Delimited debug string name

    LMSContext *ctx;  // Context this object belongs to

    LMSObj *next;     // The next object attached to this object going right
    LMSObj *back;     // The previous object attached to this object going left, defaults to parent
    LMSObj *up;       // The next object attached to this object going up
    LMSObj *down;     // The previous object attached to this object going down

    struct object {
        void *data;
        LMSObjectType type;
    } object;

    lms_trigger enter_tr;
    lms_trigger up_tr;
    lms_trigger down_tr;
    lms_trigger back_tr;
    lms_trigger next_tr;
    lms_trigger cancel_tr;
    lms_trigger draw_tr; // Called when object should be drawn
} LMSObj;

typedef struct LMSPage {
    LMSObj *base;               // Base LMS object
    lms_page_callback callback; // Called when this page gains focus
    LMSObj **children;          // Children objects
    uint8_t len;                // Number of children
} LMSPage;

typedef struct LMSTxt {
    LMSObj *base;    // Base LMS object
    LMSPage *parent; // Parent Page this object belongs to
    const char *str;
    uint8_t len;     // Length without delimeter
    uint8_t pad;     // Padding for the text
} LMSTxt;

typedef struct LMSBtn {
    LMSObj *base;              // Base LMS object
    LMSPage *parent;           // Parent Page this object belongs to
    lms_btn_callback callback; // The callback for this button being pressed
} LMSBtn;

typedef struct LMSNumSel {
    LMSObj *base;    // Base LMS object
    LMSPage *parent; // Parent Page this object belongs to
    int8_t number;   // The current number for this selector
} LMSNumSel;

typedef struct LMSContext {
    coord vw, vh;          // Viewport width and height
    coord vx, vy;          // Viewport position, top left
    LMSPage *root_page;    // The root page that is returned to by default
    LMSPage *current_page; // The current page being displayed
    LMSObj *focus;         // The current object that is being focused

    uint8_t signaled;
    uint8_t signals[LMS_SIGNAL_COUNT];

    LcdGFX *gfx; // Graphics context object
} LMSContext;

void lms_default_tr_enter(LMSObj *object);
void lms_default_tr_up(LMSObj *object);
void lms_default_tr_down(LMSObj *object);
void lms_default_tr_back(LMSObj *object);
void lms_default_tr_next(LMSObj *object);
void lms_default_tr_cancel(LMSObj *object);
void lms_default_tr_draw(LMSObj *object);

LMSNumSel *lms_new_num_sel(LMSPage *page, const char *name);
LMSTxt *lms_new_text(LMSPage *page, const char *text, uint8_t padding);
LMSBtn *lms_new_btn(LMSPage *page, const char *name, coord width, coord height, lms_btn_callback on_enter);
LMSPage *lms_new_page(LMSContext *ctx, const char *name, coord width, coord height, lms_page_callback on_focus);

LMSContext *lms_new_menu(LcdGFX *gfx);
LMSContext *lms_initialize_menu(LMSContext *ctx, LMSPage *root_page);

// Call this to trigger a signal
void lms_signal_menu(LMSContext *ctx, LMSSignal signal);

void lms_update_menu(LMSContext *ctx);

void lms_free_page(LMSPage *page);
void lms_free_text(LMSTxt *txt);
void lms_free_num_sel(LMSNumSel *num);
void lms_free_btn(LMSBtn *btn);
void lms_free_menu(LMSContext *ctx);