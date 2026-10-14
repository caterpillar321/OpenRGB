# OpenRGB — SekaiOS fork

This is a fork of [OpenRGB](https://codeberg.org/OpenRGB/OpenRGB) (Adam Honse and contributors,
GPL-2.0-or-later), branch `sekai`, based on the `release_1.0` tag. It is what SekaiOS ships as the
`openrgb` package (`1.0.0+sekaiN`). Everything else is unchanged upstream OpenRGB; see `README.md`.

Changes on top of 1.0:

- **ENE: serialize register access, queue CLI per-LED colors on the device thread.**
  ENE registers take two SMBus transfers (set pointer, then read/write). The CLI wrote per-LED
  colors from the main thread while the device thread was applying the mode, so the transfers
  interleaved and color bytes landed in 0x8020-0x8025. On ASUS ENE DRAM and the ROG Astral
  RTX 5090 this switched direct mode on or set 0x8024, turning the LEDs off.
- **debian: let shlibs pick the Qt and mbedtls packages** (`libmbedtls14` is not in Debian trixie).

Please report problems with these changes to SekaiOS, not to upstream OpenRGB.
