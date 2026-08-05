################################################################################
#
# aic8800-modules
#
################################################################################

AIC8800_MODULES_VERSION = custom
AIC8800_MODULES_SITE = /tmp/licheervnano-ref
AIC8800_MODULES_SITE_METHOD = local
AIC8800_MODULES_LICENSE = GPL-2.0
AIC8800_MODULES_LICENSE_FILES = LICENSE

AIC8800_MODULES_DEPENDENCIES = linux

AIC8800_MODULES_SRC = $(@D)/src/aic8800-vendor

define AIC8800_MODULES_APPLY_PATCHES
	cd $(AIC8800_MODULES_SRC) && \
	for patchfile in $(sort $(wildcard $(AIC8800_MODULES_PKGDIR)/patches/*.patch)); do \
		patch -p1 < $$patchfile || exit 1; \
	done
endef
# Sources are pre-patched in the reference directory
# AIC8800_MODULES_POST_EXTRACT_HOOKS += AIC8800_MODULES_APPLY_PATCHES

define AIC8800_MODULES_BUILD_CMDS
	$(LINUX_MAKE_ENV) $(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) \
		M=$(AIC8800_MODULES_SRC)/aic8800_bsp \
		CONFIG_AIC_FW_PATH="/lib/firmware/aic8800_sdio/aic8800_and_aic8800D80" \
		modules
	$(LINUX_MAKE_ENV) $(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) \
		M=$(AIC8800_MODULES_SRC)/aic8800_fdrv \
		KBUILD_EXTRA_SYMBOLS=$(AIC8800_MODULES_SRC)/aic8800_bsp/Module.symvers \
		modules
	$(LINUX_MAKE_ENV) $(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) \
		M=$(AIC8800_MODULES_SRC)/aic8800_btlpm \
		KBUILD_EXTRA_SYMBOLS=$(AIC8800_MODULES_SRC)/aic8800_bsp/Module.symvers \
		modules
endef

define AIC8800_MODULES_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/extra/aic8800
	$(INSTALL) -D -m 0644 \
		$(AIC8800_MODULES_SRC)/aic8800_bsp/aic8800_bsp.ko \
		$(AIC8800_MODULES_SRC)/aic8800_fdrv/aic8800_fdrv.ko \
		$(AIC8800_MODULES_SRC)/aic8800_btlpm/aic8800_btlpm.ko \
		$(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/extra/aic8800/
	$(HOST_DIR)/sbin/depmod -a -b $(TARGET_DIR) $(LINUX_VERSION_PROBED)
endef

$(eval $(kernel-module))
$(eval $(generic-package))
