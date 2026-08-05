################################################################################
#
# fip-sg200x
#
################################################################################

FIP_SG200X_VERSION = 3bf6e4d22956d8d36644abac4b514d68ef8f0e81
FIP_SG200X_SITE = $(call github,varyhin,licheervnano,$(FIP_SG200X_VERSION))
FIP_SG200X_SITE_METHOD = git
FIP_SG200X_INSTALL_IMAGES = YES
FIP_SG200X_INSTALL_TARGET = NO
FIP_SG200X_LICENSE = MIT
FIP_SG200X_LICENSE_FILES = LICENSE
FIP_SG200X_DEPENDENCIES = fsbl-sg200x opensbi uboot licheervnano-firmware host-python3

define FIP_SG200X_BUILD_CMDS
	python3 $(@D)/src/fiptool/fiptool \
		--fsbl       $(BINARIES_DIR)/cv181x.bin \
		--ddr_param  $(BINARIES_DIR)/ddr_param.bin \
		--rtos       $(BINARIES_DIR)/cvirtos.bin \
		--opensbi    $(BINARIES_DIR)/fw_dynamic.bin \
		--uboot      $(BINARIES_DIR)/u-boot.bin \
		$(@D)/fip.bin
endef

define FIP_SG200X_INSTALL_IMAGES_CMDS
	$(INSTALL) -D -m 0644 $(@D)/fip.bin $(BINARIES_DIR)/fip.bin
endef

$(eval $(generic-package))
