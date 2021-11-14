#!/bin/sh

set -e

. "$(dirname "$0")/common.sh"

cd build-ui
task package-ui
