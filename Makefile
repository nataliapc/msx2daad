.PHONY: clean test release

AS = sdasz80
AR = sdar
CC = sdcc
HEX2BIN = hex2bin

DEFINES := -D_DEBUG -D_VERBOSE -D_VERBOSE2 -DLANG_ES -DMSX2 -D_MSXDOS2
LDFLAGS := -rc
WRFLAGS := --less-pedantic --disable-warning 196 --disable-warning 84
CCFLAGS := --code-loc 0x0106 --data-loc 0 -mz80 --no-std-crt0 --out-fmt-ihx --opt-code-size $(DEFINES) $(WRFLAGS)

SRCDIR = src/
SRCLIB = $(SRCDIR)libs/
LIBDIR = libs/
INCDIR = include/
OBJDIR = obj/
DIR_GUARD=@mkdir -p $(OBJDIR)


LIBS := dos.lib vdp.lib utils.lib
REL_LIBS := $(addprefix $(OBJDIR), crt0msx_msxdos.rel heap.rel daad.rel daad_condacts.rel daad_platform_msx2.rel) $(addprefix $(LIBDIR), $(LIBS))

PROGRAMS = msx2daad.com

all: $(PROGRAMS)

$(OBJDIR)%.rel: $(SRCDIR)%.s
	@echo $(DOS_LIB_SRC)
	@echo "#### ASM $@"
	$(DIR_GUARD)
	$(AS) -o $@ $^

$(OBJDIR)%.rel: $(SRCDIR)%.c
	@echo "#### CC $@"
	$(DIR_GUARD)
	$(CC) $(CCFLAGS) -I$(INCDIR) -c -o $@ $^

$(OBJDIR)%.c.rel: $(SRCLIB)%.c
	@echo "#### CC $@"
	$(DIR_GUARD)
	$(CC) $(CCFLAGS) -I$(INCDIR) -c -o $@ $^

$(OBJDIR)%.s.rel: $(SRCLIB)%.s
	@echo "#### ASM $@"
	$(DIR_GUARD)
	$(AS) -o $@ $^

$(LIBDIR)dos.lib: $(patsubst $(SRCLIB)%, $(OBJDIR)%.rel, $(wildcard $(SRCLIB)dos_*))
	@echo "######## Compiling $@"
	$(DIR_GUARD)
	$(AR) $(LDFLAGS) $@ $^

$(LIBDIR)vdp.lib: $(patsubst $(SRCLIB)%, $(OBJDIR)%.rel, $(wildcard $(SRCLIB)vdp_*))
	@echo "######## Compiling $@"
	$(DIR_GUARD)
	$(AR) $(LDFLAGS) $@ $^

$(LIBDIR)utils.lib: $(patsubst $(SRCLIB)%, $(OBJDIR)%.rel, $(wildcard $(SRCLIB)utils_*))
	@echo "######## Compiling $@"
	$(DIR_GUARD)
	$(AR) $(LDFLAGS) $@ $^

msx2daad.com: $(REL_LIBS) $(SRCDIR)msx2daad.c
	@echo "######## Compiling $@"
	$(DIR_GUARD)
	$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) asm.lib $(subst .com,.c,$^)
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@)


clean:
	@rm -f dsk/msx2daad.com
	@rm -f *.com *.asm *.lst *.sym *.bin *.ihx *.lk *.map *.noi *.rel
	@rm -f $(OBJDIR)/*
	@rm -f $(addprefix $(LIBDIR), $(LIBS))


prepro: $(INCDIR)daad_defines.h clean
	@php bin/precomp.php dsk/DAAD.DDB $(INCDIR)daad_defines.h


test: release
	@bash -c 'if pgrep -x "openmsx" > /dev/null \
	; then \
		echo "**** openmsx already running..." \
	; else \
		openmsx -machine msx2_eu -ext ram1mb -ext debugdevice -diska dsk/ -script ./emulation/boot.tcl \
	; fi'


release: all
	@echo "**** Copying .COM files to DSK/"
	@cp *.com dsk/
#	@echo "**** Updating .COM files in 'test.dsk'"
#	@dsktool a test.dsk $(PROGRAMS) > /dev/null
