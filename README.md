# Buildroot External for Sipeed LicheeRV Nano

A [Buildroot](https://buildroot.org) external tree that produces a bootable
SD card image for the **Sipeed LicheeRV Nano** board — a RISC-V single-board
computer based on the Sophgo SG2002 SoC (C906 core @ 1 GHz, 256 MB DDR3,
0.5 TOPS NPU).

## About this project

This is a Buildroot conversion of the excellent
**[varyhin/licheervnano](https://github.com/varyhin/licheervnano)** project,
which pioneered the mainline-stack boot chain (FSBL → OpenSBI → U-Boot →
Linux) for the SG2002. All credit for the device tree patches, hardware
enablement, and upstream version pins goes to the original author and
contributors of that repository. This project repackages their work
into Buildroot's BR2_EXTERNAL framework for a more familiar embedded
Linux build experience.

A single SD card image supports all four board variants out of the box:

| Variant | Network | DTB |
|---------|---------|-----|
| **B**  | None                | `sg2002-licheerv-nano-b.dtb`  |
| **E**  | 100M Ethernet       | `sg2002-licheerv-nano-e.dtb`  |
| **W**  | WiFi 6 / BT 5       | `sg2002-licheerv-nano-w.dtb`  |
| **WE** | Ethernet + WiFi / BT | `sg2002-licheerv-nano-we.dtb` |

The variant is selected at boot via U-Boot's extlinux menu (default: W).

## Component versions

| Component | Version | Patches |
|-----------|---------|---------|
| Linux     | 6.18.29 | 20 patches (DTS, audio, Ethernet, USB, WiFi, TPU, thermal, RTC, watchdog, pinmux) |
| U-Boot    | 2025.10 | 4 patches (load addresses, eFuse MAC, sysreset) |
| OpenSBI   | 1.8.1   | 1 patch (CV1800B sysreset driver) |
| FSBL (BL2)| d4003f1 | 5 patches (mainline toolchain compatibility) |

## Prerequisites

- Buildroot 2026.05 or later
- Host: Linux amd64 with typical development packages

## Build

```sh
cd /path/to/buildroot
make BR2_EXTERNAL=/path/to/buildroot-external-licheerv-nano sipeed_licheerv_nano_defconfig
make -j$(nproc)
```

The first build takes ~30–60 minutes (toolchain + kernel). Subsequent
builds are incremental.

### Output

```
output/images/
├── sdcard.img                        ← flash this to SD card
├── boot.vfat                         ← intermediate VFAT boot image
├── rootfs.ext4                       ← intermediate ext4 rootfs image
├── fip.bin                           ← FIP container (see below)
├── Image                             ← Linux kernel
├── fw_dynamic.bin                    ← OpenSBI with embedded DTB
├── u-boot.bin                        ← U-Boot
├── cv181x.bin                        ← FSBL (BL2)
├── ddr_param.bin / cvirtos.bin       ← vendor boot blobs
├── sg2002-licheerv-nano-{b,e,w,we}.dtb
└── extlinux.conf
```

## Flash

Write the whole image to the SD card device (not a partition):

```sh
dd if=output/images/sdcard.img of=/dev/sdX bs=4M conv=fsync
```

Or use balenaEtcher / Raspberry Pi Imager.

### Access

- **Serial console**: UART0 TX/RX on the 2×14 header, 115200 8N1
- **Root login**: set via `make menuconfig` → System configuration
- **Network**: Ethernet `end0` uses DHCP (if E/WE variant)

## SD card layout

```
+-------------------------------------+
| MBR                                 |
+-------------------------------------+
| Partition 1: FAT16, ~132 MB (boot)  |
|   /fip.bin                          |
|   /Image                            |
|   /*.dtb                            |
|   /extlinux/extlinux.conf           |
+-------------------------------------+
| Partition 2: ext4, ~256 MB (root)   |
|   Buildroot root filesystem         |
+-------------------------------------+
```

## What is FIP?

**FIP** (Firmware Image Package) is a container format defined by Arm
Trusted Firmware, also adopted by Sophgo for the SG2002 boot ROM. On
this SoC the chip's mask ROM only understands FAT file systems and
expects to find a `fip.bin` file on the SD card's first partition.

The FIP container bundles the entire early boot chain into a single
blob that the BootROM parses:

```
fip.bin
├── BL2         cv181x.bin        FSBL (first stage bootloader, DDR init)
├── DDR_PARAM   ddr_param.bin     DDR3 timing parameters
├── BLCP_2ND    cvirtos.bin       FreeRTOS firmware for the small co-processor
├── MONITOR     fw_dynamic.bin    OpenSBI (M-mode runtime, SBI services)
└── LOADER_2ND  u-boot.bin        U-Boot (S-mode, distro boot, extlinux)
```

**Boot flow:**

```
BootROM (mask ROM)
  → reads fip.bin from FAT partition
  → FSBL initialises DDR3, loads remaining FIP components
  → OpenSBI provides SBI services (timer, IPI, reset)
  → U-Boot scans for extlinux.conf, shows variant menu
  → Linux 6.18 with selected device tree
```

The BootROM imposes the FAT partition requirement for `fip.bin`. The
extlinux.conf and kernel Image can live on the same partition.

## License

The Buildroot external wrappers, configuration, and package definitions
in this repository are distributed under the MIT License. Each upstream
component (Linux, U-Boot, OpenSBI, FSBL) retains its own license.
Proprietary firmware blobs (`ddr_param.bin`, `cvirtos.bin`, AIC8800
Wi-Fi firmware) originate from the Sipeed SDK and varyhin/licheervnano
repositories.

## Acknowledgements

- **[varyhin/licheervnano](https://github.com/varyhin/licheervnano)** —
  the original mainline-stack build system for LicheeRV Nano. Device
  tree patches, hardware enablement, source pins, and boot chain
  architecture are adapted from this project.
- **Sipeed** — hardware design and vendor SDK.
- **Sophgo** — SG2002 SoC and FIP tooling.
