#!/bin/sh

# Launcher Script for MuOS GPLove
# This script is intended to be deployed to the device under packaging/muos/gplove.sh
# Tested under muos banana 2410.1 TODO test on newer builds.

. /opt/muos/script/var/func.sh

# Mark the app active
echo app >/tmp/act_go

# Define paths and commands
APPDIR="$(GET_VAR "device" "storage/rom/mount")/MUOS/application/.gplove2"
CONFDIR="$APPDIR/conf/"

# Export environment variables
export SDL_GAMECONTROLLERCONFIG_FILE="/usr/lib/gamecontrollerdb.txt"
export XDG_DATA_HOME="$CONFDIR"

# Launcher
cd "$APPDIR" || exit 1
SET_VAR "system" "foreground_process" "gplove"
export LD_LIBRARY_PATH="$APPDIR/libs:$LD_LIBRARY_PATH"

# Launch the native gplove binary
./gplove

# kill -9 "$(pidof gptokeyb2.armhf)"
# technically the app handles the quit. 
# gptokeyb2 is meant to be a safer way to quit. WIP on that..


