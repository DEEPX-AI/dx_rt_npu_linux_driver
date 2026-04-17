#!/bin/bash
#
# service_manager.sh - systemd service management functions
#
# This script provides functions to manage the dxrt.service systemd service,
# including starting, stopping, waiting for state changes, and debugging.

# Source common utilities if not already loaded
if [ -z "$(type -t logmsg)" ]; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    source "${SCRIPT_DIR}/common.sh"
fi

# ============================================
# Service Debugging
# ============================================

# Dumps comprehensive debug information for dxrt.service
# Arguments:
#   $1 - phase: Description of when this dump is being taken (e.g., "stop_fail_cmd")
function _dxrt_service_dump_debug() {
    local phase=$1
    command -v systemctl &>/dev/null || return 0
    logerr "[dxrt.service][${phase}] Debug dump begin"
    systemctl show dxrt.service -p ActiveState -p SubState -p ExecMainCode -p ExecMainStatus -p MainPID 2>&1 | sed 's/^/  /'
    systemctl status dxrt.service 2>&1 | head -n 25 | sed 's/^/  /'
    journalctl -u dxrt.service -n ${SERVICE_DEBUG_JOURNAL_LINES} --no-pager 2>&1 | sed 's/^/  /'
    if command -v ps &>/dev/null; then
        local mpid
        mpid=$(systemctl show -p MainPID --value dxrt.service 2>/dev/null || echo 0)
        if [[ "$mpid" != "0" ]]; then
            logmsg "  /proc/${mpid} excerpt:"
            { tr '\0' ' ' < /proc/${mpid}/cmdline 2>/dev/null; echo; } | sed 's/^/    CMD: /'
            cat /proc/${mpid}/status 2>/dev/null | head -n 15 | sed 's/^/    /'
        fi
    fi
    if command -v lsof &>/dev/null; then
        lsof -p $(systemctl show -p MainPID --value dxrt.service 2>/dev/null || echo 0) 2>/dev/null | head -n 15 | sed 's/^/  LSOF: /'
    fi
    logerr "[dxrt.service][${phase}] Debug dump end"
}

# ============================================
# Service State Management
# ============================================

# Waits for the service to reach a desired state within a timeout period
# Arguments:
#   $1 - desired: Target state (active|inactive)
#   $2 - timeout: Maximum time to wait in seconds (can be fractional)
# Returns:
#   0 if desired state reached, 1 if timeout
function _wait_service_state() {
    local desired=$1   # active|inactive
    local timeout=$2   # seconds (can be fractional)
    local interval=0.2

    command -v systemctl &>/dev/null || return 0

    # Convert a (possibly fractional) seconds value to integer milliseconds.
    _to_ms() {
        # Avoid external bc; use bash string ops.
        local val="$1"
        # If already integer
        if [[ $val =~ ^[0-9]+$ ]]; then
            echo "$(( val * 1000 ))"
            return 0
        fi
        # If fractional (e.g. 1.23)
        if [[ $val =~ ^([0-9]+)\.([0-9]+)$ ]]; then
            local whole=${BASH_REMATCH[1]}
            local frac=${BASH_REMATCH[2]}
            # pad / trim frac to 3 digits for ms
            frac=${frac}000
            frac=${frac:0:3}
            echo "$(( whole * 1000 + 10#${frac} ))"
            return 0
        fi
        # Fallback: treat as 0
        echo 0
    }

    local timeout_ms=$(_to_ms "${timeout:-0}")
    local interval_ms=$(_to_ms "$interval")
    (( timeout_ms <= 0 )) && return 1

    # Monotonic time in ms (prefer /proc/uptime; fallback to date)
    _now_ms() {
        if [[ -r /proc/uptime ]]; then
            # /proc/uptime: "<seconds> <idle>" with seconds possibly fractional
            local up
            read -r up _ < /proc/uptime
            # Reuse _to_ms
            _to_ms "$up"
        else
            # date +%s%3N (not all date versions support %3N). Try nanoseconds then cut.
            if date +%s%3N >/dev/null 2>&1; then
                date +%s%3N
            else
                # Fallback seconds only *1000
                echo "$(( $(date +%s) * 1000 ))"
            fi
        fi
    }

    local start_ms=$(_now_ms)
    local now_ms elapsed_ms

    while :; do
        if systemctl is-active --quiet dxrt.service; then
            [[ $desired == "active" ]] && return 0
        else
            local st
            st=$(systemctl show -p ActiveState --value dxrt.service 2>/dev/null || echo unknown)
            if [[ $desired == "inactive" && ( $st == inactive || $st == failed ) ]]; then
                return 0
            fi
        fi

        now_ms=$(_now_ms)
        elapsed_ms=$(( now_ms - start_ms ))
        (( elapsed_ms >= timeout_ms )) && break
        # Sleep using seconds (interval may be fractional)
        sleep "$interval"
    done
    return 1
}

# ============================================
# Service Control
# ============================================

# Checks for and stops the dxrt systemd service if it is running.
# Sets DXRT_SERVICE_WAS_ACTIVE=1 if service was active before stopping.
function stop_dxrt_service() {
    logmsg "-> Checking for dxrt.service..."
    # 'systemctl' might not exist on all systems, check first.
    if ! command -v systemctl &> /dev/null; then
        logmsg "--> 'systemctl' not found. Skipping service check."
        return
    fi

    # Use 'systemctl is-active' which is quiet and efficient for checks.
    if systemctl is-active --quiet dxrt.service; then
        logmsg "--> dxrt.service active. Attempting stop..."
        DXRT_SERVICE_WAS_ACTIVE=1
        local pre_pid
        pre_pid=$(systemctl show -p MainPID --value dxrt.service 2>/dev/null || echo 0)

        if ! sudo systemctl stop dxrt.service; then
            logerr "--> systemctl stop command failed immediately."
            _dxrt_service_dump_debug stop_fail_cmd
            exit 1
        fi
        if ! _wait_service_state inactive ${STOP_TIMEOUT_SEC}; then
            logerr "--> Service did not reach inactive within ${STOP_TIMEOUT_SEC}s"
            _dxrt_service_dump_debug stop_timeout
            # last resort: force kill
            if [[ -n "$pre_pid" && "$pre_pid" != "0" ]]; then
                logerr "--> Forcing kill -9 on old PID ${pre_pid}"
                sudo kill -9 "$pre_pid" 2>/dev/null || true
                sleep 1
                if systemctl is-active --quiet dxrt.service; then
                    logerr "--> Still active after forced kill. Aborting."
                    exit 1
                fi
            fi
        else
            logmsg "--> Service dxrt.service now inactive."
        fi
    else
        logmsg "--> Service dxrt.service is not active."
    fi
}

# Restart dxrt.service only if it was active before stopping.
# Uses DXRT_SERVICE_WAS_ACTIVE global variable to determine if restart is needed.
function restart_dxrt_service_if_needed() {
    if [[ ${DXRT_SERVICE_WAS_ACTIVE} -eq 1 ]]; then
        if ! command -v systemctl &> /dev/null; then
            logmsg "--> 'systemctl' not found. Cannot restart dxrt.service."
            return
        fi
        logmsg "-> Restarting previously active dxrt.service..."
        systemctl reset-failed dxrt.service 2>/dev/null || true
        local start_rc=0
        if ! sudo systemctl start dxrt.service; then
            start_rc=$?
            logerr "--> systemctl start returned failure (rc=${start_rc})"
            _dxrt_service_dump_debug start_fail_cmd
            return
        fi
        if ! _wait_service_state active ${START_TIMEOUT_SEC}; then
            logerr "--> dxrt.service failed to become active within ${START_TIMEOUT_SEC}s"
            _dxrt_service_dump_debug start_timeout
            return
        fi
        # Stabilization: ensure it doesn't exit immediately after appearing active
        sleep 0.5
        if ! systemctl is-active --quiet dxrt.service; then
            logerr "--> dxrt.service became inactive shortly after start"
            _dxrt_service_dump_debug start_unstable
            return
        fi
        logmsg "--> dxrt.service restarted successfully (active and stable)."
    else
        logmsg "-> dxrt.service was not active before install; not starting."
    fi
}
