#!/bin/bash
source util.sh

## Step1 - Check pcie kernel module
module_num=$(eval lsmod | grep -c dx_dma)
if [[ "$module_num" == "0" ]]; then
  echo "[ERR] Please check pcie driver whether loaded into system or not"
  exit
fi

## Step2 - Check enumration
sudo $PCIE_RESCAN_F > /dev/null
dx_pcie_cnt=$(eval lspci | grep -c "Processing accelerators: Synopsys, Inc. Device 1004")
if [[ "$dx_pcie_cnt" == "0" ]]; then
  echo "Synopsys PCIE is not detected (Enumeration Fail!!)"
  exit
fi

## Step3 - run test program
sudo ./test_main.sh 1 1