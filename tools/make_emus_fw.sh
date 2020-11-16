#!/bin/sh

set -e

MKFW=mkfw

PROJECT_NAME='caprice-esp32'
VERSION="$(cat version)"

APP_BIN=build/caprice-esp32.bin

$MKFW "${PROJECT_NAME}(${VERSION})" media/tile.raw 0 16 1048576 "$PROJECT_NAME" ${APP_BIN}
mv firmware.fw build/caprice-esp32.fw

cp build/caprice-esp32.fw release/caprice-esp32.fw
cp build/caprice-esp32.elf release/caprice-esp32.elf
