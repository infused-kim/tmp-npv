/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/kernel.h>
#include "util.h"

// Rotates the canvas content by 90 degrees into cbuf_rot and makes cbuf_rot the
// canvas buffer. width and height are the dimensions before the rotation.
void rotate_canvas(lv_obj_t *canvas, uint8_t cbuf_rot[], uint8_t width, uint8_t height) {
    const lv_draw_buf_t *src_buf = lv_canvas_get_draw_buf(canvas);
    const uint8_t *cbuf = src_buf->data;
    const uint32_t src_stride = src_buf->header.stride;
    const uint32_t dst_stride = lv_draw_buf_width_to_stride(height, CANVAS_COLOR_FORMAT);

#if !CONFIG_NICE_PERI_VIEW_ROTATE_DISPLAY
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            cbuf_rot[i * dst_stride + (height - 1 - j)] = cbuf[i + j * src_stride];
        }
    }
#else
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            cbuf_rot[(width - i - 1) * dst_stride + j] = cbuf[i + j * src_stride];
        }
    }
#endif

    lv_canvas_set_buffer(canvas, cbuf_rot, height, width, CANVAS_COLOR_FORMAT);
}

void init_label_dsc(lv_draw_label_dsc_t *label_dsc, lv_color_t color, const lv_font_t *font,
                    lv_text_align_t align) {
    lv_draw_label_dsc_init(label_dsc);
    label_dsc->color = color;
    label_dsc->font = font;
    label_dsc->align = align;
}

void init_rect_dsc(lv_draw_rect_dsc_t *rect_dsc, lv_color_t bg_color) {
    lv_draw_rect_dsc_init(rect_dsc);
    rect_dsc->bg_color = bg_color;
}
void canvas_draw_text(lv_obj_t *canvas, int32_t x, int32_t y, int32_t max_w,
                      lv_draw_label_dsc_t *draw_dsc, const char *txt) {
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);

    draw_dsc->text = txt;
    const int32_t h = lv_canvas_get_draw_buf(canvas)->header.h;
    lv_area_t coords = {x, y, x + max_w - 1, h - 1};
    lv_draw_label(&layer, draw_dsc, &coords);

    lv_canvas_finish_layer(canvas, &layer);
}
