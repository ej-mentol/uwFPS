# uwFPS

A [MetaHookSv](https://github.com/hzqst/MetaHookSv) plugin for GoldSrc that automatically adjusts `fps_max` when entering or leaving water.

## Features

- Lowers FPS cap when fully submerged, restores it on exit
- Configurable trigger depth (waist-deep or fully submerged)
- Configurable delay before FPS change kicks in
- Map whitelist / blacklist via `maps.ini`
- Restores original FPS on plugin disable, map change, or game exit
- No FPS changes while dead, spectating, or during intermission

## Installation

1. Copy `uwFPS.dll` to `<game>/metahook/plugins/`
2. Add `uwFPS.dll` to `<game>/metahook/configs/plugins.lst`
3. (Optional) Create `<game>/uwFPS/maps.ini` for map filtering

## Console variables

| Cvar | Default | Description |
|------|---------|-------------|
| `uw_auto` | `1` | Enable / Disable the plugin (`uw_toggle` command) |
| `uw_fps_underwater` | `20` | FPS cap while submerged |
| `uw_fps_normal` | `0` | FPS cap on exit (`0` = restore original value) |
| `uw_delay` | `0.3` | Seconds before FPS change is applied after entering/leaving water |
| `uw_trigger_level` | `3` | Water level required to trigger (`2` = waist-deep, `3` = fully submerged) |
| `uw_mode` | `0` | Map list mode: `0` = blacklist, `1` = whitelist |

All cvars are saved to `config.cfg` automatically (`FCVAR_ARCHIVE`).

## Console commands

| Command | Description |
|---------|-------------|
| `uw_toggle` | Toggle `uw_auto` on/off and restore FPS immediately if needed |

## Map filtering

Create `<game>/uwFPS/maps.ini` — one map name per line (without `.bsp` extension).  
Lines starting with `;` or `#` are treated as comments.

```
; example maps.ini
bm_c0a0
c1a0
```

With `uw_mode 0` (blacklist) the plugin skips listed maps.  
With `uw_mode 1` (whitelist) the plugin only runs on listed maps.

## Building

Requires MetaHookSv SDK. Add the project to `MetaHookSv.sln` and build `Release` or `Release_AVX2`.

## Requirements

- MetaHookSv (V4)
- GoldSrc engine (HL / HL25 / SvEngine)