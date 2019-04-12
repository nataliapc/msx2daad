.PHONY: clean test release

AS = sdasz80
AR = sdar
CC = sdcc
HEX2BIN = hex2bin

DEFINES := -DDEBUG -DVERBOSE -DVERBOSE2 -DSCREEN=7
LDFLAGS := -rc
WRFLAGS := --less-pedantic --disable-warning 196 --disable-warning 84
CCFLAGS := --code-loc 0x0106 --data-loc 0 -mz80 --no-std-crt0 --out-fmt-ihx --opt-code-size $(DEFINES) $(WRFLAGS)

SRCDIR = src/
SRCLIB = $(SRCDIR)libs/
LIBDIR = libs/
INCDIR = include/
OBJDIR = obj/

LIBS := dos.lib vdp.lib utils.lib
REL_LIBS := $(addprefix $(OBJDIR), crt0msx_msxdos.rel heap.rel daad.rel daad_condacts.rel) $(addprefix $(LIBDIR), $(LIBS))

PROGRAMS = msx2daad.com

all: $(PROGRAMS)

$(OBJDIR)%.rel: $(SRCDIR)%.s
	@echo $(DOS_LIB_SRC)
	@echo "#### ASM $@"
	$(AS) -o $@ $^

$(OBJDIR)%.rel: $(SRCDIR)%.c
	@echo "#### CC $@"
	$(CC) $(CCFLAGS) -I$(INCDIR) -c -o $@ $^

$(OBJDIR)%.c.rel: $(SRCLIB)%.c
	@echo "#### CC $@"
	$(CC) $(CCFLAGS) -I$(INCDIR) -c -o $@ $^

$(OBJDIR)%.s.rel: $(SRCLIB)%.s
	@echo "#### ASM $@"
	$(AS) -o $@ $^

$(LIBDIR)dos.lib: $(patsubst $(SRCLIB)%, $(OBJDIR)%.rel, $(wildcard $(SRCLIB)dos_*))
	@echo "######## Compiling $@"
	$(AR) $(LDFLAGS) $@ $^

$(LIBDIR)vdp.lib: $(patsubst $(SRCLIB)%, $(OBJDIR)%.rel, $(wildcard $(SRCLIB)vdp_*))
	@echo "######## Compiling $@"
	$(AR) $(LDFLAGS) $@ $^

$(LIBDIR)utils.lib: $(patsubst $(SRCLIB)%, $(OBJDIR)%.rel, $(wildcard $(SRCLIB)utils_*))
	@echo "######## Compiling $@"
	$(AR) $(LDFLAGS) $@ $^

msx2daad.com: $(REL_LIBS) $(SRCDIR)msx2daad.c
	@echo "######## Compiling $@"
	$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) asm.lib $(subst .com,.c,$^)
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@)


clean:
	@rm -f dsk/msx2daad.com
	@rm -f *.com *.asm *.lst *.sym *.bin *.ihx *.lk *.map *.noi *.rel
	@rm -f $(OBJDIR)/*
	@rm -f $(INCDIR)/*.com $(INCDIR)/*.asm $(INCDIR)/*.lst $(INCDIR)/*.sym $(INCDIR)/*.bin $(INCDIR)/*.ihx $(INCDIR)/*.lk $(INCDIR)/*.map $(INCDIR)/*.noi $(INCDIR)/*.rel
	@rm -f $(addprefix $(LIBDIR), $(LIBS))


test: release
	@bash -c 'if pgrep -x "openmsx" > /dev/null \
	; then \
		echo "**** openmsx already running..." \
	; else \
		openmsx -machine Philips_NMS_8245 -ext msxdos2 -ext debugdevice -diska dsk/ -script ./emulation/boot.tcl \
	; fi'


release: all
	@echo "**** Copying .COM files to DSK/"
	@cp *.com dsk/
#	@echo "**** Updating .COM files in 'test.dsk'"
#	@dsktool a test.dsk $(PROGRAMS) > /dev/null
