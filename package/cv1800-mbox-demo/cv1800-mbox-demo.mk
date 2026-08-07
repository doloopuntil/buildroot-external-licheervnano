################################################################################
#
# cv1800-mbox-demo
#
################################################################################

CV1800_MBOX_DEMO_VERSION = 1.0
CV1800_MBOX_DEMO_SITE = $(CV1800_MBOX_DEMO_PKGDIR)
CV1800_MBOX_DEMO_SITE_METHOD = local
CV1800_MBOX_DEMO_LICENSE = GPL-2.0
CV1800_MBOX_DEMO_LICENSE_FILES = cv1800_mbox_demo.c

CV1800_MBOX_DEMO_DEPENDENCIES = linux

define CV1800_MBOX_DEMO_BUILD_CMDS
	$(LINUX_MAKE_ENV) $(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) \
		M=$(@D) modules
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(@D)/mbox-demo $(@D)/mbox-demo.c
endef

define CV1800_MBOX_DEMO_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0644 $(@D)/cv1800_mbox_demo.ko \
		$(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/extra/cv1800_mbox_demo.ko
	$(HOST_DIR)/sbin/depmod -a -b $(TARGET_DIR) $(LINUX_VERSION_PROBED)
	$(INSTALL) -D -m 0755 $(@D)/mbox-demo $(TARGET_DIR)/usr/bin/mbox-demo
endef

$(eval $(kernel-module))
$(eval $(generic-package))
