/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

#include "util.h"

#define BATTERY_CANVAS_HEIGHT 15
#define BATTERY_CANVAS_WIDTH 68

struct zmk_widget_batteries_status {
    sys_snode_t node;
    lv_obj_t *canvas;
    uint8_t cbuf[CANVAS_BUF_SIZE(BATTERY_CANVAS_WIDTH, BATTERY_CANVAS_HEIGHT)] __aligned(LV_DRAW_BUF_ALIGN);
    uint8_t cbuf_rot[CANVAS_BUF_SIZE(BATTERY_CANVAS_HEIGHT, BATTERY_CANVAS_WIDTH)] __aligned(LV_DRAW_BUF_ALIGN);
    lv_draw_label_dsc_t central_label_dsc;
    lv_draw_label_dsc_t peripheral_label_dsc;
    char central_text[9];
    char peripheral_text[9];
    bool connected;
};

int zmk_widget_batteries_status_init(struct zmk_widget_batteries_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_batteries_status_obj(struct zmk_widget_batteries_status *widget);