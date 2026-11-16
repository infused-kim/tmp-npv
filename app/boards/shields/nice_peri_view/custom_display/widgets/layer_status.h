/*
 * Copyright (c) 2020 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>

#include "util.h"

#define LAYER_CANVAS_HEIGHT 20
#define LAYER_CANVAS_WIDTH 68

struct zmk_widget_layer_status
{
    sys_snode_t node;
    lv_obj_t *canvas;
    uint8_t cbuf[CANVAS_BUF_SIZE(LAYER_CANVAS_WIDTH, LAYER_CANVAS_HEIGHT)] __aligned(LV_DRAW_BUF_ALIGN);
    uint8_t cbuf_rot[CANVAS_BUF_SIZE(LAYER_CANVAS_HEIGHT, LAYER_CANVAS_WIDTH)] __aligned(LV_DRAW_BUF_ALIGN);
    lv_draw_label_dsc_t layer_label_dsc;
};

int zmk_widget_layer_status_init(struct zmk_widget_layer_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_layer_status_obj(struct zmk_widget_layer_status *widget);