.PHONY: clean test legacy

SDCC_VER := 4.5.0
DOCKER_IMG = nataliapc/sdcc:$(SDCC_VER)
DOCKER_RUN = docker run -i --rm -u $(shell id -u):$(shell id -g) -v $(realpath $(CURDIR)/..):/src -w /src/unitTests $(DOCKER_IMG)

AS = $(DOCKER_RUN) sdasz80
AR = $(DOCKER_RUN) sdar
CC = $(DOCKER_RUN) sdcc
HEX2BIN = $(DOCKER_RUN) hex2bin

CXXFLAGS := -DUNIT_TESTS -DLANG_ES -DMSXDOS1 -DMSX2
OPFLAGS := --opt-code-size
WRFLAGS := --less-pedantic --disable-warning 196 --disable-warning 84 --disable-warning 85
CCFLAGS := --code-loc 0x0180 --data-loc 0 -mz80 --no-std-crt0 --out-fmt-ihx $(OPFLAGS) $(WRFLAGS) $(CXXFLAGS)

SRCDIR = ./src/
DAADSRCDIR = ../src/
DAAD_SRCDIR2 = ../src/daad/
LIBDIR = ../libs/
INCDIR = ../include/
OBJDIR = ./obj/
DIR_GUARD=@mkdir -p $(OBJDIR)

EMUSCRIPTS = -script ./emulation/boot.tcl

LIBS := dos.lib vdp.lib utils.lib
REL_LIBS          := $(addprefix $(OBJDIR), crt0msx_msxdos_advanced.rel heap.rel assert.rel daad_condacts.rel) \
                     $(addprefix $(LIBDIR), $(LIBS))
DOS_REL_LIBS      := $(addprefix $(OBJDIR), crt0msx_msxdos_advanced.rel heap.rel assert.rel) \
                     $(addprefix $(LIBDIR), $(LIBS))
CONDACTS_BASE_LIBS := $(addprefix $(OBJDIR), crt0msx_msxdos_advanced.rel heap.rel assert.rel condacts_stubs.rel daad_condacts.rel daad_objects.rel daad_getObjectWeight.rel) \
                     $(addprefix $(LIBDIR), $(LIBS))

DAAD_BASE_LIBS := $(addprefix $(OBJDIR), crt0msx_msxdos_advanced.rel heap.rel assert.rel daad_stubs.rel) \
                  $(addprefix $(LIBDIR), $(LIBS))

CONDACTS_PROGRAMS = condact1.com condact2.com condact3.com condact4.com condact5.com condact6.com
DAAD_PROGRAMS = objwght.com daadobjs.com gettoken.com initobj.com
PROGRAMS = dos.com $(CONDACTS_PROGRAMS) $(DAAD_PROGRAMS)

all: $(addprefix $(OBJDIR), $(PROGRAMS))

$(OBJDIR)%.rel: $(SRCDIR)%.s
	@echo $(DOS_LIB_SRC)
	@echo "#### ASM $@"
	@$(DIR_GUARD)
	@$(AS) -g -o $@ $^ ;

$(OBJDIR)%.rel: $(SRCDIR)%.c
	@echo "#### CC $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -c -o $@ $^ ;

$(OBJDIR)%.rel: $(DAADSRCDIR)%.c
	@echo "#### CC $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -c -o $@ $^ ;

$(OBJDIR)%.rel: $(DAAD_SRCDIR2)%.c
	@echo "#### CC $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -c -o $@ $^ ;

$(addprefix $(OBJDIR), $(CONDACTS_PROGRAMS)): $(OBJDIR)condact%.com: $(CONDACTS_BASE_LIBS) $(SRCDIR)tests_condacts%.c
	@echo "######## Compiling $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $^ -o $(subst .com,.ihx,$@) ;
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@) ;
	@echo "**** Copying .COM files to DSK/"
	@cp $@ dsk/

$(OBJDIR)objwght.com: $(DAAD_BASE_LIBS) $(OBJDIR)daad_getObjectWeight.rel $(SRCDIR)tests_daad_getObjectWeight.c
	@echo "######## Compiling $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $^ -o $(subst .com,.ihx,$@) ;
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@) ;
	@echo "**** Copying .COM files to DSK/"
	@cp $@ dsk/

$(OBJDIR)daadobjs.com: $(DAAD_BASE_LIBS) $(OBJDIR)daad_objects.rel $(OBJDIR)daad_getObjectWeight.rel $(SRCDIR)tests_daad_objects.c
	@echo "######## Compiling $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $^ -o $(subst .com,.ihx,$@) ;
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@) ;
	@echo "**** Copying .COM files to DSK/"
	@cp $@ dsk/

$(OBJDIR)gettoken.com: $(DAAD_BASE_LIBS) $(OBJDIR)daad_getToken.rel $(SRCDIR)tests_daad_getToken.c
	@echo "######## Compiling $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $^ -o $(subst .com,.ihx,$@) ;
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@) ;
	@echo "**** Copying .COM files to DSK/"
	@cp $@ dsk/

$(OBJDIR)initobj.com: $(DAAD_BASE_LIBS) $(OBJDIR)daad_init.rel $(OBJDIR)daad_getObjectWeight.rel $(SRCDIR)tests_daad_init.c
	@echo "######## Compiling $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $^ -o $(subst .com,.ihx,$@) ;
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@) ;
	@echo "**** Copying .COM files to DSK/"
	@cp $@ dsk/

$(OBJDIR)dos.com: $(DOS_REL_LIBS) $(SRCDIR)tests_dos.c
	@echo "######## Compiling $@"
	@$(DIR_GUARD)
	@$(CC) $(CCFLAGS) -I$(INCDIR) -L$(LIBDIR) $(subst .com,.c,$^) -o $(subst .com,.ihx,$@) ;
	@$(HEX2BIN) -e com $(subst .com,.ihx,$@) ;
	@echo "**** Copying .COM files to DSK/"
	@cp $@ dsk/

clean:
	@rm -f $(addprefix dsk/, dos.com $(CONDACTS_PROGRAMS) $(DAAD_PROGRAMS) tests.txt)
	@rm -f *.com *.asm *.lst *.sym *.bin *.ihx *.lk *.map *.noi *.rel
	@rm -rf $(OBJDIR)

test: all
	@bash -c 'if pgrep -x "openmsx" > /dev/null \
	; then \
		echo "**** openmsx already running..." \
	; else \
#		openmsx -machine Philips_NMS_8245 -ext debugdevice -diska dsk/ $(EMUSCRIPTS) \
#		openmsx -machine Sony_HB-F1XD -ext debugdevice -diska dsk/ $(EMUSCRIPTS) \
#		openmsx -machine msx2plus -ext debugdevice -diska dsk/ $(EMUSCRIPTS) \
		openmsx -machine turbor -ext debugdevice -diska dsk/ $(EMUSCRIPTS) \
	; fi'
	@if [ -f dsk/tests.txt ]; then \
		OK=$$(grep -c "\.\.\. OK$$" dsk/tests.txt || true) ; \
		FAIL=$$(grep -c "### Assert failed" dsk/tests.txt || true) ; \
		TODO=$$(grep -c "### TODO:" dsk/tests.txt || true) ; \
		TOTAL=$$((OK + FAIL + TODO)) ; \
		{ \
		echo "" ; \
		echo "========================================" ; \
		echo " Test results  (dsk/tests.txt)" ; \
		echo "   OK:    $$OK / $$TOTAL" ; \
		echo "   FAIL:  $$FAIL / $$TOTAL" ; \
		echo "   TODO:  $$TODO / $$TOTAL" ; \
		echo "   TOTAL: $$TOTAL" ; \
		echo "========================================" ; \
		} | tee -a dsk/tests.txt ; \
	else \
		echo "**** dsk/tests.txt not found" ; \
	fi

legacy:
	cd legacy ; ./compile.sh
	cd legacy ; dosbox run.bat
