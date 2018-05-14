.PHONY: all clean

CC:=gcc
CFLAGS:=-Wall -g3 -ggdb -fPIC  
CFLAGS+= -I. -I../common
LDFLAGS:=
LDLIBS:=
AR:=ar
ARFLAGS:=cq

GENSRCS:=libelf_fsize.c libelf_msize.c libelf_convert.c
srcs:=elf.c						\
elf_begin.c						\
elf_cntl.c						\
elf_end.c elf_errmsg.c elf_errno.c			\
elf_data.c						\
elf_fill.c						\
elf_flag.c						\
elf_getarhdr.c						\
elf_getarsym.c						\
elf_getbase.c						\
elf_getident.c						\
elf_hash.c						\
elf_kind.c						\
elf_memory.c						\
elf_next.c						\
elf_open.c						\
elf_rand.c						\
elf_rawfile.c						\
elf_phnum.c						\
elf_shnum.c						\
elf_shstrndx.c						\
elf_scn.c						\
elf_strptr.c						\
elf_update.c						\
elf_version.c						\
gelf_cap.c						\
gelf_checksum.c						\
gelf_dyn.c						\
gelf_ehdr.c						\
gelf_getclass.c						\
gelf_fsize.c						\
gelf_move.c						\
gelf_phdr.c						\
gelf_rel.c						\
gelf_rela.c						\
gelf_shdr.c						\
gelf_sym.c						\
gelf_syminfo.c						\
gelf_symshndx.c						\
gelf_xlate.c						\
libelf_align.c						\
libelf_allocate.c					\
libelf_ar.c						\
libelf_ar_util.c					\
libelf_checksum.c					\
libelf_data.c						\
libelf_ehdr.c						\
libelf_extended.c					\
libelf_memory.c						\
libelf_open.c						\
libelf_phdr.c						\
libelf_shdr.c						\
libelf_xlate.c						\
${GENSRCS}
objsdir:=objdir
objs:=$(srcs:%.c=$(objsdir)/%.o)

exe:=libelf.a

all: $(exe)

$(exe): common m4s $(objs)
	$(AR) $(ARFLAGS) -o $@ $(objs) 

common:
	../common/native-elf-format > ../common/native-elf-format.h || rm ../common/native-elf-format.h	
	
m4s: $(GENSRCS)
libelf_convert.c:	elf_types.m4 libelf_convert.m4
	m4 -D SRCDIR=. libelf_convert.m4 > libelf_convert.c
libelf_fsize.c:		elf_types.m4 libelf_fsize.m4
	m4 -D SRCDIR=. libelf_fsize.m4 > libelf_fsize.c
libelf_msize.c:		elf_types.m4 libelf_msize.m4
	m4 -D SRCDIR=. libelf_msize.m4 > libelf_msize.c

$(objs): | $(objsdir)
$(objsdir):
	@mkdir -p $@

$(objsdir)/%.o: %.c
	$(CC) -E $(CFLAGS) -o $@.i $<	
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -rf $(exe) $(objsdir)
