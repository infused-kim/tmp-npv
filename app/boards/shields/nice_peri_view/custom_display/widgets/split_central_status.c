/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

/*
 * ZMK v0.3 only raises zmk_split_peripheral_status_changed on the peripheral
 * half. The central has no public event for the split link, so the widgets
 * would never see the peripheral as connected.
 *
 * This file watches BLE connections on the central and raises the event
 * itself. A connection where this device has the BLE central role is the link
 * to a split peripheral (hosts connect to us, so there we are the BLE
 * peripheral). ZMK's own split central code uses the same check.
 *
 * Before ZMK v0.3, this module relied on ReFil's patch
 * "feat(split): raise status changed events on central" from his split data
 * transfer PR (https://github.com/zmkfirmware/zmk/pull/2036), which raised the
 * event inside ZMK's split central. This file provides the same behavior from
 * within the module.
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/sys/atomic.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/event_manager.h>
#include <zmk/events/split_peripheral_status_changed.h>

#include "split_central_status.h"

static atomic_t connected_peripherals = ATOMIC_INIT(0);

bool nice_peri_view_peripheral_connected(void) { return atomic_get(&connected_peripherals) > 0; }

static bool is_split_peripheral_conn(struct bt_conn *conn) {
    struct bt_conn_info info;

    if (bt_conn_get_info(conn, &info) < 0) {
        return false;
    }

    return info.type == BT_CONN_TYPE_LE && info.role == BT_CONN_ROLE_CENTRAL;
}

static void raise_status(void) {
    raise_zmk_split_peripheral_status_changed(
        (struct zmk_split_peripheral_status_changed){.connected =
                                                         nice_peri_view_peripheral_connected()});
}

static void npv_connected(struct bt_conn *conn, uint8_t err) {
    if (err || !is_split_peripheral_conn(conn)) {
        return;
    }

    atomic_inc(&connected_peripherals);
    LOG_DBG("Split peripheral connected");
    raise_status();
}

static void npv_disconnected(struct bt_conn *conn, uint8_t reason) {
    if (!is_split_peripheral_conn(conn)) {
        return;
    }

    if (atomic_get(&connected_peripherals) > 0) {
        atomic_dec(&connected_peripherals);
    }
    LOG_DBG("Split peripheral disconnected (reason 0x%02x)", reason);
    raise_status();
}

BT_CONN_CB_DEFINE(nice_peri_view_conn_callbacks) = {
    .connected = npv_connected,
    .disconnected = npv_disconnected,
};
