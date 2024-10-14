
#!/bin/bash

. /opt/muos/script/var/func.sh


echo app >/tmp/act_go

# Define paths and commands
LOVEDIR="$(GET_VAR "device" "storage/rom/mount")/MUOS/application/.gplove"
GPTOKEYB="$(GET_VAR "device" "storage/rom/mount")/MUOS/emulator/gptokeyb/gptokeyb2.armhf"
CONFDIR="$LOVEDIR/conf/"

# Export environment variables
export SDL_GAMECONTROLLERCONFIG_FILE="/usr/lib/gamecontrollerdb.txt"
export XDG_DATA_HOME="$CONFDIR"

# Launcher
cd "$LOVEDIR" || exit
SET_VAR "system" "foreground_process" "love"
export LD_LIBRARY_PATH="$LOVEDIR/libs:$LD_LIBRARY_PATH"
$GPTOKEYB "love" &
./love gplove
kill -9 "$(pidof gptokeyb2.armhf)"

