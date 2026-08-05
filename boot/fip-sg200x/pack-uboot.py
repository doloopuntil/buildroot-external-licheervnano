#!/usr/bin/env python3
"""Wrap U-Boot in the SG2002 FIP BL33 loader header for the vendor packer."""

import argparse
import struct


LOADER_HEADER_SIZE = 32
RISCV_JUMP_TO_PAYLOAD = 0x0200006F
UBOOT_RUN_ADDRESS = 0x80200000


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    args = parser.parse_args()

    with open(args.input, "rb") as source:
        payload = source.read()

    header = struct.pack(
        "<IIIIQII",
        RISCV_JUMP_TO_PAYLOAD,
        int.from_bytes(b"BL33", "little"),
        0,
        LOADER_HEADER_SIZE + len(payload),
        UBOOT_RUN_ADDRESS,
        0,
        0,
    )

    with open(args.output, "wb") as output:
        output.write(header)
        output.write(payload)


if __name__ == "__main__":
    main()
