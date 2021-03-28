LED-PUZZLE_VERSION = 1.0
LED-PUZZLE_DEPENDENCIES = ncurses

define LED-PUZZLE_BUILD_CMDS
        $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define LED-PUZZLE_INSTALL_TARGET_CMDS
        $(INSTALL) -D -m 0755 $(@D)/helloworld $(TARGET_DIR)/bin
endef

$(eval $(generic-package))
