#!/bin/bash
# @Description
#  - Test Sequence 
#   1.[Read] EP -> Host using dma channle 0 and dma channel 1 simultaneously

TEST_TITLE="DMA Read Test for multi channel"
source util.sh

if [[ $# -eq 0 ]]; then
    DEV_N=0
    REPEAT_NUM=10
else
    DEV_N=$1
    REPEAT_NUM=$2
fi

IN_FILES=input/dummy_0.bin
IN_FILES_SIZE=$(ls -l $IN_FILES | awk '{print  $5}')
OUT_FILES=(output/dummy_out_ch0_dev"$DEV_N".bin output/dummy_out_ch1_dev"$DEV_N".bin)
# echo $IN_FILES_SIZE

REGIONS=(0x44000000 0x46000000)

W_BIN_CMD_CH0="sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a ${REGIONS[0]} -s $IN_FILES_SIZE -f ${IN_FILES}"
W_BIN_CMD_CH1="sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_1 -a ${REGIONS[1]} -s $IN_FILES_SIZE -f ${IN_FILES}"

R_BIN_CMD_CH0=" sudo $R_BIN -d /dev/dx_dma"$DEV_N"_c2h_0 -a ${REGIONS[0]} -s $IN_FILES_SIZE -f ${OUT_FILES[0]} -v 1"
R_BIN_CMD_CH1=" sudo $R_BIN -d /dev/dx_dma"$DEV_N"_c2h_1 -a ${REGIONS[1]} -s $IN_FILES_SIZE -f ${OUT_FILES[1]} -v 1"

test_out_f=(test_ch0.out test_ch1.out)
test_err_f=(test_ch0.err test_ch1.err)

### Pre-condition ##
### Fix Haps DDR ISSUE ###
check_ret "sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a ${REGIONS[0]} -s 0x2000000"
check_ret "sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a ${REGIONS[1]} -s 0x2000000"

# Write Pre-datas to memory sections
echo "Write Pre-datas to memory sections!!"
$(eval $W_BIN_CMD_CH0)
$(eval $W_BIN_CMD_CH1)

### Test Main ###
for ((var=0 ; var < $REPEAT_NUM ; var++));
do
    ## Remove output files
    rm -rf ${OUT_FILES[0]} ${OUT_FILES[1]}

    # Read Datas to memory regions of EP
    nohup ${R_BIN_CMD_CH0} >${test_out_f[0]} 2>${test_err_f[0]} &
    nohup ${R_BIN_CMD_CH1} >${test_out_f[1]} 2>${test_err_f[1]} &
    sleep 1s ## Timeout 5s
    rm -rf ${test_out_f[0]} ${test_out_f[1]}
    rm -rf ${test_err_f[0]} ${test_err_f[1]}

    # Compare output datas
    DIFF_R=$(diff ${OUT_FILES[0]} ${OUT_FILES[1]})
    if [[ "$DIFF_R" == "" ]] && [[ -e ${OUT_FILES[0]} ]] && [[ -e ${OUT_FILES[1]} ]]; then
        echo "$TEST_TITLE #$var - PASS"
        rm -rf ${OUT_FILES[0]} ${OUT_FILES[1]}
    else
# tc_fail: #
        echo "[[ $TEST_TITLE ]] --> FAIL (test_seq:$var/$REPEAT_NUM)"
        exit
    fi
done

echo "[[ $TEST_TITLE ]] --> @@PASS@@ (test_seq:$REPEAT_NUM/$REPEAT_NUM)"