/*
 * newscn.c - implementation of the elf_newscn(3) function.
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
    "@(#) $Id: newscn.c,v 1.13 2008/05/23 08:15:35 michael Exp $";
#endif				/* lint */

/*
 * 	该函数用来设置e_shnum，当然这其中是要考虑特殊情况的，
 *	一旦参数shnum是特殊，则需要第一个section中相应的sh_size来存储
 */
int _elf_update_shnum(Elf *elf, size_t shnum)
{
	size_t extshnum = 0;
	Elf_Scn *scn;

	elf_assert(elf);
	elf_assert(elf->e_ehdr);

	scn = elf->e_scn_1;			// scn指向第一个Elf_Scn
	elf_assert(scn);			// 要确保存在第一个section，否则当Extended numbering的时候就没有意义了
	elf_assert(scn->s_index == 0);

	/*	这里要考虑到段数的特殊情况
     *	If the number of entries in the section header table is larger than or equal to SHN_LORESERVE (0xff00)
     *	e_shnum holds the value zero and the real number of entries in the section header table is held in the 
	 *	sh_size member of first section
   	 */
	if (shnum >= SHN_LORESERVE) {
		extshnum = shnum;
		shnum = 0;
	}
	if (elf->e_class == ELFCLASS32) {
		((Elf32_Ehdr *)elf->e_ehdr)->e_shnum = shnum;
		scn->s_shdr32.sh_size = extshnum;
	}
#if __LIBELF64
	else if (elf->e_class == ELFCLASS64) {
		((Elf64_Ehdr *)elf->e_ehdr)->e_shnum = shnum;
		scn->s_shdr64.sh_size = extshnum;
	}
#endif				/* __LIBELF64 */
	else {
		if (valid_class(elf->e_class)) {
			seterr(ERROR_UNIMPLEMENTED);
		} else {
			seterr(ERROR_UNKNOWN_CLASS);
		}
		return -1;
	}

	elf->e_ehdr_flags |= ELF_F_DIRTY;
	scn->s_shdr_flags |= ELF_F_DIRTY;

	return 0;
}

/*
 *	动态申请空间创建一个全新的Elf_Scn, 参数index用于指定这个section的序号，从0开始
 */
static Elf_Scn *_makescn(Elf *elf, size_t index)
{
	Elf_Scn *scn;

	elf_assert(elf);
	elf_assert(elf->e_magic == ELF_MAGIC);
	elf_assert(elf->e_ehdr);
	elf_assert(_elf_scn_init.s_magic == SCN_MAGIC);

	if (!(scn = (Elf_Scn *)malloc(sizeof(*scn)))) {
		seterr(ERROR_MEM_SCN);
		return NULL;
	}
	*scn = _elf_scn_init;
	scn->s_elf = elf;
	scn->s_scn_flags = ELF_F_DIRTY;
	scn->s_shdr_flags = ELF_F_DIRTY;
	scn->s_freeme = 1;
	scn->s_index = index;

	return scn;
}

/*
 *	获得第一个section对应的Elf_Scn的地址 如果已经存在 则直接返回 如果不存在，则动态申请一个
 */
Elf_Scn *_elf_first_scn(Elf *elf)
{
	Elf_Scn *scn;

	elf_assert(elf);
	elf_assert(elf->e_magic == ELF_MAGIC);

	if ((scn = elf->e_scn_1)) {	// 玛德,一开始我还以为是 == ，搞的我莫名其妙 
		return scn;				// 现在看来这行代码真的很巧妙 (如果已经有第一个section，则返回e_scn_1)
	}

	if ((scn = _makescn(elf, 0))) {	// 新建立一个Elf_Scn，index从0开始
		elf->e_scn_1 = elf->e_scn_n = scn;	// 使第一个section和最后一个section都指向新建立的section
		if (_elf_update_shnum(elf, 1)) {
			free(scn);				// 如果失败，则释放空间按，并且设置scn等于NULL，因为scn要返回
			elf->e_scn_1 = elf->e_scn_n = scn = NULL;
		}
	}

	return scn;
}

/*
 *	创建一个section的步骤: 
 *		1: 首先确保存在第一个Elf_Scn，因为第一个Elf_Scn并不对应一个真实的section，一般用于扩展使用
 *		2: 然后调用_makescn创建一个全新的Elf_Scn (其下标是e_scn_n的s_index + 1)
 *		3: 接着调用_elf_update_shnum更新一下elf->e_shnum，
 *		   当然该函数会考虑到特殊情况的，这也是第一步为什么非要存在一个Elf_Scn的原因 
 *		4: 最后更新下elf中的e_scn_n -- 即把刚创建的section追加到scn链表上
 */
static Elf_Scn *_buildscn(Elf *elf)
{
	Elf_Scn *scn;

	// 如果已经存在第一个section，则返回e_scn_1，否则动态申请一个，并返回之
	// 创建一个新的section之前，必须存在第一个（index == 0）的section
	if (!_elf_first_scn(elf)) {
		return NULL;
	}
	scn = elf->e_scn_n;
	elf_assert(scn);

	if (!(scn = _makescn(elf, scn->s_index + 1))) {
		return NULL;
	}

	if (_elf_update_shnum(elf, scn->s_index + 1)) {
		free(scn);
		return NULL;
	}
	// 从右向左
	elf->e_scn_n = elf->e_scn_n->s_link = scn;

	return scn;
}

Elf_Scn *elf_newscn(Elf *elf)
{
	Elf_Scn *scn;

	if (!elf) {
		return NULL;
	}
	elf_assert(elf->e_magic == ELF_MAGIC);

	if (!elf->e_readable && !elf->e_ehdr) {
		seterr(ERROR_NOEHDR);
	} else if (elf->e_kind != ELF_K_ELF) {
		seterr(ERROR_NOTELF);
	} else if (!elf->e_ehdr && !_elf_cook(elf)) {
		return NULL;
	} else if ((scn = _buildscn(elf))) {
		return scn;
	}

	return NULL;
}
