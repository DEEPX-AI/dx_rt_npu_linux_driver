#!/bin/bash
# @Description
#  - Pre Condition
#   1.Register this script into crontab executing on reboot
#     <crontab examples>
#     @reboot /home/skh/PROJECT/dxrt_comm/dxrt_comm/pcie/synopsys/dw-edma_v5.13/test_sc/pcie_reset_trigger.sh 2>&1 | 
#             tee /home/skh/PROJECT/dxrt_comm/dxrt_comm/pcie/synopsys/dw-edma_v5.13/test_sc/log.txt
#   2.Execute pcie_reset.sh to start test

ROOT_DIR=$(dirname $(realpath $0))
TC_ENABLE_F=$ROOT_DIR/result/pcie_warm_reset_tc_enable.txt
tc_file=$ROOT_DIR/pcie_reset.sh

cd ${ROOT_DIR}
echo "Wait 60 seconds.."
sleep 60s
if [[ -e "$TC_ENABLE_F" ]]; then
    echo "Start pcie_reset.sh"
    ${tc_file}
else
	echo "file is not existing ${tc_file}"
fi
