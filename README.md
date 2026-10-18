# Synastria sideload

Local experiment. No remote. Do not push until the bridge can stay in a Synastria session.

`WoWExt.dll` is the 32-bit Windows hook Synastria ships for `Wow.exe` build **12340**. It calls that binary at fixed addresses (`ClientServices::SetMessageHandler` at `0x006B0B80`, `CDataStore` reads, `FrameScript_Execute`). Those addresses are not the Linux `WowClient64` image, so the DLL is not mapped into the process.

This bridge does the startup work the DLL does, in-process:

| Piece | What the bridge does |
|---|---|
| `WoWExt.dll` | Left on disk. Opcode and cache checks come from that build. |
| `WoWExt.bin` | Must start with magic `0x2838C53A`. |
| `WoWExt2.bin` | Must be version `10`. |
| `WoWExt3.bin` | Run as Lua once the client Lua state exists. |
| `WoWExtItemLoc.bin`, `WoWExtObjLoc.bin` | Header check only. |
| Opcode `0x4B8` | `CMSG_UNUSED5`. Registered when a connection exists. Command `1` is answered with opcode `0x4B8`, byte `1`, `0x36`, and the token from command `2`. |

## Run

```bash
./launch.sh probe
./launch.sh
```

`probe` only reads the caches. A bare `./launch.sh` hard-links the Release client (`WowClient64.release`, `-O3 -DNDEBUG`) into the Synastria directory and starts it with `LD_PRELOAD=build/libsynastria.so`. The client finds `Data` from `/proc/self/exe`, and a symlink would point back at `bin/Linux`. `realmlist` is already `game.synastria.org`.

The bridge raises the character list cap to 20 and runs `WoWExt3.bin` for the login buttons. `WoWExt.bin` is the container of the rest of the Synastria Lua (perks, dual-class bars, class colors). Those scripts are executed after opcode `0x4B8` command 14 delivers the class list. `CMCGetMultiClassEnabled` stays on so character creation can pick two classes. The other `Custom_*` and `ItemLoc*` names are registered so those scripts can call them.

Log: `Logs/synastria-sideload.log` next to the bins.

`SYNASTRIA_ROOT` and `WOWDECOMP_BIN` override the client directory and the wowdecomp binary.
