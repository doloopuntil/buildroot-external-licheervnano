################################################################################
#
# licheervnano-firmware
#
################################################################################

LICHEERVNANO_FIRMWARE_VERSION = custom
LICHEERVNANO_FIRMWARE_SITE = /tmp/licheervnano-ref
LICHEERVNANO_FIRMWARE_SITE_METHOD = local
LICHEERVNANO_FIRMWARE_INSTALL_IMAGES = YES
LICHEERVNANO_FIRMWARE_INSTALL_TARGET = YES
LICHEERVNANO_FIRMWARE_LICENSE = MIT, Proprietary
LICHEERVNANO_FIRMWARE_LICENSE_FILES = LICENSE

define LICHEERVNANO_FIRMWARE_BUILD_CMDS
	true
endef

define LICHEERVNANO_FIRMWARE_INSTALL_IMAGES_CMDS
	$(INSTALL) -D -m 0644 $(@D)/firmware/ddr_param.bin $(BINARIES_DIR)/ddr_param.bin
	$(INSTALL) -D -m 0644 $(@D)/firmware/cvirtos.bin $(BINARIES_DIR)/cvirtos.bin
endef

define LICHEERVNANO_FIRMWARE_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/firmware/aic8800_sdio/aic8800_and_aic8800D80
	$(INSTALL) -m 0644 $(@D)/firmware/aic8800_u03/* \
		$(@D)/firmware/aic8800d80_u02/* \
		$(TARGET_DIR)/lib/firmware/aic8800_sdio/aic8800_and_aic8800D80/
endef

$(eval $(generic-package))
