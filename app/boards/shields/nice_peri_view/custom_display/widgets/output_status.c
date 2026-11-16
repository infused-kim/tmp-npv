/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include "output_status.h"
#include "util.h"
#include <zmk/events/split_peripheral_status_changed.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct output_status_state
{
    struct zmk_endpoint_instance selected_endpoint;
    bool active_profile_connected;
    bool active_profile_bonded;
};

static struct output_status_state get_state(const zmk_event_t *_eh)
{
    return (struct output_status_state){.selected_endpoint = zmk_endpoint_get_selected(),
                                        .active_profile_connected =
                                            zmk_ble_active_profile_is_connected(),
                                        .active_profile_bonded = !zmk_ble_active_profile_is_open()};
    ;
}

static void set_status_symbol(struct zmk_widget_output_status *widget, struct output_status_state state)
{
    char text[20] = {};

    switch (state.selected_endpoint.transport)
    {
    case ZMK_TRANSPORT_USB:
        strcat(text, LV_SYMBOL_USB);
        break;
    case ZMK_TRANSPORT_BLE:
        if (state.active_profile_bonded)
        {
            if (state.active_profile_connected)
            {
                snprintf(text, sizeof(text), LV_SYMBOL_WIFI " %i " LV_SYMBOL_OK,
                         state.selected_endpoint.ble.profile_index + 1);
            }
            else
            {
                snprintf(text, sizeof(text), LV_SYMBOL_WIFI " %i " LV_SYMBOL_CLOSE,
                         state.selected_endpoint.ble.profile_index + 1);
            }
        }
        else
        {
            snprintf(text, sizeof(text), LV_SYMBOL_WIFI " %i " LV_SYMBOL_SETTINGS,
                     state.selected_endpoint.ble.profile_index + 1);
        }
        break;
    case ZMK_TRANSPORT_NONE:
        break;
    }
    lv_canvas_set_buffer(widget->canvas, widget->cbuf, OUTPUT_CANVAS_WIDTH, OUTPUT_CANVAS_HEIGHT,
                         CANVAS_COLOR_FORMAT);
    lv_canvas_fill_bg(widget->canvas, LVGL_BACKGROUND, LV_OPA_COVER);
    canvas_draw_text(widget->canvas, 0, 0, OUTPUT_CANVAS_WIDTH, &widget->output_label_dsc, text);
    rotate_canvas(widget->canvas, widget->cbuf_rot, OUTPUT_CANVAS_WIDTH, OUTPUT_CANVAS_HEIGHT);
}

static void output_status_update_cb(struct output_status_state state)
{
    struct zmk_widget_output_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_status_symbol(widget, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
// We don't get an endpoint changed event when the active profile connects/disconnects
// but there wasn't another endpoint to switch from/to, so update on BLE events too.
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

int zmk_widget_output_status_init(struct zmk_widget_output_status *widget, lv_obj_t *parent)
{
    sys_slist_append(&widgets, &widget->node);
    widget->canvas = lv_canvas_create(parent);
    init_label_dsc(&widget->output_label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT);

    widget_output_status_init();
    return 0;
}

lv_obj_t *zmk_widget_output_status_obj(struct zmk_widget_output_status *widget)
{
    return widget->canvas;
}