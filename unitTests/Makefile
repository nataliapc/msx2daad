.PHONY: clean test release

AS = sdasz80
AR = sdar
CC = sdcc
HEX2BIN = hex2bin

CXXFLAGS := -DUNIT_TESTS -DLANG_ES -DMSXDOS1 -DMSX2
OPFLAGS := --opt-code-size -pragma-define:CRT_ENABLE_STDIO=0
WRFLAGS := --less-pedantic --disable-warning 196 --disable-warning 84 --disable-warning 85
CCFLAGS := --code-loc 0x0180 --data-loc 0 -mz80 --no-std-crt0 --out-fmt-ihx $(OPFLAGS) $(WRFLAGS) $(CXXFLAGS)

TESTDIR = ./
SRCDIR = ../src/
LIBDIR = ../libs/
INCDIR = ../include/
OBJDIR = ./obj/
DIR_GUARD=@mkdir -p $(OBJDIR)

EMUSCRIPTS = -script ../emulation/boot.tcl

LIBS := dos.lib vdp.lib utils.lib
REL_LIBS := $(addprefix $(OBJDIR), crt0msx_msxdos_advanced.rel heap.rel assert.rel daad_condacts.rel) $(addprefix $(LIBDIR), $(LIBS))

PROGRAMS = $(OBJDIR)condacts.com

all: $(PROGRAMS)

$(OBJDIR)%.rel: $(SRCDIR)%.s
	@echo $(DOS_LIB_SRC)
	@echo "#### ASM $@"
	@$(DIR_GUARD)
	@$(AS) -o $@ $^

$(OBJDIR)%.rel: $(TESTDIR)%.c
	@echo "#### CC $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -c -o $@ $^

$(OBJDIR)%.rel: $(SRCDIR)%.c
	@echo "#### CC $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -c -o $@ $^

$(OBJDIR)condacts.com: $(REL_LIBS) $(TESTDIR)tests_condacts.c
	@echo "######## Compiling $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $(subst .com,.c,$^) -o $(subst .com,.ihx,$@)
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@)
	@echo "**** Copying .COM files to DSK/"
	@mv $(OBJDIR)*.com dsk/

clean:
	@rm -f dsk/tests.com
	@rm -f *.com *.asm *.lst *.sym *.bin *.ihx *.lk *.map *.noi *.rel
	@rm -f $(OBJDIR)/*

test: all
	@bash -c 'if pgrep -x "openmsx" > /dev/null \
	; then \
		echo "**** openmsx already running..." \
	; else \
#		openmsx -machine Philips_NMS_8245 -ext debugdevice -diska dsk/ $(EMUSCRIPTS) \
#		openmsx -machine turbor -ext debugdevice -diska dsk/ $(EMUSCRIPTS) \
#		openmsx -machine Sony_HB-F1XD -ext debugdevice -diska dsk/ $(EMUSCRIPTS) \
		openmsx -machine msx2plus -ext debugdevice -diska dsk/ $(EMUSCRIPTS) \
	; fi'
