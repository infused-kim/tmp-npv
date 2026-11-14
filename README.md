# Nice Peripheral View

A ZMK module with alternative nice!view widgets for the central half of a split keyboard. They show the peripheral half's battery level and connection status on the central's display.

It's a module version of the widgets by Tobias Arndt. You can find the [original version in his zmk-config](https://github.com/tobiasarndt/zmk-config).

It also integrates the changes required to detect changes in the peripheral status that were originally added by ReFill in his [PR #2036](https://github.com/zmkfirmware/zmk/pull/2036) so that an unforked version of zmk can be used.

<p align="middle">
<img src="https://github.com/tobiasarndt/zmk-config/assets/54204861/53c7cc19-ce8d-4848-8c77-9eeaa39a5d66" height="400"/>
<img src="https://github.com/tobiasarndt/zmk-config/assets/54204861/5a872f72-4adb-41ab-b733-b1440e55ae4e" height="400"/>
<img src="https://github.com/tobiasarndt/zmk-config/assets/54204861/2ab47ef1-7d45-4489-bb01-68d5bcdb517f" height="400"/>
</p>

The widgets include:

- The peripheral's battery level next to the central's
- Pictograms that show whether the peripheral half is connected
- Output, BLE profile and layer status, rotated for the nice!view
- Art on the sleep screen

## Compatibility

The module's versions follow ZMK releases. Use the module revision that matches your ZMK version:

| ZMK version | Module revision |
| ----------- | --------------- |
| v0.3        | `v0.3`          |
| v0.1, v0.2  | `v0.2`          |

ZMK v0.3 moved the split central API, so `v0.3` of this module doesn't build with older ZMK versions, and older module versions don't build with ZMK v0.3.

## Setup

This module replaces the nice!view widgets on the central half. Make sure the regular nice!view works on your keyboard first. See the [nice!view documentation](https://nicekeyboards.com/docs/nice-view/getting-started) and ZMK's [nice_view shield](https://github.com/zmkfirmware/zmk/tree/v0.3/app/boards/shields/nice_view).

### 1. Add the module to `config/west.yml`

Add the remote and the project. Setting the revision under `defaults` keeps ZMK and the module on the same version:

```diff
manifest:
+ defaults:
+   revision: v0.3 # ZMK release; modules use the same revision
+
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
+   - name: infused-kim
+     url-base: https://github.com/infused-kim

  projects:
    - name: zmk
      remote: zmkfirmware
-     revision: main
      import: app/west.yml
+
+   - name: tmp-npv
+     remote: infused-kim

  self:
    path: config
```

If you pin ZMK explicitly instead, set `revision: v0.3` on both `zmk` and `tmp-npv`.

### 2. Use the `nice_peri_view` shield on the central half

In `build.yaml`, replace `nice_view` with `nice_peri_view` on the **central** half only. The peripheral half keeps the regular `nice_view`:

```diff
include:
  - board: nice_nano_v2
-   shield: corne_left nice_view
+   shield: corne_left nice_peri_view

  - board: nice_nano_v2
    shield: corne_right nice_view
```

The central is usually the left half. If your central is the right half, use `nice_peri_view` on the right half instead and set `CONFIG_NICE_PERI_VIEW_RIGHT_SIDE_CENTRAL=y` (see below).

### 3. Build

That's all. The shield enables everything it needs, so no extra config is required:

- the display (`CONFIG_ZMK_DISPLAY`) and the custom status screen
- fetching the peripheral's battery level (`CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING`)
- the LVGL fonts and widgets the screen uses

It also turns off display blanking on idle (`CONFIG_ZMK_DISPLAY_BLANK_ON_IDLE=n`).

## Configuration options

Set these in your keyboard's `.conf` file, e.g. `config/corne.conf`. The values shown are the defaults:

```ini
# Your central half is on the right
CONFIG_NICE_PERI_VIEW_RIGHT_SIDE_CENTRAL=n

# Rotate the display by 180 degrees
CONFIG_NICE_PERI_VIEW_ROTATE_DISPLAY=n

# Invert the display colors
CONFIG_NICE_PERI_VIEW_SCREEN_INVERTED=n

# When to show the sleep art
CONFIG_NICE_PERI_VIEW_SHOW_SLEEP_ART_ON_IDLE=y
CONFIG_NICE_PERI_VIEW_SHOW_SLEEP_ART_ON_SLEEP=y
```

## Troubleshooting

### `__device_dts_ord_DT_CHOSEN_zephyr_display_ORD undeclared`

```
zephyr/include/zephyr/device.h:85:41: error: '__device_dts_ord_DT_CHOSEN_zephyr_display_ORD' undeclared here (not in a function)
```

`CONFIG_ZMK_DISPLAY=y` is enabled for a half that has no display. Either add `nice_view` to the peripheral half in `build.yaml`, or remove `CONFIG_ZMK_DISPLAY` from your config: the `nice_peri_view` shield enables it for the central anyway. To use different settings per half, put them in `config/<keyboard>_left.conf` and `config/<keyboard>_right.conf`.

## Local builds

When building locally with west and the module isn't in your `config/west.yml`, add it to `ZMK_EXTRA_MODULES`. Separate multiple modules with `;`:

```bash
west build -p -s zmk/app -b nice_nano_v2 -- \
  -DZMK_CONFIG="/path/to/zmk-config/config" \
  -DZMK_EXTRA_MODULES="/path/to/nice-peri-view" \
  -DSHIELD="corne_left nice_peri_view"
```

If your zmk-config contains its own boards or shields (it has a `zephyr/module.yml`), add it to `ZMK_EXTRA_MODULES` as well, e.g. `"/path/to/zmk-config;/path/to/nice-peri-view"`.

See ZMK's documentation on [setting up a local toolchain](https://zmk.dev/docs/development/local-toolchain/setup) and [building with external modules](https://zmk.dev/docs/development/local-toolchain/build-flash#building-with-external-modules).
