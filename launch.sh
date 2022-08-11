#!/usr/bin/env bash

export TF2_PATH="/home/user/.local/share/Steam/steamapps/common/Team Fortress 2"
export LD_LIBRARY_PATH="$TF2_PATH/bin:$LD_LIBRARY_PATH"
steam-run "$TF2_PATH/hl2.sh" \
  -game tf \
  -steam -insecure +sv_lan 1 \
  -console -novid \
  -nominidumps -nobreakpad
