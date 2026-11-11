#!/bin/sh
# PortMaster-style launcher template. The compiled ARM binary must be named lastshift.
PORTDIR="$(dirname "$0")"
cd "$PORTDIR"
export SDL_GAMECONTROLLERCONFIG="${SDL_GAMECONTROLLERCONFIG:-}"
./lastshift
