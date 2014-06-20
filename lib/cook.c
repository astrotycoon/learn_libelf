/*
 * cook.c - read and translate ELF files.
 * Copyright (C) 1995 - 2006 Michael Riepe
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <private.h>

#ifndef lint
static const char rcsid[] =
    "@(#) $Id: cook.c,v 1.29 2008/05/23 08:15:34 michael Exp $";
#endif				/* lint */

const Elf_Scn _elf_scn_init = INIT_SCN;
const Scn_Data _elf_data_init = INIT_DATA;

Elf_Type _elf_scn_type(unsigned t)
{
	switch (t) {
	    case SHT_DYNAMIC:
		    return ELF_T_DYN;
	    case SHT_DYNSYM:
		    return ELF_T_SYM;
	    case SHT_HASH:
		    return ELF_T_WORD;
	    case SHT_REL:
		    return ELF_T_REL;
	    case SHT_RELA:
		    return ELF_T_RELA;
	    case SHT_SYMTAB:
		    return ELF_T_SYM;
	    case SHT_SYMTAB_SHNDX:
		    return ELF_T_WORD;	/* XXX: really? */
#if __LIBELF_SYMBOL_VERSIONS
#if __LIBELF_SUN_SYMBOL_VERSIONS
	    case SHT_SUNW_verdef:
		    return ELF_T_VDEF;
	    case SHT_SUNW_verneed:
		    return ELF_T_VNEED;
	    case SHT_SUNW_versym:
		    return ELF_T_HALF;
#else				/* __LIBELF_SUN_SYMBOL_VERSIONS */
	    case SHT_GNU_verdef:
		    return ELF_T_VDEF;
	    case SHT_GNU_verneed:
		    return ELF_T_VNEED;
	    case SHT_GNU_versym:
		    return ELF_T_HALF;
#endif				/* __LIBELF_SUN_SYMBOL_VERSIONS */
#endif				/* __LIBELF_SYMBOL_VERSIONS */
	}
	return ELF_T_BYTE;
}

/*
 * Check for overflow on 32-bit systems
 */
#define overflow(a,b,t)	(sizeof(a) < sizeof(t) && (t)(a) != (b))

/* 如果类型t为ELF_T_EHDR，那么错误类型为ERROR_TRUNC_EHDR */
/* 如果类型t为ELF_T_PHDR，那么错误类型为ERROR_TRUNC_PHDR */
/* 都不是的话，那么错误类型为ERROR_INTERNAL */
#define truncerr(t) ((t) == ELF_T_EHDR ? ERROR_TRUNC_EHDR:	\
		    ((t) == ELF_T_PHDR ? ERROR_TRUNC_PHDR:	\
		    ERROR_INTERNAL))
/* 如果类型t为ELF_T_EHDR，那么错误类型为ERROR_MEM_EHDR */
/* 如果类型t为ELF_T_PHDR，那么错误类型为ERROR_MEM_PHDR */
/* 都不是的话，那么错误类型为ERROR_INTERNAL */
#define memerr(t)   ((t) == ELF_T_EHDR ?ERROR_MEM_EHDR:	\
		    ((t) == ELF_T_PHDR ? ERROR_MEM_PHDR:	\
		    ERROR_INTERNAL))

Elf_Data *_elf_xlatetom(const Elf *elf, Elf_Data *dst, const Elf_Data *src)
{
	if (elf->e_class == ELFCLASS32) {
		return elf32_xlatetom(dst, src, elf->e_encoding);	/* 在两种表现形式转化时是需要考虑编码方式的 */
	}
#if __LIBELF64
	else if (elf->e_class == ELFCLASS64) {
		return elf64_xlatetom(dst, src, elf->e_encoding);
	}
#endif				/* __LIBELF64 */
	seterr(ERROR_UNIMPLEMENTED);
	return NULL;
}

/* 本函数的目的: 将elf指向的ELF文件的类型为type，偏移为off的文本部分以内存的形式拷贝到buf，如果buf为NULL，则
  动态申请 */
/* 该文件的文本形式已经mmap到内存，但依然是文本形式 -- 遵循一定的特性，如大小端特性 */
static char *_elf_item(void *buf, Elf * elf, Elf_Type type, size_t off)
{
	Elf_Data src, dst;	/* src-->file  dst-->memory */

	elf_assert(valid_type(type));

	if (off < 0 || off > elf->e_size) {
		seterr(ERROR_OUTSIDE);
		return NULL;
	}

	src.d_type = type;					
	src.d_version = elf->e_version;		/* src的version赋值为传递进来的elf->e_version ---> 也即待处理文件的version */
	src.d_size = _fsize(elf->e_class, src.d_version, type);	// _fsize --> 文件形式大小
	elf_assert(src.d_size);
	if ((elf->e_size - off) < src.d_size) {
		seterr(truncerr(type));
		return NULL;
	}

	dst.d_version = _elf_version;		/* dst的version赋值为当前的_elf_version, 它有可能与elf->e_version是不同的 */
										/* _elf_version是用户通过elf_version传进来的，表示告诉该库我们的应用程序懂的版本 */
	dst.d_size = _msize(elf->e_class, dst.d_version, type);	// _msize --> 内存形式大小
	elf_assert(dst.d_size);

	if (!(dst.d_buf = buf) && !(dst.d_buf = malloc(dst.d_size))) {	/* 如果传递进来的buf是NULL，则为dst.d_buf动态申请内存空间 */
		seterr(memerr(type));
		return NULL;
	}

	elf_assert(elf->e_data);
	if (elf->e_rawdata) {
		src.d_buf = elf->e_rawdata + off;
	} else {
		src.d_buf = elf->e_data + off;
	}

	if (_elf_xlatetom(elf, &dst, &src)) {	/* 将src指向的ELF Header(文件表现形式) 转换为 dst指向的内存区域(内存表现形式) */
		return (char *)dst.d_buf;  	/* 在这里返回的话，在哪释放呢 */  /* --> 在elf_end函数中_elf_free(elf->e_ehdr)*/
	}
	/* 只有当_elf_xlatetom函数返回NULL(即函数返回失败)时才会执行到这里 */
	if (dst.d_buf != buf) {
		free(dst.d_buf);
	}

	return NULL;
}

static int _elf_cook_phdr(Elf * elf)
{
	size_t num, off, entsz;

	if (elf->e_class == ELFCLASS32) {	/* 首先根据获得的ELF header获取ELF program heaer table的偏移，entry个数，以及每个entry的大小 */
		num = ((Elf32_Ehdr *)elf->e_ehdr)->e_phnum;		/* 当然当e_phnum == PN_XNUM，那么其真实的个数在第一个section的sh_info中 */
		off = ((Elf32_Ehdr *)elf->e_ehdr)->e_phoff;
		entsz = ((Elf32_Ehdr *)elf->e_ehdr)->e_phentsize;
	}
#if __LIBELF64
	else if (elf->e_class == ELFCLASS64) {
		num = ((Elf64_Ehdr *) elf->e_ehdr)->e_phnum;
		off = ((Elf64_Ehdr *) elf->e_ehdr)->e_phoff;
		entsz = ((Elf64_Ehdr *) elf->e_ehdr)->e_phentsize;
		/*
		 * Check for overflow on 32-bit systems
		 */
		if (overflow(off, ((Elf64_Ehdr *)elf->e_ehdr)->e_phoff, Elf64_Off)) {
			seterr(ERROR_OUTSIDE);
			return 0;
		}
	}
#endif				/* __LIBELF64 */
	else {
		seterr(ERROR_UNIMPLEMENTED);
		return 0;
	}

	if (off) {
		Elf_Scn *scn;
		size_t size;
		unsigned i;
		char *p;

		/*	
		 * PN_XNUM(0xffff): Special value for e_phnum.  This indicates that the real number of
       	 *		 			program headers is too large to fit into e_phnum.  Instead the real
		 *					value is in the field sh_info of section 0. 
         */
		if (num == PN_XNUM) {
			/*
			 * Overflow in ehdr->e_phnum.
			 * Get real value from first SHDR.
			 */
			if (!(scn = elf->e_scn_1)) {
				seterr(ERROR_NOSUCHSCN);
				return 0;
			}
			if (elf->e_class == ELFCLASS32) {
				num = scn->s_shdr32.sh_info;
			}
#if __LIBELF64
			else if (elf->e_class == ELFCLASS64) {
				num = scn->s_shdr64.sh_info;
			}
#endif				/* __LIBELF64 */
			/* we already had this
			   else {
			   seterr(ERROR_UNIMPLEMENTED);
			   return 0;
			   }
			 */
		}

		size = _fsize(elf->e_class, elf->e_version, ELF_T_PHDR);
		elf_assert(size);
#if ENABLE_EXTENDED_FORMAT
		if (entsz < size) {
#else				/* ENABLE_EXTENDED_FORMAT */
		if (entsz != size) {
#endif				/* ENABLE_EXTENDED_FORMAT */
			seterr(ERROR_EHDR_PHENTSIZE);
			return 0;
		}

		size = _msize(elf->e_class, _elf_version, ELF_T_PHDR);
		elf_assert(size);

		if (!(p = malloc(num * size))) {
			seterr(memerr(ELF_T_PHDR));
			return 0;
		}
		for (i = 0; i < num; i++) {
			if (!_elf_item(p + i * size, elf, ELF_T_PHDR, off + i * entsz)) {
				free(p);
				return 0;
			}
		}
		elf->e_phdr = p;
		elf->e_phnum = num;
	}

	return 1;
}

static int _elf_cook_shdr(Elf * elf)
{
	size_t num, off, entsz;

	if (elf->e_class == ELFCLASS32) {
		num = ((Elf32_Ehdr *)elf->e_ehdr)->e_shnum;
		off = ((Elf32_Ehdr *)elf->e_ehdr)->e_shoff;
		entsz = ((Elf32_Ehdr *)elf->e_ehdr)->e_shentsize;
	}
#if __LIBELF64
	else if (elf->e_class == ELFCLASS64) {
		num = ((Elf64_Ehdr *) elf->e_ehdr)->e_shnum;
		off = ((Elf64_Ehdr *) elf->e_ehdr)->e_shoff;
		entsz = ((Elf64_Ehdr *) elf->e_ehdr)->e_shentsize;
		/*
		 * Check for overflow on 32-bit systems
		 */
		if (overflow(off, ((Elf64_Ehdr *) elf->e_ehdr)->e_shoff, Elf64_Off)) {
			seterr(ERROR_OUTSIDE);
			return 0;
		}
	}
#endif				/* __LIBELF64 */
	else {
		seterr(ERROR_UNIMPLEMENTED);
		return 0;
	}

	if (off) {
		struct tmp {
			Elf_Scn scn;
			Scn_Data data;
		} *head;
		Elf_Data src, dst;
		Elf_Scn *scn;
		Scn_Data *sd;
		unsigned i;

		if (off < 0 || off > elf->e_size) {
			seterr(ERROR_OUTSIDE);
			return 0;
		}

		src.d_type = ELF_T_SHDR;
		src.d_version = elf->e_version;
		src.d_size = _fsize(elf->e_class, src.d_version, ELF_T_SHDR);
		elf_assert(src.d_size);
#if ENABLE_EXTENDED_FORMAT
		if (entsz < src.d_size) {
#else				/* ENABLE_EXTENDED_FORMAT */
		if (entsz != src.d_size) {
#endif				/* ENABLE_EXTENDED_FORMAT */
			seterr(ERROR_EHDR_SHENTSIZE);
			return 0;
		}

		dst.d_version = EV_CURRENT;	/* 这里为什么不是_elf_version呢？*/

		if (num == 0) {	
			union {
				Elf32_Shdr sh32;
#if __LIBELF64
				Elf64_Shdr sh64;
#endif				/* __LIBELF64 */
			} u;

			/*
			 * Overflow in ehdr->e_shnum.
			 * Get real value from first SHDR.
			 */
			if (elf->e_size - off < entsz) {
				seterr(ERROR_TRUNC_SHDR);
				return 0;
			}
			if (elf->e_rawdata) {
				src.d_buf = elf->e_rawdata + off;
			} else {
				src.d_buf = elf->e_data + off;
			}
			dst.d_buf = &u;
			dst.d_size = sizeof(u);
			if (!_elf_xlatetom(elf, &dst, &src)) {
				return 0;
			}
			elf_assert(dst.d_size ==
				   _msize(elf->e_class, EV_CURRENT,
					  ELF_T_SHDR));
			elf_assert(dst.d_type == ELF_T_SHDR);
			if (elf->e_class == ELFCLASS32) {
				num = u.sh32.sh_size;
			}
#if __LIBELF64
			else if (elf->e_class == ELFCLASS64) {
				num = u.sh64.sh_size;
				/*
				 * Check for overflow on 32-bit systems
				 */
				if (overflow(num, u.sh64.sh_size, Elf64_Xword)) {
					seterr(ERROR_OUTSIDE);
					return 0;
				}
			}
#endif				/* __LIBELF64 */
		}

		if ((elf->e_size - off) / entsz < num) {
			seterr(ERROR_TRUNC_SHDR);
			return 0;
		}

		if (!(head = (struct tmp *)malloc(num * sizeof(struct tmp)))) {
			seterr(ERROR_MEM_SCN);
			return 0;
		}

		for (scn = NULL, i = num; i-- > 0;) {
			head[i].scn = _elf_scn_init;
			head[i].data = _elf_data_init;
			head[i].scn.s_link = scn;
			if (!scn) {
				elf->e_scn_n = &head[i].scn;
			}
			scn = &head[i].scn;
			sd = &head[i].data;

			if (elf->e_rawdata) {
				src.d_buf = elf->e_rawdata + off + i * entsz;
			} else {
				src.d_buf = elf->e_data + off + i * entsz;
			}

			dst.d_buf = &scn->s_uhdr;
			dst.d_size = sizeof(scn->s_uhdr);

			if (!_elf_xlatetom(elf, &dst, &src)) {
				elf->e_scn_n = NULL;
				free(head);
				return 0;
			}

			elf_assert(dst.d_size == _msize(elf->e_class, EV_CURRENT, ELF_T_SHDR));
			elf_assert(dst.d_type == ELF_T_SHDR);

			scn->s_elf = elf;
			scn->s_index = i;
			scn->s_data_1 = sd;
			scn->s_data_n = sd;

			sd->sd_scn = scn;

			if (elf->e_class == ELFCLASS32) {
				Elf32_Shdr *shdr = &scn->s_shdr32;

				scn->s_type = shdr->sh_type;
				scn->s_size = shdr->sh_size;
				scn->s_offset = shdr->sh_offset;
				sd->sd_data.d_align = shdr->sh_addralign;
				sd->sd_data.d_type = _elf_scn_type(scn->s_type);
			}
#if __LIBELF64
			else if (elf->e_class == ELFCLASS64) {
				Elf64_Shdr *shdr = &scn->s_shdr64;

				scn->s_type = shdr->sh_type;
				scn->s_size = shdr->sh_size;
				scn->s_offset = shdr->sh_offset;
				sd->sd_data.d_align = shdr->sh_addralign;
				/*
				 * Check for overflow on 32-bit systems
				 */
				if (overflow(scn->s_size, shdr->sh_size, Elf64_Xword)
				    || overflow(scn->s_offset, shdr->sh_offset, Elf64_Off)
				    || overflow(sd->sd_data.d_align, shdr->sh_addralign, Elf64_Xword)) {
					seterr(ERROR_OUTSIDE);
					return 0;
				}
				sd->sd_data.d_type = _elf_scn_type(scn->s_type);
				/*
				 * QUIRKS MODE:
				 *
				 * Some 64-bit architectures use 64-bit entries in the
				 * .hash section. This violates the ELF standard, and
				 * should be fixed. It's mostly harmless as long as the
				 * binary and the machine running your program have the
				 * same byte order, but you're in trouble if they don't,
				 * and if the entry size is wrong.
				 *
				 * As a workaround, I let libelf guess the right size
				 * for the binary. This relies pretty much on the fact
				 * that the binary provides correct data in the section
				 * headers. If it doesn't, it's probably broken anyway.
				 * Therefore, libelf uses a standard conforming value
				 * when it's not absolutely sure.
				 */
				if (scn->s_type == SHT_HASH) {
					int override = 0;

					/*
					 * sh_entsize must reflect the entry size
					 */
					if (shdr->sh_entsize == ELF64_FSZ_ADDR) {
						override++;
					}
					/*
					 * sh_size must be a multiple of sh_entsize
					 */
					if (shdr->sh_size % ELF64_FSZ_ADDR == 0) {
						override++;
					}
					/*
					 * There must be room for at least 2 entries
					 */
					if (shdr->sh_size >= 2 * ELF64_FSZ_ADDR) {
						override++;
					}
					/*
					 * sh_addralign must be correctly set
					 */
					if (shdr->sh_addralign == ELF64_FSZ_ADDR) {
						override++;
					}
					/*
					 * The section must be properly aligned
					 */
					if (shdr->sh_offset % ELF64_FSZ_ADDR == 0) {
						override++;
					}
					/* XXX: also look at the data? */
					/*
					 * Make a conservative decision...
					 */
					if (override >= 5) {
						sd->sd_data.d_type = ELF_T_ADDR;
					}
				}
				/*
				 * END QUIRKS MODE.
				 */
			}
#endif				/* __LIBELF64 */
			/* we already had this
			else {
				seterr(ERROR_UNIMPLEMENTED);
				return 0;
			}
			 */

			sd->sd_data.d_size = scn->s_size;
			sd->sd_data.d_version = _elf_version;
		}

		elf_assert(scn == &head[0].scn);
		elf->e_scn_1 = &head[0].scn;
		head[0].scn.s_freeme = 1;
	}

	return 1;
}

static int _elf_cook_file(Elf * elf)
{
	elf->e_ehdr = _elf_item(NULL, elf, ELF_T_EHDR, 0);
	if (!elf->e_ehdr) {
		return 0;
	}
	/*
	 * Note: _elf_cook_phdr may require the first section header!
	 */
	if (!_elf_cook_shdr(elf)) {
		return 0;
	}
	if (!_elf_cook_phdr(elf)) {
		return 0;
	}
	return 1;
}

int _elf_cook(Elf * elf)
{
	elf_assert(_elf_scn_init.s_magic == SCN_MAGIC);
	elf_assert(_elf_data_init.sd_magic == DATA_MAGIC);
	elf_assert(elf);
	elf_assert(elf->e_magic == ELF_MAGIC);
	elf_assert(elf->e_kind == ELF_K_ELF);
	elf_assert(!elf->e_ehdr);

	if (!valid_version(elf->e_version)) {
		seterr(ERROR_UNKNOWN_VERSION);
	} else if (!valid_encoding(elf->e_encoding)) {
		seterr(ERROR_UNKNOWN_ENCODING);
	} else if (valid_class(elf->e_class)) {
		return _elf_cook_file(elf);
	} else {
		seterr(ERROR_UNKNOWN_CLASS);
	}

	return 0;
}
