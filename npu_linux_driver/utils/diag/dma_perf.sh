#!/bin/bash
# @Description
#  - Pre Condition
#   1. This test will be needed with macro( DMA_PERF_MEASURE )

echo "!!! Start DMA Transfer performace Test !!!!"

PWD=$(dirname $(realpath $0))
PWD=$(dirname $(realpath $0))
W_BIN="$PWD/../tools/dma_to_device -d /dev/dx_dma0_h2c_0"
R_BIN="$PWD/../tools/dma_from_device -d /dev/dx_dma0_c2h_0"
PERF_USER_LOG=dma_perf_user.log
PERF_USER_KERNEL=dma_perf_kernel.log
ADDR=0x4100000

##### SETTING VALUE #####
REPEAT_NUM=10

function exec_cmd() {
    CMD=$1
    echo "CMD : $CMD"
    eval $CMD
}
function exec_cmd_silent() {
    CMD=$1
    # echo "$SHELL_PREFIX CMD : $CMD"
    {
        eval $CMD
    } &> /dev/null
}
function hex_to_dec(){
    NUMBER=$1
    if [[ "$NUMBER" == 0x* ]]; then
        echo $(($NUMBER))
    else
        echo $NUMBER
    fi
}

DMA_AVG_TIME_USER=""
DMA_AVG_TIME_KERNEL=""
function parsing_log() {
    ## User - Parsing from file
    if [[ -e ${PERF_USER_LOG} ]]; then
        DMA_AVG_TIME_USER=$(cat ${PERF_USER_LOG} | grep avg_time | cut -d '=' -f2 | cut -d ',' -f1 | tr -d ' ')
        rm -rf ${PERF_USER_LOG}
    else
        echo "Please check ${PERF_USER_LOG} file"
        exit
    fi
    ## Kernel - Parsing from dmesg
    exec_cmd_silent "dmesg | grep [PERF] > ${PERF_USER_KERNEL}"
    if [[ -e ${PERF_USER_KERNEL} ]]; then
        TEMP=$(cat ${PERF_USER_KERNEL} | grep PERF | cut -d '@' -f2 | cut -d ' ' -f2)
        # echo ${TEMP[*]}
        sum=0
        for exec_t in ${TEMP[*]}
        do
            sum=$(expr $sum + $exec_t)
        done
        DMA_AVG_TIME_KERNEL=$(expr $sum / $REPEAT_NUM)
        # echo "DMA_AVG_TIME_KERNEL : $DMA_AVG_TIME_KERNEL"
        rm -rf ${PERF_USER_KERNEL}
    else
        echo "Please check ${PERF_USER_KERNEL} file"
        exit
    fi
}
function print_result() {
    local size=$(hex_to_dec $1)
    local size_desc=$(expr $size / 1024)
    printf "S:%10d KB \t User:%20.3f ns \t Kernel:%20d ns \n" "$size_desc" $DMA_AVG_TIME_USER "$DMA_AVG_TIME_KERNEL"
}

#1K, 4K, 16K, 100K, 1M, 2M
CYCLIC_MODE_SIZE=(0x400 0x1000 0x4000 0x19000 0x100000 0x200000)
#1K, 4K, 16K, 100K, 1M, 2M, 10M, 40M, 100M
SG_MODE_SIZE=(0x400 0x1000 0x4000 0x19000 0x100000 0x200000 0xA00000 0x2800000 0x6400000)
# SG_MODE_SIZE=(0x400)

echo "Start DMA Test >> Test repeat number : $REPEAT_NUM"
############################################## Pre-settings ##############################################
DMA_SIZE=${SG_MODE_SIZE[*]}
echo "DMA Operaion Mode : Scatter Gather"

############################################## Start Test ##############################################
echo "** DMA_TO_DEVICE performance test (repeat num per size :$REPEAT_NUM)"
for size in ${DMA_SIZE[*]}
do
    exec_cmd_silent "dmesg -C"
    exec_cmd_silent "sudo $W_BIN -a ${ADDR} -s ${size} -t -c $REPEAT_NUM -v > $PERF_USER_LOG"
    parsing_log
    print_result ${size}
done

echo "** DMA_FROM_DEVICE performance test (repeat num per size :$REPEAT_NUM)"
for size in ${DMA_SIZE[*]}
do
    exec_cmd_silent "dmesg -C"
    exec_cmd_silent "sudo $R_BIN -a ${ADDR} -s ${size} -c $REPEAT_NUM -v > $PERF_USER_LOG"
    parsing_log
    print_result ${size}
done

## Move
exec_cmd_silent "cd $PWD"
