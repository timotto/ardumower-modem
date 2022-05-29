#!/bin/sh

set -e

. "$(dirname "$0")/common.sh"

cd source-code

cd ui
npm i
cd ..

task build-ui
