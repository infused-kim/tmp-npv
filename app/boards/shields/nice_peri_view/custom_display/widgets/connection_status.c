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
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <zmk/events/split_peripheral_status_changed.h>
#include "connection_status.h"
#include "util.h"
#include "split_central_status.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct connection_status_state {
    bool connected;
};

LV_IMAGE_DECLARE(keyboard_pictogram_large);
LV_IMAGE_DECLARE(not_connected_pictogram_large);
LV_IMAGE_DECLARE(not_connected_pictogram_large_left);

static void set_connection_img(struct zmk_widget_connection_status *widget,
                               struct connection_status_state state) {
    if (state.connected) {
        lv_obj_add_flag(widget->not_connected, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(widget->not_connected, LV_OBJ_FLAG_HIDDEN);
    }
}

static void connection_status_update_cb(struct connection_status_state state) {
    struct zmk_widget_connection_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_connection_img(widget, state); }
}

static struct connection_status_state connection_status_get_state(const zmk_event_t *eh) {
    // eh is NULL when the widget initializes
    const struct zmk_split_peripheral_status_changed *ev =
        (eh != NULL) ? as_zmk_split_peripheral_status_changed(eh) : NULL;
    return (struct connection_status_state){
        .connected = (ev != NULL) ? ev->connected : nice_peri_view_peripheral_connected()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_connection_status, struct connection_status_state,
                            connection_status_update_cb, connection_status_get_state)

ZMK_SUBSCRIPTION(widget_connection_status, zmk_split_peripheral_status_changed);

int zmk_widget_connection_status_init(struct zmk_widget_connection_status *widget,
                                      lv_obj_t *parent) {
    sys_slist_append(&widgets, &widget->node);
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 29, 68);
    widget->art = lv_image_create(widget->obj);
    lv_image_set_src(widget->art, &keyboard_pictogram_large);
    lv_obj_align(widget->art, LV_ALIGN_TOP_LEFT, 0, 0);
    widget->not_connected = lv_image_create(widget->obj);
#if CONFIG_NICE_PERI_VIEW_RIGHT_SIDE_CENTRAL
    lv_image_set_src(widget->not_connected, &not_connected_pictogram_large_left);
#if !CONFIG_NICE_PERI_VIEW_ROTATE_DISPLAY
    lv_obj_align(widget->not_connected, LV_ALIGN_TOP_LEFT, 0, 0);
#else
    lv_obj_align(widget->not_connected, LV_ALIGN_BOTTOM_LEFT, 0, 0);
#endif
#else
    lv_image_set_src(widget->not_connected, &not_connected_pictogram_large);
#if !CONFIG_NICE_PERI_VIEW_ROTATE_DISPLAY
    lv_obj_align(widget->not_connected, LV_ALIGN_BOTTOM_LEFT, 0, 0);
#else
    lv_obj_align(widget->not_connected, LV_ALIGN_TOP_LEFT, 0, 0);
#endif
#endif
    lv_obj_add_flag(widget->not_connected, LV_OBJ_FLAG_HIDDEN);

    widget_connection_status_init();
    return 0;
}

lv_obj_t *zmk_widget_connection_status_obj(struct zmk_widget_connection_status *widget) {
    return widget->obj;
}