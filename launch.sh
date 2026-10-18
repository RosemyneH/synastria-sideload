#!/usr/bin/env bash
# ʕ •ᴥ•ʔ✿ Start wowdecomp from the Synastria client directory with the sideload bridge. ✿ ʕ •ᴥ•ʔ
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
PLAY="${SYNASTRIA_ROOT:-$HOME/Games/Synastria}"
CLIENT="${WOWDECOMP_BIN:-$HOME/Repos/wowdecomp/bin/Linux/WowClient64}"

make -C "$ROOT" -s

if [[ "${1:-}" == "probe" ]]; then
    exec "$ROOT/build/probe" "$PLAY"
fi

if [[ ! -x "$CLIENT" ]]; then
    echo "wowdecomp client missing: $CLIENT" >&2
    exit 1
fi

# ʕ •ᴥ•ʔ✿ The client takes its Data root from /proc/self/exe. A symlink points back at bin/Linux. ✿ ʕ •ᴥ•ʔ
PLAY_BIN="$PLAY/WowClient64"
if [[ ! -e "$PLAY_BIN" ]] || [[ "$(stat -c %d:%i "$PLAY_BIN")" != "$(stat -c %d:%i "$CLIENT")" ]]; then
    cp -fl "$CLIENT" "$PLAY_BIN" 2>/dev/null || cp -f "$CLIENT" "$PLAY_BIN"
fi

mkdir -p "$PLAY/Logs"
cd "$PLAY"
export SYNASTRIA_ROOT="$PLAY"
export LD_PRELOAD="$ROOT/build/libsynastria.so${LD_PRELOAD:+:$LD_PRELOAD}"
exec ./WowClient64 "$@"
