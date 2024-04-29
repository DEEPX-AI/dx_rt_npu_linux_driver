#!/bin/bash
source util.sh

RESULT_DIR=result
RESULT_FILE=result_tcTC_NUM_devDEV_N.txt # Don't modify "TC_NUM/DEV_N"

if [[ $# -eq 0 ]]; then
    n_devs=1
    TEST_NUM=10
else
    n_devs=$1
    TEST_NUM=$2
fi

tc_run_cmds=()
total_tc_num=0
result=""
function insert_cmd() {
    local cmd_template=("$@")
    for ((idx=0 ; idx < $n_devs ; idx++)); do
        # output="${cmd_template/DEV_N/"$idx"}"
        output=$(echo "${cmd_template[*]}" | sed "s/DEV_N/$idx/g" | sed "s/TC_NUM/$total_tc_num/g")
        tc_run_cmds+=("${output}")
        echo "Insert#$idx : "${tc_run_cmds[$idx]}
    done
    total_tc_num=$(expr $total_tc_num + 1)
}
function clear_cmd() {
    tc_run_cmds=()
}
function run_process() {
    for ((idx=0 ; idx < $n_devs ; idx++)); do
        run_cmd=${tc_run_cmds[$idx]}
        echo "RUN : $run_cmd"
        $(eval ${run_cmd})&
        pids[$idx]=$!
    done
}
function wait_process() {
    echo "Wait PID Arrays : " ${pids[*]}
    for pid in ${pids[*]}; do
        wait $pid
    done
    clear_cmd
}
function check_tc_result() {
    local tc_i=$1
    local n_devs=$2
    for ((dev_i=0 ; dev_i < $n_devs ; dev_i++)); do
        file=$(echo "$RESULT_DIR/$RESULT_FILE" | sed "s/DEV_N/$dev_i/g" | sed "s/TC_NUM/$tc_i/g")
        result_p_c=$(find $file -type f | xargs grep -c "@@PASS@@")
        if [[ "$result_p_c" == "0" ]]; then
            goto tc_results
        fi
    done
}

### Pre-condition ###
user=$(logname)
if [[ ! -d $PWD/$RESULT_DIR ]]; then
    mkdir -p $PWD/$RESULT_DIR
    echo " ** Make Directory : $PWD/$RESULT_DIR"
fi
chown $user:$user -R $PWD/$RESULT_DIR
if [[ ! -f $W_BIN ]] && [[ ! -f $R_BIN ]]; then
    echo "Need to compile : $TOOLS_D"
    cd $TOOLS_D && make
fi

### Test Main ###
echo "********************** DMA TEST START~~!! **********************"

echo "@@ [TC$total_tc_num] DMA Continuous read/write test @@"
cmd_temp=("sudo ./dma_test.sh DEV_N $TEST_NUM > $RESULT_DIR/$RESULT_FILE")
insert_cmd $cmd_temp
run_process
wait_process
echo ""
check_tc_result "$(expr $total_tc_num - 1)" "$n_devs"

echo "@@ [TC$total_tc_num] DMA async test @@"
cmd_temp=("sudo ./dma_async_test.sh DEV_N $TEST_NUM > $RESULT_DIR/$RESULT_FILE")
insert_cmd $cmd_temp
run_process
wait_process
echo ""
check_tc_result "$(expr $total_tc_num - 1)" "$n_devs"

echo "@@ [TC$total_tc_num] DMA read multi channel test @@"
cmd_temp=("sudo ./dma_multi_channel_rd.sh DEV_N $TEST_NUM > $RESULT_DIR/$RESULT_FILE")
insert_cmd $cmd_temp
run_process
wait_process
echo ""
check_tc_result "$(expr $total_tc_num - 1)" "$n_devs"

echo "@@ [TC$total_tc_num] DMA write multi channel test @@"
cmd_temp=("sudo ./dma_multi_channel_wr.sh DEV_N $TEST_NUM > $RESULT_DIR/$RESULT_FILE")
insert_cmd $cmd_temp
run_process
wait_process
echo ""
check_tc_result "$(expr $total_tc_num - 1)" "$n_devs"


# tc_results: #
echo "********************** DMA TEST END~~!! **********************"
### Pasrsing for test result
echo ""
echo "*************************************************************"
echo "********************** DMA TEST RESULT **********************"
echo "*************************************************************"
for ((tc_i=0 ; tc_i < $total_tc_num ; tc_i++)); do
    result="PASS"
    for ((dev_i=0 ; dev_i < $n_devs ; dev_i++)); do
        file=$(echo "$RESULT_DIR/$RESULT_FILE" | sed "s/DEV_N/$dev_i/g" | sed "s/TC_NUM/$tc_i/g")
        result_p_c=$(find $file -type f | xargs grep -c "@@PASS@@")
        if [[ "$result_p_c" == "0" ]]; then
            echo "  ** Fail Position -> TC:#${tc_i}, DEV_N:#${dev_i} **"
            result="FAIL"
        fi
    done
    echo " ** [ TC${tc_i}_RESULT ] --> ${result} ** "
done

