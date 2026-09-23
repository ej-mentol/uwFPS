# uwFPS 1.4.0

uwFPS is a client-side [MetaHookSv](https://github.com/hzqst/MetaHookSv) plugin for GoldSrc. It lowers `fps_max` while the local player is fully submerged and restores the configured or previously active FPS cap when the effect ends.

## Features

- Configurable underwater FPS cap, trigger depth, and delay.
- Optional map filtering with blacklist or whitelist mode.
- Restores the FPS cap when leaving the water, disabling the plugin, changing maps, disconnecting, or exiting the game.
- Clears the underwater state while dead, spectating, in intermission, or viewing through a non-player camera. The saved FPS cap is restored on these inactive paths.
- Includes a fallback that checks the archived `uw_saved_fps` value on the first HUD frame after startup.

## Installation

1. Copy `uwFPS.dll` to `<game>/metahook/plugins/`.
2. Add `uwFPS.dll` to `<game>/metahook/configs/plugins.lst`.
3. Optionally create `<game>/uwFPS/maps.ini` to filter maps.

## Console variables

| Cvar | Default | Description |
| --- | ---: | --- |
| `uw_auto` | `1` | Enables automatic underwater FPS control. |
| `uw_fps_underwater` | `20` | FPS cap while fully submerged. |
| `uw_fps_normal` | `0` | FPS cap after leaving the water; `0` restores the cap saved on entry. |
| `uw_delay` | `0.3` | Delay in seconds before changing the cap after entering or leaving water. |
| `uw_trigger_level` | `3` | Water level required to trigger the effect; `2` is waist-deep and `3` is fully submerged. |
| `uw_mode` | `0` | Map list mode: `0` is blacklist, `1` is whitelist. |
| `uw_saved_fps` | `0` | Internal archived recovery value. Do not edit manually. |

All listed cvars use `FCVAR_ARCHIVE` and are stored in `config.cfg` by the engine.

## Command

| Command | Description |
| --- | --- |
| `uw_toggle` | Toggles `uw_auto`; disabling it restores the saved FPS cap immediately. |

## Map filtering

Create `<game>/uwFPS/maps.ini` with one map name per line, without the `.bsp` extension. Lines beginning with `;` or `#` are ignored.

```text
; Example maps.ini
bm_c0a0
c1a0
```

With `uw_mode 0`, listed maps are excluded. With `uw_mode 1`, only listed maps are enabled.

## Build

Open `MetaHook.sln` or `uwFPS.vcxproj` in Visual Studio and build the `uwFPS` project with `Release|Win32` or `Release_AVX2|Win32`.

## Requirements

- MetaHookSv plugin API V4
- Sven Co-op client supported by MetaHookSv
