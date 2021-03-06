# Wayfire

# Introduction

[![Join the chat at https://gitter.im/Wayfire-WM/Lobby](https://badges.gitter.im/Wayfire-WM/Lobby.svg)](https://gitter.im/Wayfire-WM/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) or join #wayfire at freenode.net

Wayfire is a wayland compositor based on wlroots. It aims to create a customizable, extendable and lightweight environment without sacrificing its appearance. If you want to gain a better impression at what it can do, see [screenshots](https://github.com/ammen99/wayfire/wiki/Screenshots), [list of plugins](https://github.com/ammen99/wayfire/wiki/Configuration-&-Plugins)

# Build and install

To build wayfire, you'll need `cmake`, `glm`, `gio-2.0`, `freetype2`, `cairo` and `wlroots` and its build dependencies. Optional dependencies are `gdk-2.0`/`gtk2` and `gdk-pixbuf-2.0`(for loading jpeg backgrounds). When ready, simply clone this repo, compile and install:

```
git clone https://github.com/ammen99/wayfire && cd wayfire
meson build --prefix=/usr --buildtype=release 
ninja -C build && sudo ninja -C build install
```

**Before running Wayfire, copy the default configuration file which is located in the root of the repository and place it in:**
```
cp wayfire.ini.default ~/.config/wayfire.ini
```

You can adjust background, panel properties (font family/size, which launchers to use, etc.) and key/button bindings in this file to your liking. To start wayfire, just execute `wayfire` from a TTY. If you encounter any issues, please read [debug report guidelines](https://github.com/ammen99/wayfire/wiki/Debugging-problems) and open a bug in this repo. You can also write in gitter.
# Project status

**IMPORTANT**: Although many of the features one can expect from a WM are implemented, Wayfire should be considered as **WIP** and **pre-alpha**. In my setup it works flawlessly, but this project hasn't been thoroughly tested, so there are a lot of bugs to be expected and to be fixed.

Currently supported:
1. Seamless integration of both native wayland & Xwayland clients
2. Workspaces (or more like viewports if you are familiar with compiz)
3. Configurable bindings
4. Configuration on-the-fly - changes made to the config file are applied immediately without restarting wayfire
5. Various plugins: Desktop cube, Expo(live workspace previews), Grid(arrange floating windows in a grid), Auto snap at edges, etc. See *plugins*
6. Shell panel with launchers, date, internet connection & battery support
7. Basic touchscreen gestures

See the list of issues to know what else is coming to wayfire. Also, don't hesitate to open a new one if you find any bugs or want some new feature.

# Contributing to the project

There are many ways you can help, aside from developing - open bug reports, test features, add documentation, etc.

If you want to write your own plugin, a general outline of how the plugin system works is here: [plugin architecture](https://github.com/ammen99/wayfire/wiki/Plugin-architecture). Sadly, I don't have much time so there's close to none documentation. You can take a look at the simpler plugins(in `plugins/single_plugins`, the simplest are `command`, `screenshot`, `rotator`, around 50-60 loc each). Don't hesitate to write in IRC (or gitter) if you have any questions.
