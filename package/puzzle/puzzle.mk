PUZZLE_VERSION = 1.0
PUZZLE_DEPENDENCIES = libgpiod
PUZZLE_INSTALL_TARGET:=YES
PUZZLE_SITE:=../package/puzzle
PUZZLE_SITE_METHOD:=local


define PUZZLE_BUILD_CMDS
	$(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) all
endef

define PUZZLE_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 0755 $(@D)/puzzle $(TARGET_DIR)/bin
endef

$(eval $(generic-package))
