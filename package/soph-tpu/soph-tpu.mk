################################################################################
#
# soph-tpu
#
################################################################################

SOPH_TPU_VERSION = 3bf6e4d22956d8d36644abac4b514d68ef8f0e81
SOPH_TPU_SITE = $(call github,varyhin,licheervnano,$(SOPH_TPU_VERSION))
SOPH_TPU_SITE_METHOD = git
SOPH_TPU_LICENSE = GPL-2.0
SOPH_TPU_LICENSE_FILES = LICENSE

SOPH_TPU_DEPENDENCIES = linux

SOPH_TPU_SRC = $(@D)/src/cvitek-tpu-vendor

define SOPH_TPU_APPLY_PATCHES_AND_OVERLAY
	cd $(SOPH_TPU_SRC) && \
	patch -p1 < $(SOPH_TPU_PKGDIR)/0001-forward-port-soph_tpu-kernel-6.18.patch
	cp $(SOPH_TPU_PKGDIR)/tpu_kernel_compat.h $(SOPH_TPU_SRC)/
	mkdir -p $(SOPH_TPU_SRC)/uapi
	cp $(SOPH_TPU_PKGDIR)/cvi_tpu_ioctl.h $(SOPH_TPU_SRC)/uapi/
endef
SOPH_TPU_POST_EXTRACT_HOOKS += SOPH_TPU_APPLY_PATCHES_AND_OVERLAY

define SOPH_TPU_BUILD_CMDS
	$(LINUX_MAKE_ENV) $(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) \
		M=$(SOPH_TPU_SRC) modules
endef

define SOPH_TPU_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/extra
	$(INSTALL) -D -m 0644 $(SOPH_TPU_SRC)/soph_tpu.ko \
		$(TARGET_DIR)/lib/modules/$(LINUX_VERSION_PROBED)/extra/soph_tpu.ko
	$(HOST_DIR)/sbin/depmod -a -b $(TARGET_DIR) $(LINUX_VERSION_PROBED)
endef

$(eval $(kernel-module))
$(eval $(generic-package))
