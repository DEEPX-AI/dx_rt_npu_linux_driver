#!/bin/bash
# If you need to modify the existing rules,
# you should instead copy the rule file you wish to modify from /lib/udev/rules.d to /etc/udev/rules.d,
# keeping the original name of the file. Any files in /etc/udev/rules.d will override files with identical names in /lib/udev/rules.d
DRIVER_NAME="dxrt"
RULES_FILE="/etc/udev/rules.d/51-deepx-udev.rules"

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