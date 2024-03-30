#include "lcd_menu.h"

#include <memory.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

uint16_t lms_get_id() {
    static uint16_t id = 0;
    return id++;
}

// void lms_set_menu_page(LMSContext *ctx, LMSPage *page) {
//     ctx->vx = 0;
//     ctx->vy = 0;
//     ctx->current_page = page;
// }

void lms_set_menu_focus(LMSContext *ctx, LMSObj *object) {
    if (object != NULL && object->enabled) {
        LMSPage *to_focus;
        switch (object->object.type) {
            case LMSTxt_t:
                LMSTxt *txt = (LMSTxt *)object->object.data;
                to_focus = txt->parent;
                break;
            case LMSNumSel_t:
                LMSNumSel *num = (LMSNumSel *)object->object.data;
                to_focus = num->parent;
                break;
            case LMSBtn_t:
                LMSBtn *btn = (LMSBtn *)object->object.data;
                to_focus = btn->parent;
                break;
            case LMSPage_t:
                to_focus = (LMSPage *)object->object.data;
                break;
            default:
                to_focus = NULL;
                break;
        }
        if (to_focus != NULL) {
            ctx->focus = object;
            if (ctx->current_page != to_focus) { // Swap to new page
                ctx->vx = 0;
                ctx->vy = 0;
                ctx->refresh = 1;
                ctx->current_page = to_focus;
                if (to_focus->callback != NULL) {
                    to_focus->callback(to_focus);
                }
                // lms_set_menu_page(ctx, to_focus);
            }
        }
    }
}

void lms_default_tr_enter(LMSObj *object) {
    if (object->object.type == LMSBtn_t) {
        LMSBtn *btn = (LMSBtn *)object->object.data;
        if (btn->callback != NULL && !btn->callback(btn))
            return;
    }
    lms_default_tr_next(object);
}

void lms_default_tr_up(LMSObj *object) {
    if (object->object.type == LMSNumSel_t) {
        LMSNumSel *num = (LMSNumSel *)object->object.data;
        num->number = (num->number + 1) % 10;
        *num->ext_num = num->number;
        // } else if (object->object.type == LMSPage_t) {
        //     object->ctx->vy++;
        //     object->ctx->refresh = 1;
    } else {
        lms_set_menu_focus(object->ctx, object->up);
    }
}

void lms_default_tr_down(LMSObj *object) {
    if (object->object.type == LMSNumSel_t) {
        LMSNumSel *num = (LMSNumSel *)object->object.data;
        num->number--;
        if (num->number < 0)
            num->number = 9;
        *num->ext_num = num->number;
        // } else if (object->object.type == LMSPage_t) {
        //     object->ctx->vy--;
        //     object->ctx->refresh = 1;
    } else {
        lms_set_menu_focus(object->ctx, object->down);
    }
}

void lms_default_tr_back(LMSObj *object) {
    lms_set_menu_focus(object->ctx, object->back);
}

void lms_default_tr_next(LMSObj *object) {
    lms_set_menu_focus(object->ctx, object->next);
}

void lms_default_tr_cancel(LMSObj *object) {
    lms_set_menu_focus(object->ctx, object->ctx->root_page->base);
}

void lms_default_tr_draw(LMSObj *object) {
    coord x = object->x - object->ctx->vx;
    coord y = object->y - object->ctx->vy;
    uint8_t focus = object->ctx->focus == object;
    switch (object->object.type) {
        case LMSTxt_t:
            LMSTxt *txt = (LMSTxt *)object->object.data;
            lcdGFX_print_string(object->ctx->gfx, x, y, txt->str, txt->pad);
            break;
        case LMSNumSel_t:
            LMSNumSel *num = (LMSNumSel *)object->object.data;
            lcdGFX_print_char(object->ctx->gfx, x, y, focus ? num->up_char_focus : num->up_char_nofocus);
            lcdGFX_print_char(object->ctx->gfx, x, y + 2, focus ? num->down_char_focus : num->down_char_nofocus);
            lcdGFX_print_char(object->ctx->gfx, x, y + 1, '0' + num->number);
            break;
        case LMSBtn_t:
            LMSBtn *btn = (LMSBtn *)object->object.data;
            lcdGFX_draw_box(object->ctx->gfx, x, y, object->w, object->h, focus ? btn->char_focus : btn->char_nofocus);
            break;
        case LMSPage_t:
            // TODO: cut drawn items by page width and height
            LMSPage *page = (LMSPage *)object->object.data;
            // lcdGFX_print_string(object->ctx->gfx, x, y, object->name, 10);
            for (size_t i = 0; i < page->len; i++) {
                lms_default_tr_draw(page->children[i]);
            }
            break;
        default:
            break;
    }
}

LMSObj *lms_new_obj(LMSContext *ctx, void *obj_data, LMSObjectType obj_type, const char *name, coord width, coord height, uint8_t enabled) {
    LMSObj *obj = malloc(sizeof(LMSObj));
    obj->id = lms_get_id();
    obj->x = 0;
    obj->y = 0;
    obj->z = 0;
    obj->w = width;
    obj->h = height;
    obj->enabled = !!enabled;
    obj->name = name;

    obj->ctx = ctx;

    obj->object.data = obj_data;
    obj->object.type = obj_type;

    obj->next = NULL;
    obj->back = NULL;
    obj->up = NULL;
    obj->down = NULL;

    obj->enter_tr = lms_default_tr_enter;
    obj->up_tr = lms_default_tr_up;
    obj->down_tr = lms_default_tr_down;
    obj->back_tr = lms_default_tr_back;
    obj->next_tr = lms_default_tr_next;
    obj->cancel_tr = lms_default_tr_cancel;
    obj->draw_tr = lms_default_tr_draw;

    return obj;
}

void lms_free_obj(LMSObj *obj) {
    free(obj);
}

LMSPage *lms_new_page(LMSContext *ctx, const char *name, coord width, coord height, lms_page_callback on_focus) { // on_focus can be NULL
    LMSPage *page = malloc(sizeof(LMSPage));
    page->children = calloc(1, sizeof(LMSObj *));
    page->len = 0;
    page->callback = on_focus;

    page->base = lms_new_obj(ctx, (void *)page, LMSPage_t, name, width, height, 1);

    return page;
}

void lms_free_page(LMSPage *page) {
    free(page->children);
    free(page->base);
    free(page);
    // TODO : free all children
}

void lms_insert_child(LMSPage *page, LMSObj *obj) {
    page->children = realloc(page->children, sizeof(LMSObj *) * (++page->len));
    page->children[page->len - 1] = NULL;
    for (size_t i = 0; i < page->len; i++) { // Keep child array sorted
        LMSObj *sel = page->children[i];
        if (sel == NULL) {
            page->children[i] = obj;
            return;
        } else if (sel->z > obj->z) {
            page->children[i] = obj;
            obj = sel;
        }
    }
    // page->children[page->len - 1] = obj;
}

LMSTxt *lms_new_text(LMSPage *page, const char *text, uint8_t padding) {
    LMSTxt *txt = malloc(sizeof(LMSTxt));
    txt->len = strlen(text);
    txt->str = text;
    txt->pad = padding;
    txt->parent = page;

    txt->base = lms_new_obj(page->base->ctx, (void *)txt, LMSTxt_t, text, txt->len, 1, 0);
    txt->base->z = LMS_PAGE_LAYERS;

    lms_insert_child(page, txt->base);

    return txt;
}

void lms_free_text(LMSTxt *txt) {
    free(txt);
}

LMSNumSel *lms_new_num_sel(LMSPage *page, const char *name, int8_t *ext_num) {
    LMSNumSel *num = malloc(sizeof(LMSNumSel));
    num->number = 0;
    num->ext_num = ext_num;
    num->parent = page;

    num->base = lms_new_obj(page->base->ctx, (void *)num, LMSNumSel_t, name, 1, 3, 1);
    num->base->enter_tr = lms_default_tr_next;
    num->base->z = LMS_PAGE_LAYERS * 2 / 3;

    num->up_char_focus = page->base->ctx->default_chars.num.up.focus;
    num->down_char_focus = page->base->ctx->default_chars.num.down.focus;
    num->up_char_nofocus = page->base->ctx->default_chars.num.up.nofocus;
    num->down_char_nofocus = page->base->ctx->default_chars.num.down.nofocus;

    lms_insert_child(page, num->base);

    return num;
}

void lms_free_num_sel(LMSNumSel *num) {
    free(num);
}

LMSBtn *lms_new_btn(LMSPage *page, const char *name, coord width, coord height, lms_btn_callback on_enter) { // Callback can be NULL
    LMSBtn *btn = malloc(sizeof(LMSBtn));
    btn->callback = on_enter;
    btn->parent = page;

    btn->base = lms_new_obj(page->base->ctx, (void *)btn, LMSBtn_t, name, 1, 3, 1);
    btn->base->z = LMS_PAGE_LAYERS / 3;
    btn->base->w = width;
    btn->base->z = height;

    btn->char_focus = page->base->ctx->default_chars.btn.focus;
    btn->char_nofocus = page->base->ctx->default_chars.btn.nofocus;

    lms_insert_child(page, btn->base);

    return btn;
}

void lms_free_btn(LMSBtn *btn) {
    free(btn);
}

LMSContext *lms_new_menu(LcdGFX *gfx) {
    LMSContext *ctx = malloc(sizeof(LMSContext));
    ctx->vw = gfx->lcd->columns;
    ctx->vh = gfx->lcd->rows;
    ctx->vx = 0;
    ctx->vy = 0;
    ctx->root_page = NULL;
    ctx->current_page = NULL;
    ctx->focus = NULL;
    ctx->gfx = gfx;
    ctx->signaled = 0;
    memset(ctx->signals, 0, sizeof(uint8_t) * LMS_SIGNAL_COUNT);

    ctx->default_chars.btn.focus = 255;
    ctx->default_chars.btn.nofocus = '#';

    ctx->default_chars.num.up.focus = '^';
    ctx->default_chars.num.down.focus = 'v';
    ctx->default_chars.num.up.nofocus = '|';
    ctx->default_chars.num.down.nofocus = '|';

    return ctx;
}

LMSContext *lms_initialize_menu(LMSContext *ctx, LMSPage *root_page) {
    ctx->root_page = root_page;
    ctx->current_page = root_page;
    ctx->focus = root_page->base;

    lcdGFX_initialize(ctx->gfx);

    return ctx;
}

void lms_free_menu(LMSContext *ctx) {
    // TODO : free entire menu
    free(ctx);
}

void lms_signal_menu(LMSContext *ctx, LMSSignal signal) {
    ctx->signaled = 1;
    ctx->signals[signal] = 1;
}

#define LMS_CONCAT(a, b) a##b
#define LMS_UPDATE_CALL(_call)                                                        \
    if (ctx->focus != NULL) {                                                         \
        if (ctx->focus->LMS_CONCAT(_call, _tr) == NULL)                               \
            ctx->current_page->base->LMS_CONCAT(_call, _tr)(ctx->current_page->base); \
        else                                                                          \
            ctx->focus->LMS_CONCAT(_call, _tr)(ctx->focus);                           \
    }

void lms_update_menu(LMSContext *ctx) {
    if (ctx->signaled) {
        for (size_t i = 0; i < LMS_SIGNAL_COUNT; i++) {
            if (ctx->signals[i]) {
                switch (i) {
                    case ENTER:
                        LMS_UPDATE_CALL(enter);
                        break;
                    case UP:
                        LMS_UPDATE_CALL(up);
                        break;
                    case DOWN:
                        LMS_UPDATE_CALL(down);
                        break;
                    case BACK:
                        LMS_UPDATE_CALL(back);
                        break;
                    case NEXT:
                        LMS_UPDATE_CALL(next);
                        break;
                    case CANCEL:
                        LMS_UPDATE_CALL(cancel);
                        break;
                    default:
                        break;
                }
                ctx->signals[i] = 0;
            }
        }
    }

    if (ctx->refresh) {
        ctx->refresh = 0;
        lcdGFX_clear_buffer(ctx->gfx);
    }

    ctx->current_page->base->draw_tr(ctx->current_page->base);
    lcdGFX_update(ctx->gfx);
}
