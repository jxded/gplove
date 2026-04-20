# gplove

An SDL2-based rewrite of my popular GPLöve input tester application for anbernic's H700 line of gaming handheld devices, originally depended on a compiled löve2D engine binary.

## Features

- SDL2 game controller support
- Left/right stick position and fading trail visualization
- Button, trigger, and D-pad state rendering
- Uses `SDL2_gfx` for circle rendering
- Uses `SDL2_ttf` for text rendering
- Loads community controller mappings from `SDL\gamecontrollerdb.txt`
- Native desktop build and cross-compile support for an aarch64 device target
- Hopefully convenient dev experience with automated workflows

## Building, Prototyping, Contributing:

### Requirements

- `cmake` 3.20+
- C++17 compiler
- `pkg-config`
- SDL2 development libraries:
  - `SDL2`
  - `SDL2_ttf`
  - `SDL2_image`
  - `SDL2_gfx`


### macOS (Apple Silicon)
Recommended install via Homebrew:

```bash
# build/core tools
brew install cmake ninja git pkg-config

# SDL2, native arm64 (for desktop prototyping on Mac)
brew install sdl2 sdl2_ttf sdl2_image sdl2_gfx

# Cross-compile toolchain: aarch64 Linux GNU on macOS
# You may need to have xcode(-select) updated to latest.
brew tap messense/macos-cross-toolchain
brew install aarch64-unknown-linux-gnu

# Network tools (optional)
brew install nmap
```

### Linux
(Arch) prerequisite setup:
```bash
# Build tools
sudo pacman -S cmake ninja git base-devel pkg-config

# SDL2, native x86_64 (for desktop prototyping)
sudo pacman -S sdl2 sdl2_image sdl2_ttf sdl2_gfx

# Cross-compile toolchain (aarch64)
sudo pacman -S aarch64-linux-gnu-gcc aarch64-linux-gnu-binutils aarch64-linux-gnu-glibc

# Network tools (optional, for on-device deployment)
sudo pacman -S nmap openssh
```

in case the gamecontrollerDB needs to be updated,
```bash
# mkdir -p assets
curl -o assets/gamecontrollerdb.txt \
  https://raw.githubusercontent.com/mdqinc/SDL_GameControllerDB/master/gamecontrollerdb.txt
```

### Build & deploy

### Native desktop:
```bash
make run-native 
make run-native-release # (For better runtime performance profiling)

```
For macs, you may need to do the following upon the first build. Cmake build configuration steps are included in the makefile, including the proper library includes, but:
```bash
export PKG_CONFIG_PATH="$(brew --prefix)/lib/pkgconfig:$PKG_CONFIG_PATH"
export LDFLAGS="-L$(brew --prefix)/lib"
export CPPFLAGS="-I$(brew --prefix)/include"
```

### Cross compiling to AArch64 linux: (WIP)
```bash
make configure-cross
make build-cross
```

### Deploy to device
This assumes your device has SSH/SFTP support. 
Set the following vars in `Makefile`:
```makefile
# For deployment, set these env vars to ssh target
DEVICE        ?= rg35xxh
DEVICE_USER   ?= root
REMOTE_PATH    = /mnt/mmc/MUOS/application/gplove
```
if needed, adjust the launch script `(TODO)` and then: 
```bash
make deploy
make run-device

#if needed:
make kill
```

