#!/bin/sh

set -e

artifacts="$PWD/test-artifacts"

cd tests/util/serial_test_runner
GOOS=linux GOARCH=arm64 go build -o "$artifacts/serial_test_runner" .
cd -

cd tests/util/validation
GOOS=linux GOARCH=arm64 go build -o "$artifacts/validate" ./cmd/validate
cd -
