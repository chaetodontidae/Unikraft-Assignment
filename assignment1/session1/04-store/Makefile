UK_ROOT ?= $(HOME)/.unikraft/unikraft
UK_LIBS ?= $(HOME)/.unikraft/libs
LIBS := $(UK_LIBS)/musl:$(UK_LIBS)/lwip:$(UK_LIBS)/nginx

all:
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L=$(LIBS)

$(MAKECMDGOALS):
	@$(MAKE) -C $(UK_ROOT) A=$(PWD) L=$(LIBS) $(MAKECMDGOALS)
