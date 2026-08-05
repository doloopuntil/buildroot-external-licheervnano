#!/bin/sh
set -e

BOARD_DIR="$(dirname "$0")"
BINARIES_DIR="${1}"

GENIMAGE_TMP="${BUILD_DIR}/genimage.tmp"
BOOTPATH_TMP="$(mktemp -d)"

trap 'rm -rf "${BOOTPATH_TMP}"' EXIT
rm -rf "${GENIMAGE_TMP}"

cp "${BINARIES_DIR}/fip.bin" \
   "${BINARIES_DIR}/Image" \
   "${BINARIES_DIR}/sg2002-licheerv-nano-b.dtb" \
   "${BINARIES_DIR}/sg2002-licheerv-nano-e.dtb" \
   "${BINARIES_DIR}/sg2002-licheerv-nano-w.dtb" \
   "${BINARIES_DIR}/sg2002-licheerv-nano-we.dtb" \
   "${BOOTPATH_TMP}/"

mkdir -p "${BOOTPATH_TMP}/extlinux"
cp "${BOARD_DIR}/extlinux.conf" "${BOOTPATH_TMP}/extlinux/extlinux.conf"

genimage \
	--rootpath "${BOOTPATH_TMP}" \
	--tmppath "${GENIMAGE_TMP}" \
	--inputpath "${BINARIES_DIR}" \
	--outputpath "${BINARIES_DIR}" \
	--config "${BOARD_DIR}/genimage.cfg"

exit $?
