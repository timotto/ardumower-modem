#!/bin/bash

# git-tag.sh prints git details to stdout as compile time defines

set -euo pipefail

GIT_HASH=$(git log -1 --format="%h")
GIT_TIME=$(git log -1 --format="%cI")
echo -n "-DGIT_HASH=\"$GIT_HASH\" "
echo -n "-DGIT_TIME=\"$GIT_TIME\" "

tags=$(git log -1 --format="%D")
echo "${tags//, /,}" | tr , \\n \
| while read prefix value
  do
    test "$prefix" = "tag:" || continue
    echo -n "-DGIT_TAG=\"$value\" "
    break
  done
echo
