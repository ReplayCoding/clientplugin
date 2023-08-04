#!/usr/bin/env bash

export TF2_PATH="/home/user/.local/share/Steam/steamapps/common/Team Fortress 2"
export LD_LIBRARY_PATH="$TF2_PATH/bin:$TF2_PATH/tf/bin:$LD_LIBRARY_PATH"
pushd "$TF2_PATH"

steam-run lldb "$TF2_PATH/hl2_linux" -- \
  -game tf \
  -steam -insecure +sv_lan 1 \
  -console \
  -nobreakpad \
  -dev +spew_consolelog_to_debugstring 1 -allowdebug \
  -novid -nojoy -nosteamcontroller -nohltv -particles 1 -precachefontchars -noquicktime \
  +mat_autoload_glshaders 0

popd
