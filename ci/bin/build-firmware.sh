#!/bin/bash

set -eo pipefail

. "$(dirname "$0")/common.sh"

rm -rf /var/cache/apt
ln -s $PWD/cache/apt /var/cache/apt
rm -f /etc/apt/apt.conf.d/docker-clean

apt update && apt install -y -qq git

artifacts="${PWD}/artifacts"
temp="${PWD}/temp"
mkdir -p "${temp}"

mkdir -p bin
export PATH=$PATH:$PWD/bin

tar -zxf tool-task/task_linux_amd64.tar.gz -C bin
cp tool-semver-bumper/semver-bumper-* bin/semver-bumper
chmod +x bin/semver-bumper

arduino-cli lib install AUnit@1.6.1
arduino-cli lib install ArduinoJson@6.19.3
arduino-cli lib install ArduinoWebsockets@0.5.3
arduino-cli lib install MQTT@2.5.0
arduino-cli lib install NimBLE-Arduino@1.3.7
arduino-cli lib install AsyncTCP@1.1.4
arduino-cli lib install "ESP Async WebServer"@2.9.0

cd ardumower-modem

semver-bumper \
  -o "$temp/release-version" \
  -c "$temp/commitlog"

semver-bumper \
  --pre build \
  -o "$temp/rc-version"

echo "v$(cat $temp/release-version)" > "$temp/release-name"
echo "v$(cat $temp/rc-version)" > "$temp/rc-name"

while read hash message
do
  test -n "$hash" || continue
  echo "${hash:0:7} $message"
done \
< "$temp/commitlog" \
>> "$temp/notes"

echo "master" > "$temp/commitish"

cat "$temp/release-name" > "$temp/release-notes"
cat "$temp/rc-name" > "$temp/rc-notes"
cat "$temp/notes" >> "$temp/release-notes"
cat "$temp/notes" >> "$temp/rc-notes"

git tag "$(cat "${temp}/rc-name")"

task build

cpy() {
  variant="$1"
  target="$2"
  for file in bin elf partitions.bin
  do
    cp -v \
      "build/dist/${variant}/ardumower-modem.ino.${file}" \
      "${temp}/ardumower-modem${target}.${file}"
  done
}

cpy ESP_MODEM_APP ""
cpy ESP_MODEM_SIM "_sim"
cpy ESP_MODEM_TEST "_test"

cp -v ~/.arduino15/packages/esp32/hardware/esp32/1.0.6/tools/partitions/boot_app0.bin "${temp}/"
cp -v ~/.arduino15/packages/esp32/hardware/esp32/1.0.6/tools/sdk/bin/bootloader_qio_80m.bin "${temp}/"

tar \
  -czv \
  -f "${artifacts}/firmware-artifacts.tgz" \
  -C "${temp}/" \
  .
