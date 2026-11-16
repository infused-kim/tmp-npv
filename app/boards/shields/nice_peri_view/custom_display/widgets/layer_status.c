/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include "layer_status.h"
#include "util.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct layer_status_state
{
    uint8_t index;
    const char *label;
};

static void set_layer_symbol(struct zmk_widget_layer_status *widget,
                             struct layer_status_state state)
{
    char text[13] = {};
    if (state.label == NULL)
    {
        sprintf(text, "%i", state.index);
    }
    else
    {
        snprintf(text, sizeof(text), "%s", state.label);
    }

    lv_canvas_set_buffer(widget->canvas, widget->cbuf, LAYER_CANVAS_WIDTH, LAYER_CANVAS_HEIGHT,
                         CANVAS_COLOR_FORMAT);
    lv_canvas_fill_bg(widget->canvas, LVGL_BACKGROUND, LV_OPA_COVER);
    canvas_draw_text(widget->canvas, 0, 0, LAYER_CANVAS_WIDTH, &widget->layer_label_dsc, text);
    rotate_canvas(widget->canvas, widget->cbuf_rot, LAYER_CANVAS_WIDTH, LAYER_CANVAS_HEIGHT);
}

static void layer_status_update_cb(struct layer_status_state state)
{
    struct zmk_widget_layer_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_symbol(widget, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh)
{
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){.index = index, .label = zmk_keymap_layer_name(index)};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

int zmk_widget_layer_status_init(struct zmk_widget_layer_status *widget, lv_obj_t *parent)
{
    sys_slist_append(&widgets, &widget->node);
    widget->canvas = lv_canvas_create(parent);
    init_label_dsc(&widget->layer_label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT);

    widget_layer_status_init();
    return 0;
}

lv_obj_t *zmk_widget_layer_status_obj(struct zmk_widget_layer_status *widget)
{
    return widget->canvas;
}