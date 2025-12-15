# DOOM-ASCII-WASM

![LOGO](screenshots/logo.png)

**DOOM ASCII engine compiled to WebAssembly for Raycast integration**

This is a custom fork of [wojciech-graj/doom-ascii](https://github.com/wojciech-graj/doom-ascii) with WASM-specific modifications for the [RayDoom](https://github.com/Saketh-Chandra/raydoom-core) Raycast extension.

## Custom Modifications

This fork includes the following enhancements for Raycast integration:
- **WASM Platform Support**: Compiled to WebAssembly using Emscripten
- **Runtime Exports**: Additional exported functions for state access from JavaScript
- **Player Status API**: Exports for health, armor, ammo, and weapon status
- **Dynamic WAD Loading**: Removed `--preload-file` to allow runtime WAD file loading
- **Raycast Input Handling**: Custom key mappings optimized for Raycast environment

## Building for WASM

Requires Emscripten SDK (emsdk) installed and activated.

```sh
source /path/to/emsdk/emsdk_env.sh
PLATFORM=wasm make clean
PLATFORM=wasm make
```

Output files: `_wasm/obj/doom.{js,wasm}`

## Original Project

Based on [doom-ascii](https://github.com/wojciech-graj/doom-ascii) by Wojciech Graj.

**Try the original over telnet:**
```sh
telnet doom.w-graj.net 666
```

---

## For RayDoom Users

If you're using the RayDoom Raycast extension, you don't need to build this yourself. The WASM binaries are distributed via the [raydoom-core NPM package](https://github.com/Saketh-Chandra/raydoom-core).

---

## Building Original Terminal Version

### Linux / MacOS
Requires Make and a C compiler. Creates `_<YOUR OS>/game/doom_ascii`
```sh
make
```

### Other Platforms
```sh
make PLATFORM=<|unix|musl|win32|win64> <|zip|appimage|appimage-zip|appimage-release|release|clean|clean-all>
```

Note: For prebuilt native binaries, see the [original project releases](https://github.com/wojciech-graj/doom-ascii/releases).

## Settings
The following command-line arguments can be supplied:
- `-nocolor`: Disable color.
- `-nograd`: Disable text gradients, exclusively use fully filled-in pixels.
- `-nobold`: Disable bold text.
- `-chars <ascii|block|braille>`: Use ASCII characters, [unicode block elements](https://en.wikipedia.org/wiki/Block_Elements), or [braille patterns](https://en.wikipedia.org/wiki/Braille_Patterns).
- `-erase`: Erase previous frame instead of overwriting. May cause a strobe effect.
- `-fixgamma`: Scale gamma to offset darkening of pixels caused by using a text gradient. Use with caution, as colors become distorted.
- `-kpsmooth <>`: Set the number of ms a key has to be left depressed for it to count as such. Used to counteract jittery inputs when key repeat delay exceeds frametime.
- `-scaling <>`: Set resolution. Smaller numbers denote a larger display. A scale of 4 is used by default, and should work flawlessly on all terminals. Most terminals (excluding Windows CMD) should manage with scales up to and including 2.

## Controls

### RayDoom Raycast Extension
Custom keybindings for Raycast integration:

|Action           |Keybind      |
|-----------------|-------------|
|MOVE FORWARD     |W            |
|MOVE BACKWARD    |S            |
|TURN LEFT        |A            |
|TURN RIGHT       |D            |
|STRAFE LEFT      |Shift+A      |
|STRAFE RIGHT     |Shift+D      |
|FIRE             |F            |
|USE              |E            |
|WEAPON SELECT    |1-7          |
|MAP              |Tab          |

### Original Terminal Version
Default keybindings for native terminal builds:

|Action         |Default Keybind|
|---------------|---------------|
|UP             |ARROW UP       |
|DOWN           |ARROW DOWN     |
|LEFT           |ARROW LEFT     |
|RIGHT          |ARROW RIGHT    |
|STRAFE LEFT    |,              |
|STRAFE RIGHT   |.              |
|FIRE           |F              |
|USE            |E              |
|SPEED          |]              |
|WEAPON SELECT  |1-7            |

Keybinds can be remapped in `.default.cfg`, which should be placed in the same directory as the game executable.

## Performance tips
### Display
Most terminals aren't designed for massive throughput, so the game cannot be played at full 320x200 resolution at high frames per second.

Pass the command-line argument `-scaling` to determine the level of scaling (See [Settings](#settings)).

## Troubleshooting

### For RayDoom Users
If you encounter issues with the Raycast extension, please file issues at the [raydoom-core repository](https://github.com/Saketh-Chandra/raydoom-core/issues).

### Terminal Version

#### Colours are displayed incorrectly
If the displayed image looks something like [this](https://github.com/wojciech-graj/doom-ascii/issues/8), you are likely using a terminal that does not support 24 bit RGB. See [this](https://github.com/termstandard/colors) for more details, troubleshooting information, and a list of supported terminals.

#### Running make throws an error
Run `make --version` and `cc --version` to verify that you have Make and a C compiler installed. If you do, and you're still getting an error, file an issue at the [original project](https://github.com/wojciech-graj/doom-ascii/issues).
