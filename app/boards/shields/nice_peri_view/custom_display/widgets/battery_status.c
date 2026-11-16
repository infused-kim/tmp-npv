/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/services/bas.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/display/widgets/battery_status.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/battery.h>
#include <zmk/split/central.h>

#include "battery_status.h"
#include "util.h"
#include "split_central_status.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct battery_state {
    uint8_t source;
    uint8_t level;
};
struct connection_status_state {
    bool connected;
};

// static lv_color_t battery_image_buffer[ZMK_SPLIT_BLE_PERIPHERAL_COUNT][5 * 8];

// static void draw_battery(lv_obj_t *canvas, uint8_t level) {
//     lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

//     lv_draw_rect_dsc_t rect_fill_dsc;
//     lv_draw_rect_dsc_init(&rect_fill_dsc);
//     rect_fill_dsc.bg_color = lv_color_white();

//     lv_canvas_set_px(canvas, 0, 0, lv_color_white());
//     lv_canvas_set_px(canvas, 0, 4, lv_color_white());

//     if (level > 90) {
//         // full
//     } else if (level > 70) {
//         lv_canvas_draw_rect(canvas, 2, 1, 1, 3, &rect_fill_dsc);
//     } else if (level > 50) {
//         lv_canvas_draw_rect(canvas, 2, 1, 2, 3, &rect_fill_dsc);
//     } else if (level > 30) {
//         lv_canvas_draw_rect(canvas, 2, 1, 3, 3, &rect_fill_dsc);
//     } else if (level > 10) {
//         lv_canvas_draw_rect(canvas, 2, 1, 4, 3, &rect_fill_dsc);
//     } else {
//         lv_canvas_draw_rect(canvas, 2, 1, 5, 3, &rect_fill_dsc);
//     }
// }

static void draw_batteries_widget(struct zmk_widget_batteries_status *widget) {
    lv_canvas_set_buffer(widget->canvas, widget->cbuf, BATTERY_CANVAS_WIDTH, BATTERY_CANVAS_HEIGHT,
                         CANVAS_COLOR_FORMAT);

    lv_canvas_fill_bg(widget->canvas, LVGL_BACKGROUND, LV_OPA_COVER);
    #if CONFIG_NICE_PERI_VIEW_RIGHT_SIDE_CENTRAL
    canvas_draw_text(widget->canvas, 35, 0, 33, &widget->central_label_dsc, widget->central_text);
    if (widget->connected) {
        canvas_draw_text(widget->canvas, 0, 0, 33, &widget->peripheral_label_dsc,
                         widget->peripheral_text);
    }
    #else
    canvas_draw_text(widget->canvas, 0, 0, 33, &widget->central_label_dsc, widget->central_text);
    if (widget->connected) {
        canvas_draw_text(widget->canvas, 35, 0, 33, &widget->peripheral_label_dsc,
                         widget->peripheral_text);
    }
    #endif
    rotate_canvas(widget->canvas, widget->cbuf_rot, BATTERY_CANVAS_WIDTH, BATTERY_CANVAS_HEIGHT);
}

static void set_battery_symbol(struct zmk_widget_batteries_status *widget,
                               struct battery_state state) {
    if (state.source == 0) {
        strcpy(widget->central_text, "");
        char perc[5] = {};
        snprintf(perc, sizeof(perc), "%3u%%", state.level);
        strcat(widget->central_text, perc);
    }
    if (state.source == 1) {
        strcpy(widget->peripheral_text, "");
        char perc[5] = {};
        snprintf(perc, sizeof(perc), "%3u%%", state.level);
        strcat(widget->peripheral_text, perc);
    }
    draw_batteries_widget(widget);
}

void battery_status_update_cb(struct battery_state state) {
    struct zmk_widget_batteries_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_symbol(widget, state); }
}

static struct battery_state peripheral_battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_peripheral_battery_state_changed *ev =
        (eh != NULL) ? as_zmk_peripheral_battery_state_changed(eh) : NULL;
    uint8_t level = 0;
    if (ev != NULL) {
        level = ev->state_of_charge;
    } else {
        zmk_split_central_get_peripheral_battery_level(0, &level);
    }
    LOG_DBG("Peripheral Charge: %3u%%", level);
    return (struct battery_state){
        .source = 1,
        .level = level,
    };
}

static struct battery_state battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    return (struct battery_state){
        .source = 0,
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_battery_status, struct battery_state,
                            battery_status_update_cb, peripheral_battery_status_get_state)

ZMK_SUBSCRIPTION(widget_peripheral_battery_status, zmk_peripheral_battery_state_changed);

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_state, battery_status_update_cb,
                            battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);

static void set_connection(struct zmk_widget_batteries_status *widget,
                           struct connection_status_state state) {
    widget->connected = state.connected;
    uint8_t level;
    zmk_split_central_get_peripheral_battery_level(0, &level);
    set_battery_symbol(widget, (struct battery_state){
                                   .source = 1,
                                   .level = level,
                               });
    draw_batteries_widget(widget);
}
static void connection_status_update_cb(struct connection_status_state state) {
    struct zmk_widget_batteries_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_connection(widget, state); }
}

static struct connection_status_state connection_status_get_state(const zmk_event_t *eh) {
    // eh is NULL when the widget initializes
    const struct zmk_split_peripheral_status_changed *ev =
        (eh != NULL) ? as_zmk_split_peripheral_status_changed(eh) : NULL;
    return (struct connection_status_state){
        .connected = (ev != NULL) ? ev->connected : nice_peri_view_peripheral_connected()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_connection_status, struct connection_status_state,
                            connection_status_update_cb, connection_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_connection_status, zmk_split_peripheral_status_changed);

int zmk_widget_batteries_status_init(struct zmk_widget_batteries_status *widget, lv_obj_t *parent) {
    sys_slist_append(&widgets, &widget->node);
    widget->canvas = lv_canvas_create(parent);

#if CONFIG_NICE_PERI_VIEW_RIGHT_SIDE_CENTRAL
    init_label_dsc(&widget->central_label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_12,
                   LV_TEXT_ALIGN_RIGHT);
    init_label_dsc(&widget->peripheral_label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_12,
                   LV_TEXT_ALIGN_LEFT);
#else
    init_label_dsc(&widget->central_label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_12,
                   LV_TEXT_ALIGN_LEFT);
    init_label_dsc(&widget->peripheral_label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_12,
                   LV_TEXT_ALIGN_RIGHT);
#endif

    widget_battery_status_init();
    widget_peripheral_battery_status_init();
    widget_battery_connection_status_init();

    return 0;
}

lv_obj_t *zmk_widget_batteries_status_obj(struct zmk_widget_batteries_status *widget) {
    return widget->canvas;
}