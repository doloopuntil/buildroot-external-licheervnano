################################################################################
#
# sg2002-8051-demo
#
################################################################################

SG2002_8051_DEMO_VERSION = 1.0
SG2002_8051_DEMO_SITE = $(SG2002_8051_DEMO_PKGDIR)/src
SG2002_8051_DEMO_SITE_METHOD = local
SG2002_8051_DEMO_LICENSE = MIT
SG2002_8051_DEMO_LICENSE_FILES = 8051-mbox-loader.c

SG2002_8051_DEMO_SDCC_FLAGS = -mmcs51 --model-large --code-loc 0x0000 \
	--code-size 0x2000 --xram-loc 0x2000 --xram-size 0x0400 \
	--stack-auto --int-long-reent --std-c11

define SG2002_8051_DEMO_BUILD_CMDS
	$(TARGET_CC) $(TARGET_CFLAGS) -static -U_FORTIFY_SOURCE -o $(@D)/8051-mbox-loader \
		$(@D)/8051-mbox-loader.c
	sdcc $(SG2002_8051_DEMO_SDCC_FLAGS) -o $(@D)/8051-mbox-echo.ihx \
		$(@D)/8051-mbox-echo.c
	sdobjcopy -I ihex -O binary $(@D)/8051-mbox-echo.ihx \
		$(@D)/8051-mbox-echo.bin
endef

define SG2002_8051_DEMO_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/8051-mbox-loader \
		$(TARGET_DIR)/usr/bin/8051-mbox-loader
	$(INSTALL) -D -m 0644 $(@D)/8051-mbox-echo.bin \
		$(TARGET_DIR)/usr/share/sg2002-8051-demo/8051-mbox-echo.bin
endef

$(eval $(generic-package))
