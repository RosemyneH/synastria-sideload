# Synastria sideload

A World of Warcraft **3.3.5a (12340)** client with the Synastria bridge compiled in. The bridge loads the `WoWExt` cache files, speaks opcode `0x4B8`, raises the character list cap to 20, and registers the `Custom_*` and `ItemLoc*` Lua natives those scripts call.

The client source lives in [`client/`](client). Releases are built from that tree on Windows and Linux.

## Play

Download the [latest build](https://github.com/RosemyneH/synastria-sideload/releases/tag/continuous).

| Archive | Platform |
|---|---|
| `synastria-linux-x86_64.tar.gz` | Linux x86_64, `WowClient64` |
| `synastria-windows-x64.zip` | Windows x64, `WowClient64.exe` |

Put the executable in a directory that contains:

- a retail **WoW 3.3.5a (12340)** `Data` folder
- the Synastria files `WoWExt.bin`, `WoWExt2.bin`, `WoWExt3.bin`, `WoWExtItemLoc.bin`, and `WoWExtObjLoc.bin`

`WoWExt.bin` must start with magic `0x2838C53A`. `WoWExt2.bin` must be version `10`. `WoWExt3.bin` is the login Lua. Point `realmlist.wtf` at `game.synastria.org`, then start the executable from that directory.

```bash
chmod +x WowClient64
./WowClient64
```

The log is `Logs/synastria-sideload.log` next to the executable. `SYNASTRIA_ROOT` overrides that directory.

## Build

Linux dependencies match the client: `ninja`, `g++`, SDL3, Wayland, curl, ALSA, Vulkan, and xvidcore.

```bash
cmake -S client -B build/linux -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/linux --target WowClient
```

The binary is `client/bin/Linux/WowClient64`. On Windows the same configure, from a 64-bit Visual Studio shell, writes `client/bin/Win/WowClient64.exe`.

`./launch.sh` still builds `build/libsynastria.so` and preloads it into a separate wowdecomp `WowClient64.release`. The release executables do not need that preload.
