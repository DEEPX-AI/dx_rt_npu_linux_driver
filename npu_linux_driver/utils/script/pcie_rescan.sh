#!/bin/bash
KERN_CONF_F=/lib/modules/$(uname -r)/build/.config
KERN_HOTPLUG_CONFIG="CONFIG_HOTPLUG_PCI=y"
hotplug_en=""

function exec_cmd(){
    CMD=$1
    echo "CMD : $CMD"
    eval $CMD
}

pci_id=""
function get_pci_id(){
    temp=`eval lspci | grep "$1" | tr ' ' '\n' | grep "[0-9]:*\."`
    if [ "$pci_id" != "" ]; then
        pci_id+=" "
    fi
    pci_id+=$(echo $temp)
    #pci_id="0000:${temp:0:7}"
    echo Detected PCI Device ID : ${pci_id}
}

function pci_remove(){
    path="/sys/bus/pci/devices/0000:$1/remove"
    echo $path

    if [ -e $path ]; then
        echo 1 > $path
        echo "remove PCI Driver"
    else
        echo "remove fs is not existed(path:$path)"
    fi
}

function pci_rescan(){
    path="/sys/bus/pci/rescan"
    
    if [ -e $path ]; then
        echo 1 > $path
        echo "rescan PCI Driver"
    else
        echo "rescan fs is not existed(path:$path)"
    fi
}

serial_num=""
function get_serial_num(){
    serial_num=$(lshw | grep Xil -A7 | grep serial | cut -d ':' -f2 | xargs echo -n)
}
device_nums=""
function get_device_num(){
    device_nums=$(eval lspci | grep -c Xilinx)
}

function pcie_unbind_bind(){
    exec_cmd "cd /sys/bus/platform/drivers/tegra194-pcie"
    exec_cmd "echo 141a0000.pcie > unbind"
    exec_cmd "echo 141a0000.pcie > bind"
}

lock_file=/usr/local/etc/pci_lock
lock_owner=/usr/local/etc/pci_owner
function create_lock(){
    if [ -f $lock_file ]; then
        echo "lock file is existed"
    else
        exec_cmd "touch     $lock_file"
        exec_cmd "chmod 664 $lock_file"
        exec_cmd "touch     $lock_owner"
        exec_cmd "chmod 664 $lock_owner"
    fi
}
function pci_lock(){
    owner=$(eval cat $lock_owner | tr ' ' '\n')
    while :
    do
        lock=$(eval cat $lock_file | tr ' ' '\n')
        if [ "$lock" == "0" ]; then
            exec_cmd "echo 1 > $lock_file"
            $(logname > $lock_owner)
            break
        elif [ "$lock" == "1" ]; then
            sleep 1
            echo "wait until the lock is relased($lock_file). (owner:$owner)"
        else
            exec_cmd "echo 0 > $lock_file"
            echo "lock file is initialized"
        fi
    done
}
function pci_unlock(){
    exec_cmd "echo 0 > $lock_file"
    exec_cmd "echo none > $lock_owner"
}
#### Start ####
# create_lock
# pci_lock

# Check hotplug function of pcie
if [ -e ${KERN_CONF_F} ]; then
    hotplug_en=$(eval cat ${KERN_CONF_F} | grep -c ${KERN_HOTPLUG_CONFIG})
    if [ "$hotplug_en" == "0" ]; then
        echo "KERNEL PCIE HOTPLUG FUNCTION is not supported"
    else
        echo "KERNEL PCIE HOTPLUG FUNCTION is supported"
    fi
else
    echo "Please check kernel config file : ${KERN_CONF_F}"
    exit
fi

### PCIE SCAN MAIN LOGIC ###
if [ "$hotplug_en" == "0" ]; then
    ## JETSON AGX ORIN ##
    echo "Start PCIE Unbind / Bind !!"
    pcie_unbind_bind
else
    ### Start PCI Device Remove & Rescan ###
    echo "[Get] Xilinx PCIE ID"
    get_pci_id "Xilinx"

    if [ "$pci_id" == "" ]; then
        echo "[Get] DeepX PCIE ID"
        get_pci_id "Processing accelerators: DEEPX Co., Ltd."
        get_pci_id "Processing accelerators: Synopsys"
    fi

    for id in ${pci_id}
    do
        echo "[Remove] PCI ID : ${id}"
        pci_remove ${id}
    done
    pci_rescan
    ### End PCI Device Remove & Rescan ###
fi

# pci_unlock
#### End ####