# Copyright (c) 2020 The ZMK Contributors
# SPDX-License-Identifier: MIT

if(CONFIG_ZMK_DISPLAY AND CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM AND CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    zephyr_library()
    zephyr_library_sources(${ZEPHYR_BASE}/misc/empty_file.c)
    zephyr_library_include_directories(${ZEPHYR_LVGL_MODULE_DIR})
    zephyr_library_include_directories(${ZEPHYR_BASE}/lib/gui/lvgl/)
    zephyr_library_include_directories(${ZEPHYR_BASE}/drivers)
    zephyr_library_include_directories(${CMAKE_SOURCE_DIR}/include)
    zephyr_library_sources(custom_display/status_screen.c)
    zephyr_library_sources(custom_display/widgets/battery_status.c)
    zephyr_library_sources(custom_display/widgets/connection_status.c)
    zephyr_library_sources(custom_display/widgets/connection_status_art.c)
    zephyr_library_sources(custom_display/widgets/layer_status.c)
    zephyr_library_sources(custom_display/widgets/output_status.c)
    zephyr_library_sources(custom_display/widgets/sleep_status.c)
    zephyr_library_sources(custom_display/widgets/sleep_status_art.c)
    zephyr_library_sources(custom_display/widgets/util.c)
endif()