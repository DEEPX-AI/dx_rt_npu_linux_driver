#!/bin/bash
################## Variable ##################
PWD=$(dirname $(realpath $0))
TOOLS_D=$PWD/../tools
W_BIN=$TOOLS_D/dma_to_device
R_BIN=$TOOLS_D/dma_from_device

################## Common Function ##################
goto() {
  label=$1
  cmd=$(sed -En "/^[[:space:]]*#[[:space:]]*$label:[[:space:]]*#/{:a;n;p;ba};" "$0")
  eval "$cmd"
  exit
}
function check_ret {
    $($1)
    local ret_val=$?
    # echo "ret_val : $ret_val"
    if [[ "$ret_val" != "0" ]]; then
        goto "tc_fail"
    fi
}
