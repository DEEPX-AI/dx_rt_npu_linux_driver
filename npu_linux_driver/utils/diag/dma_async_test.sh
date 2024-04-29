#!/bin/bash
# @Description
#  - PreCodition
#   1.[Write] Host -> EP (0x4400000)
#  - Test Sequence 
#   1.[Write] Host -> EP (0x400000) and [Read] EP -> Host (0x4400000)
#     --> Back-ground process to operate simulatneously
#   2.sleep 2s
#   3.Compare Write and Read datas and decide test result.

TEST_TITLE="DMA Async read/write test"
source util.sh

if [[ $# -eq 0 ]]; then
    DEV_N=0
    REPEAT_NUM=10
else
    DEV_N=$1
    REPEAT_NUM=$2
fi

IN_FILE=input/dummy_0.bin
OUT_WR_FILE=output/dummy_out_0_dma_wr_dev"$DEV_N".bin
OUT_RD_FILE=output/dummy_out_0_dma_rd_dev"$DEV_N".bin

EP_REGION1=0x40400000
EP_REGION2=0x44400000
FILE_SIZE=0x1910000

echo "Start pre-setting!!"
## Fix Haps DDR ISSUE ###
check_ret "sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a $EP_REGION1 -s 0x2000000"
check_ret "sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a $EP_REGION2 -s 0x2000000"

## Pre-codition ###
sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a $EP_REGION2 -s $FILE_SIZE -f $IN_FILE

### Test Main ###
echo "Start Main Test(Async DMA)!!"
for ((var=0 ; var < $REPEAT_NUM ; var++)); do
    ## Remove output files
    rm -rf $OUT_WR_FILE $OUT_RD_FILE

    ## Clear as zero
    sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a $EP_REGION1 -s $FILE_SIZE

    ## Test Async
    sudo $W_BIN -d /dev/dx_dma"$DEV_N"_h2c_0 -a $EP_REGION1 -s $FILE_SIZE -f $IN_FILE& #back-ground
    sudo $R_BIN -d /dev/dx_dma"$DEV_N"_c2h_0 -a $EP_REGION2 -s $FILE_SIZE -f $OUT_RD_FILE&
    sleep 1s ## Timeout 5s

    # Read ep region1 datas
    sudo $R_BIN -d /dev/dx_dma"$DEV_N"_c2h_0 -a $EP_REGION2 -s $FILE_SIZE -f $OUT_WR_FILE

    # Compare ep region1 and region2
    DIFF_R=$(diff ${OUT_RD_FILE} ${OUT_WR_FILE})
    if [[ "$DIFF_R" == "" ]] && [[ -e $OUT_RD_FILE ]] && [[ -e $OUT_WR_FILE ]]; then
        echo "$TEST_TITLE #$var - PASS"
        rm -rf $OUT_WR_FILE $OUT_RD_FILE
    else
# tc_fail: #
        echo "[[ $TEST_TITLE ]] --> FAIL (test_seq:$var/$REPEAT_NUM)"
        exit
    fi
done
echo "[[ $TEST_TITLE ]] --> @@PASS@@ (test_seq:$REPEAT_NUM/$REPEAT_NUM)"