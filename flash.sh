#!/usr/bin/env bash
set -eo pipefail

SELF="$(basename "$0")"

function _script_echo   { echo "[$(date)] $SELF[$$]" "${@:2}" "#$1" >&2; }
function debug          { [[ -z "$DEBUG" ]] || _script_echo DEBUG "$@"; }
function e              { [[ -n "$QUIET" ]] || _script_echo INFO "$@"; }
function info           { e "$@"; }
function warn           { [[ -n "$QUIET" ]] || _script_echo WARNING "$@"; }
function warning        { warn "$@"; }
function error          { _script_echo ERROR "$@" >&2; }
function death          { error "$@"; exit 1; }
function debug_call     { debug 'call:' "$@"; "$@"; }

FW="${1:?}"
DNXP="${2:-DNXP_0x1-prod.bin}"

type -a dldrcli || death "Cannot find \"dldrcli\" in PATH"

e "Provisioning the eMMc"
dldrcli -v --command configpart --fw_dnx "$DNXP" --path "DNX-dldrcli" -v/cfgpart.xml --device 2 --idx 0 "$FW" \
    || warn "Provisioning failed, probably already provisioned."

e "Checking for RPMB"
dldrcli -v --command readbootmedia --fw_dnx "$DNXP" --path "rpmb.bin" --device 2 --idx 0 --start 0 --blocks 4096 --part 16 \
    && warn "Unit has a RPMB area and will not support the Slim version of the CSE!" \
             "This means if you are trying to flash a BIOS version greater then #010, it will fail!" \
             "This issue can NOT be fixed!" \
    || :

e "Downloading the BIOS"
dldrcli -v --command downloadfwos --fw_dnx "$DNXP" --device "emmc" --idx 0 --fw_image "$FW" \
    || death "Downloading failed!"

e "Clearing NvStorage"
dldrcli -v --command clearrpmb --fw_dnx "$DNXP" --device 2 --idx 0 \
    || death "Clearing NvStorage failed!"

e "Done!"

