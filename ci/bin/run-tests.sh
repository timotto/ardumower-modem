#!/bin/sh

set -e

artifacts="$PWD/rc-artifacts"

flash_sh="$PWD/ci/ci/bin/flash.sh"

flash() {
  "$flash_sh" "${SERIAL_PORT}" \
    0xe000 "$artifacts/boot_app0.bin" \
    0x1000 "$artifacts/bootloader_qio_80m.bin" \
    0x10000 "$artifacts/${1}.bin" \
    0x8000 "$artifacts/${1}.partitions.bin"
}

target="$PWD/test-artifacts"

# task test
flash ardumower-modem_test
"${target}/serial_test_runner" ${SERIAL_PORT}

# task validate
flash ardumower-modem_sim

validate_retry() {
  for x in $(seq 1 3)
  do
    "${target}/validate" ${SERIAL_PORT} \
    && return 0 \
    || continue
  done
  return 1
}

validate_retry
