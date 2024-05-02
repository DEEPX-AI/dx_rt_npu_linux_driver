#!/bin/bash
DRIVER_NAME="dxrt"
RULES_FILE="/usr/lib/udev/rules.d/51-deepx-udev.rules"

echo "$DRIVER_NAME permissions will be changed(0666)"

echo "#Change mode rules for DEEPX's PCIe driver" > "$RULES_FILE"
echo "SUBSYSTEM==\"$DRIVER_NAME\", MODE=\"0666\"" >> "$RULES_FILE"

if command -v udevadm &> /dev/null; then
    udevadm control --reload-rules
    udevadm trigger
else
    echo "udevadm command not found. Rules are not reloaded."
    echo "Please reboot host to apply the rules"
fi