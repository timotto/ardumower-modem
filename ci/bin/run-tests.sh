#!/bin/sh

set -e

artifacts="$PWD/rc-artifacts"

flash() {
  arduino-cli upload \
    --verbose \
    --port "${SERIAL_PORT}" \
    --fqbn esp32:esp32:lolin32 \
    --input-file "$artifacts/${1}.bin"
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
