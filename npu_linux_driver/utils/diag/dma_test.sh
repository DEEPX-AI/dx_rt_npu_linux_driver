#!/bin/bash
# @Description
#  - Test Sequence 
#   1.[Write] Host -> EP
#   2.[Read]  EP -> Host
#   3.Compare Write and Read datas and decide test result.
#   4.Repeat step1~3 with other binary data


TEST_TITLE="DMA Continuous read/write test"
source util.sh

if [[ $# -eq 0 ]]; then
    DEV_N=0
    REPEAT_NUM=10
else
    DEV_N=$1
    REPEAT_NUM=$2
fi

FILES_NUM=2
IN_FILES=(input/dummy_0.bin input/dummy_1.bin)
OUT_FILES=(output/dummy_out_0_dev"$DEV_N".bin output/dummy_out_1_dev"$DEV_N".bin)

EP_REGION1=0x44100000
### Fix Haps DDR ISSUE ###
check_ret "sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a $EP_REGION1 -s 0x2000000)"

### Test Main ###
for ((var=0 ; var < $REPEAT_NUM ; var++));
do
    for ((idx=0 ; idx < $FILES_NUM ; idx++));
    do
        sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a $EP_REGION1 -s 26279936 -f ${IN_FILES[$idx]}
        sudo $R_BIN -d /dev/dx_dma"$DEV_N"_c2h_0 -a $EP_REGION1 -s 26279936 -f ${OUT_FILES[$idx]}
        DIFF_R=$(diff ${IN_FILES[$idx]} ${OUT_FILES[$idx]})
        if [[ "$DIFF_R" == "" ]] && [[ -e ${IN_FILES[$idx]} ]] && [[ ${OUT_FILES[$idx]} ]]; then
            echo "$TEST_TITLE #$var, $idx - PASS"
            rm -rf ${OUT_FILES[*]}
        else
# tc_fail: #
            echo "[[ $TEST_TITLE ]] --> FAIL (test_seq:$var/$REPEAT_NUM, FILE:$idx)"
            exit
        fi
    done
done

echo "[[ $TEST_TITLE ]] --> @@PASS@@ (test_seq:$REPEAT_NUM/$REPEAT_NUM)"