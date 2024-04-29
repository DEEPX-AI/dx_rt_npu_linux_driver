#!/bin/bash
# @Description
#  - Pre Condition
#   1.Call this script into crontab to start
#  - Test Sequence 
#   1.Check whether start test by reading file(pcie_warm_reset_tc_enable.txt)
#   2.Find synopsys pcie using lspci and generate dma transaction
#   3.Decide result (Pass:reboot, Fail:Test Exit)

TC_TITLE="PCIE Warm Reset Test"
TC_ENABLE_F=result/pcie_warm_reset_tc_enable.txt
TC_RESULT_F=result/result_tc_warm_reset.txt

PWD=$(dirname $(realpath $0))
REPEAT_NUM=100

function exec_cmd_silent() {
    CMD=$1
    # echo "$SHELL_PREFIX CMD : $CMD"
    {
        eval $CMD
    } &> /dev/null
}
function pcie_dma_transaction(){
    pushd $PWD/../
    make
    sudo ./module_remove.sh
    sudo ./module_insert.sh
    cd ../../tools/ && make && sudo ./dma_from_device -d /dev/dx_dma0_c2h_0 -a 0x400000 -s 0x10 -p -v
    popd
}
dx_pcie_cnt=""
function detect_pcie_cdev(){
    dx_pcie_cnt=$(eval lspci | grep -c "Processing accelerators: Synopsys")
    if [ "$dx_pcie_cnt" == "1" ]; then
        pcie_dma_transaction
        echo "Synopsys PCIE is detected!!"
    else
        echo "Synopsys PCIE is not detected!!"
    fi
}
function print_result(){
    local result=$1
    local curr_idx=$2
    echo "*************************************************************"
    echo "*******   ${TC_TITLE}  -->> ${result} (${curr_idx}/${REPEAT_NUM})    ******"
    echo "*************************************************************"
}
function increase_tc_count(){
    cmd=$(echo $1 > $TC_ENABLE_F)
    $cmd
}
function get_tc_count(){
    cmd="eval cat $TC_ENABLE_F | tr -d ' '"
    $(echo $cmd)
} 
function stop_tc(){
    rm -rf ${TC_ENABLE_F}
    exit
}
function start_tc(){
    if [[ -e "$TC_ENABLE_F" ]]; then
        local tc_count=$(get_tc_count)
        if [[ $tc_count -lt $1 ]]; then
            increase_tc_count $(expr $tc_count + 1)
        else
            print_result "PASS" "${REPEAT_NUM}"
            stop_tc
        fi
    else
        echo "[INIT] Create TC enable File : ${TC_ENABLE_F}"
        echo 1 > ${TC_ENABLE_F}
    fi
}
function decide_result(){
    local result=$1
    if [[ "$result" == "1" ]]; then
        echo "PASS ($(get_tc_count)/${REPEAT_NUM})"
        /sbin/reboot
    else
        print_result "FAIL" "$(get_tc_count)"
        stop_tc
    fi
}

start_tc $REPEAT_NUM
detect_pcie_cdev
echo "Test Result : $dx_pcie_chk [1:PASS, 0:FAIL]"
decide_result $dx_pcie_cnt
