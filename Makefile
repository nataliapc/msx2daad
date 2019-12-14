.PHONY: clean test release

AS = sdasz80
AR = sdar
CC = sdcc
HEX2BIN = hex2bin

ifndef VERSION
	VERSION := 1.2
endif
ifndef CXXFLAGS
	CXXFLAGS := -DTEST -D_TRANSCRIPT -D_DEBUG -D_VERBOSE -D_VERBOSE2 -DLANG_ES -DMSXDOS1 -DMSX2
endif
LDFLAGS := -rc
WRFLAGS := --less-pedantic --disable-warning 196 --disable-warning 84
CCFLAGS := --code-loc 0x0180 --data-loc 0 -mz80 --no-std-crt0 --out-fmt-ihx --opt-code-size $(CXXFLAGS) $(WRFLAGS)

SRCDIR = src/
SRCLIB = $(SRCDIR)libs/
LIBDIR = libs/
INCDIR = include/
OBJDIR = obj/
PKGDIR = package/
DIR_GUARD=@mkdir -p $(OBJDIR)
LIB_GUARD=@mkdir -p $(LIBDIR)


LIBS := dos.lib vdp.lib utils.lib
REL_LIBS := $(addprefix $(OBJDIR), crt0msx_msxdos_advanced.rel heap.rel daad.rel daad_condacts.rel \
								daad_platform_msx2.rel) $(addprefix $(LIBDIR), $(LIBS))

PROGRAMS = msx2daad.com

all: $(PROGRAMS) bin/testdaad


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
	$(LIB_GUARD)
	$(AR) $(LDFLAGS) $@ $^

$(LIBDIR)vdp.lib: $(patsubst $(SRCLIB)%, $(OBJDIR)%.rel, $(wildcard $(SRCLIB)vdp_*))
	@echo "######## Compiling $@"
	$(LIB_GUARD)
	$(AR) $(LDFLAGS) $@ $^

$(LIBDIR)utils.lib: $(patsubst $(SRCLIB)%, $(OBJDIR)%.rel, $(wildcard $(SRCLIB)utils_*))
	@echo "######## Compiling $@"
	$(LIB_GUARD)
	$(AR) $(LDFLAGS) $@ $^


msx2daad.com: $(REL_LIBS) $(SRCDIR)msx2daad.c
	@echo "######## Compiling $@"
	$(DIR_GUARD)
	$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $(subst .com,.c,$^)
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@)
	@echo "**** Copying .COM files to DSK/"
	@cp *.com dsk/


clean:
	@rm -f dsk/msx2daad.com
	@rm -f *.com *.asm *.lst *.sym *.bin *.ihx *.lk *.map *.noi *.rel
	@rm -f $(OBJDIR)/*
	@rm -f $(addprefix $(LIBDIR), $(LIBS))


EN_SC5:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_EN -DSCREEN=5 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
EN_SC6:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_EN -DSCREEN=6 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
EN_SC7:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_EN -DSCREEN=7 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
EN_SC8:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_EN -DSCREEN=8 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
EN_SC10:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_EN -DSCREEN=10 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
EN_SC12:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_EN -DSCREEN=12 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
ES_SC5:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_ES -DSCREEN=5 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
ES_SC6:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_ES -DSCREEN=6 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
ES_SC7:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_ES -DSCREEN=7 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
ES_SC8:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_ES -DSCREEN=8 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com"  _package_single
ES_SC10:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_ES -DSCREEN=10 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
ES_SC12:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DMSX2 -DMSXDOS1 -DLANG_ES -DSCREEN=12 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
EN_SC8_TR:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DTRANSCRIPT -DMSX2 -DMSXDOS1 -DLANG_EN -DSCREEN=8 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
ES_SC8_TR:
	$(MAKE) $(MAKEFLAGS) CXXFLAGS="-DTRANSCRIPT -DMSX2 -DMSXDOS1 -DLANG_ES -DSCREEN=8 -DFONTWIDTH=6 -DVERSION=$(VERSION)" OUTFILE="msx2daad_$(VERSION)_$@.com" _package_single
_package_single: msx2daad.com
	@echo "####################### $(OUTFILE)"
	@mkdir -p $(PKGDIR)
	$(MAKE) clean
	$(MAKE) msx2daad.com -j
	@cp msx2daad.com $(PKGDIR)$(OUTFILE)
package: EN_SC5 EN_SC6 EN_SC7 EN_SC8 EN_SC10 EN_SC12 \
         ES_SC5 ES_SC6 ES_SC7 ES_SC8 ES_SC10 ES_SC12 \
		 EN_SC8_TR ES_SC8_TR


precomp: $(INCDIR)daad_defines.h clean
	@php bin/precomp.php dsk/DAAD.DDB $(INCDIR)daad_defines.h


test: all
	@bash -c 'if pgrep -x "openmsx" > /dev/null \
	; then \
		echo "**** openmsx already running..." \
	; else \
#		openmsx -machine Sony_HB-F1XD -ext debugdevice -diska dsk/ -script ./emulation/boot.tcl \
#		openmsx -machine turbor -ext debugdevice -diska dsk/ -script ./emulation/boot.tcl \
#		openmsx -machine Philips_NMS_8245 -ext debugdevice -diska dsk/ -script ./emulation/boot.tcl \
		openmsx -machine msx2plus -ext ram1mb -ext debugdevice -diska dsk/ -script ./emulation/boot.tcl \
	; fi'


disk: all
	@rm -f game.dsk
#	@dsktool c 2880 game.dsk
	@cd dsk ; dsktool a ../game.dsk *.*
	@echo "**** game.dsk generated ****"
	@bin/dsk2rom -c 0 -a game.dsk game.rom
	@echo "**** game.rom generated ****"


bin/testdaad: bin/testdaad.c
	gcc $^ -o $@


release: all
	@echo "**** Copying .COM files to DSK/"
	@cp *.com dsk/
#	@echo "**** Updating .COM files in 'test.dsk'"
#	@dsktool a test.dsk $(PROGRAMS) > /dev/null
