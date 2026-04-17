#!/bin/bash
#
# SanityCheck.sh - DEEPX Driver Sanity Check Wrapper
#
# This is a convenience wrapper that calls the actual sanity check script
# located in the sanity/ directory.
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SANITY_DIR="$SCRIPT_DIR/sanity"
SANITY_SCRIPT="$SANITY_DIR/sanity.sh"

# Check if sanity script exists
if [[ ! -f "$SANITY_SCRIPT" ]]; then
    echo "Error: Sanity check script not found at: $SANITY_SCRIPT"
    exit 1
fi

# Change to sanity directory and execute the script
# This ensures logs and output files are created in sanity/result/
cd "$SANITY_DIR" || exit 1
exec "./sanity.sh" "$@"
