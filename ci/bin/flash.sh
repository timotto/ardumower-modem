#!/bin/sh

set -euo pipefail

ESPTOOL_DEFAULT_ARGS="--chip esp32 --baud 921600 --before default_reset --after hard_reset"
ESPTOOL_DEFAULT_FLASH_ARGS="write_flash --compress --flash_mode dio --flash_freq 80m --flash_size detect"
ESPTOOL_ARGS="${ESPTOOL_ARGS:-$ESPTOOL_DEFAULT_ARGS}"
ESPTOOL_FLASH_ARGS="${ESPTOOL_FLASH_ARGS:-$ESPTOOL_DEFAULT_FLASH_ARGS}"

function main() {
  local cmd partitions

  cmd=`lookup_esptool`

  parse_cli_parameters "${@}"
  shift

  run \
    "$cmd" \
    --port "${serial_port}" \
    ${ESPTOOL_ARGS} \
    ${ESPTOOL_FLASH_ARGS} \
    "${@}"
}

function run() {
  local cmd

  cmd="$1"
  shift

  echo "$cmd" "${@}"
  test "$dryrun" = "0" || return 0

  "$cmd" "${@}"
}

function lookup_esptool() {
  which esptool.py && return 0 || true
  lookup_arduino_esptool && return 0 || true

  die "unable to find esptool"
}

function lookup_arduino_esptool() {
  local arduino dir

  arduino=`lookup_arduino` # || return 1
  test -d "${arduino}/packages/esp32/tools/esptool_py" # || return 1

  find \
    "${arduino}/packages/esp32/tools/esptool_py" \
    -type f -name esptool \
    -mindepth 2 -maxdepth 2 \
  | sort -Vr \
  | head -n1
}

function lookup_arduino() {
  local suffix dir
  for suffix in .arduino15 Library/Arduino15; do
    dir="${HOME}/${suffix}"
    test -d "$dir" || continue
    echo "$dir"

    return 0
  done

  return 1
}

function parse_cli_parameters() {
  local offset

  test ${#@} -ge 3 || usage "not enough arguments"

  serial_port="$1"
  shift

  test -c "$serial_port" || usage "serial-port $serial_port is not a device or does not exist"

  test $(( ${#@} % 2)) = 0 || usage "offset and partition must be given in pairs"

  while [ ${#@} -gt 0 ]
  do
    offset="$1"
    shift
    test -f "$1" || usage "partition $1 at offset $offset not found"
    shift
  done
}

function die() {
  echo "😵 ${@}" 1>&2
  exit 1
}

function usage() {
  test -z "${@}" || echo -e "${@}\n" 1>&2
  cat 1>&2 <<EOT
Usage: $0 serial-port offset1 partition1 [offset2 partition2 ...]

serial-port The virtual USB serial port of the ESP32 connected to this computer, eg /dev/ttyUSB0
offset      The flash address, eg 0x1000
partition   The partition, eg ~/.arduino15/packages/esp32/hardware/esp32/1.0.6/tools/sdk/bin/bootloader_qio_80m.bin

esptool.py must be in PATH or installed as ESP32 package for Arduino.

For example:

$0 /dev/ttyUSB0 \\
  0xe000 ~/.arduino15/packages/esp32/hardware/esp32/1.0.6/tools/partitions/boot_app0.bin \\
  0x1000 ~/.arduino15/packages/esp32/hardware/esp32/1.0.6/tools/sdk/bin/bootloader_qio_80m.bin \\
  0x10000 build/app.bin \\
  0x8000 build/app.partitions.bin

EOT
  exit 1
}

serial_port=""
firmware_bin=""
partitions_bin=""
dryrun="${ESPTOOL_DRYRUN:-0}"

main "${@}"
