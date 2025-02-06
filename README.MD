Pikmin 2  
[![Build Status]][actions] [![Code Progress]][progress] [![Data Progress]][progress] [![Linked Progress]][progress]
=============

[Build Status]: https://github.com/projectPiki/pikmin2/actions/workflows/build.yml/badge.svg
[actions]: https://github.com/projectPiki/pikmin2/actions/workflows/build.yml
[Code Progress]: https://decomp.dev/projectPiki/pikmin2.svg?mode=shield&measure=code&label=Code&category=all
[Data Progress]: https://decomp.dev/projectPiki/pikmin2.svg?mode=shield&measure=data&label=Data&category=all
[Linked Progress]: https://decomp.dev/projectPiki/pikmin2.svg?mode=shield&measure=complete_code&label=Linked%20Code&category=all
[Discord Badge]: https://img.shields.io/discord/933849697485983765?color=%237289DA&logo=discord&logoColor=%23FFFFFF
[progress]: https://decomp.dev/projectPiki/pikmin2

A mod of Pikmin 2 that adds the go-here functionality found in Pikmin 3.

Index
-----

- [Dependencies](#dependencies)
  - [Windows](#windows)
  - [macOS](#macos)
  - [Linux](#linux)
- [Installation](#installation)

Dependencies
------------

### Windows

On Windows, it's **highly recommended** to use native tooling. WSL or msys2 are **not** required.

- Install [Python](https://www.python.org/downloads/) and add it to `%PATH%`.
  - Also available from the [Windows Store](https://apps.microsoft.com/store/detail/python-311/9NRWMJP3717K).
- Download [ninja](https://github.com/ninja-build/ninja/releases) and add it to `%PATH%`.
  - Quick install via pip: `pip install ninja`

### macOS

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages):

  ```sh
  brew install ninja
  ```

- Install [wine-crossover](https://github.com/Gcenx/homebrew-wine):

  ```sh
  brew install --cask --no-quarantine gcenx/wine/wine-crossover
  ```

After OS upgrades, if macOS complains about `Wine Crossover.app` being unverified, you can unquarantine it using:

```sh
sudo xattr -rd com.apple.quarantine '/Applications/Wine Crossover.app'
```

### Linux

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).
- For non-x86(_64) platforms: Install wine from your package manager.
  - For x86(_64), [wibo](https://github.com/decompals/wibo), a minimal 32-bit Windows binary wrapper, will be automatically downloaded and used.

Installation
--------

- Clone the repository:

  ```sh
  git clone https://github.com/projectPiki/pikmin2.git
  ```

- Using [Dolphin Emulator](https://dolphin-emu.org/), extract your game's system data to `orig/GPVE01`. (Or `orig/GPVE01_D17` for demo)
  - Right-click the game in Dolphin's game list and select `Properties`.
  - Go to the `Filesystem` tab and right-click `Disc` -> `Extract System Data`.
![Dolphin filesystem extract](assets/dolphin-extract.png)
  - After extraction, the following file should exist: `orig/GPVE01/sys/main.dol`.
  - Copy the contents of `assets/Drought` to your games `files/user/` folder, so the directory: `files/user/Drought` should exist.

- Configure:

```sh
python configure.py --non-matching
```

- Build:

```sh
ninja
```

Once built with `ninja`, the new DOL will exist at `build/GPVE01/main.dol`. Copy this to your extracted games `sys/` folder, overwriting the old one. You are now ready to play, test, and develop the mod.
