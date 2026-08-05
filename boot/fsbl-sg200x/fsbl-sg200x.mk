################################################################################
#
# fsbl-sg200x
#
################################################################################

FSBL_SG200X_VERSION = d4003f15b35d43ad4842f427050ab2bba0114fa5
FSBL_SG200X_SITE = https://github.com/sipeed/LicheeRV-Nano-Build.git
FSBL_SG200X_SITE_METHOD = git
FSBL_SG200X_INSTALL_IMAGES = YES
FSBL_SG200X_INSTALL_TARGET = NO
FSBL_SG200X_LICENSE = Proprietary
FSBL_SG200X_LICENSE_FILES = LICENSE
FSBL_SG200X_DEPENDENCIES = host-python3

define FSBL_SG200X_BUILD_CMDS
	mkdir -p $(@D)/fsbl/build
	$(HOST_DIR)/bin/python3 $(@D)/build/scripts/mmap_conv.py --type h \
		$(@D)/build/boards/sg200x/sg2002_licheervnano_sd/memmap.py \
		$(@D)/fsbl/build/cvi_board_memmap.h
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/fsbl bl2 \
		CHIP_ARCH=cv181x ARCH=riscv BOOT_CPU=riscv \
		CROSS_COMPILE_GLIBC_RISCV64=$(TARGET_CROSS) \
		DDR_CFG=ddr3_1866_x16 TPU_PERF_MODE=y OD_CLK_SEL=y \
		BUILD_STRING=reproducible-bld \
		BUILD_MESSAGE_TIMESTAMP='"0000-00-00T00:00:00+00:00"' \
		CFLAGS="-fno-stack-protector" \
		O=$(@D)/fsbl/build
endef

define FSBL_SG200X_INSTALL_IMAGES_CMDS
	$(INSTALL) -D -m 0644 $(@D)/fsbl/build/bl2.bin $(BINARIES_DIR)/cv181x.bin
endef

$(eval $(generic-package))
