/*
getdata.c - implementation of the elf_getdata(3) function.
Copyright (C) 1995 - 2001 Michael Riepe

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
*/

#include <private.h>

#ifndef lint
static const char rcsid[] =
    "@(#) $Id: getdata.c,v 1.13 2008/05/23 08:15:34 michael Exp $";
#endif				/* lint */

static Elf_Data *_elf_cook_scn(Elf * elf, Elf_Scn * scn, Scn_Data * sd)
{
	Elf_Data dst;
	Elf_Data src;
	int flag = 0;
	size_t dlen;

	elf_assert(elf->e_data);

	/*
	 * Prepare source
	 */
	src = sd->sd_data;
	src.d_version = elf->e_version;
	if (elf->e_rawdata) {
		src.d_buf = elf->e_rawdata + scn->s_offset;
	} else {
		src.d_buf = elf->e_data + scn->s_offset;
	}

	/*
	 * Prepare destination (needs prepared source!)
	 */
	dst = sd->sd_data;
	if (elf->e_class == ELFCLASS32) {
		dlen = _elf32_xltsize(&src, dst.d_version, elf->e_encoding, 0);
	}
#if __LIBELF64
	else if (elf->e_class == ELFCLASS64) {
		dlen = _elf64_xltsize(&src, dst.d_version, elf->e_encoding, 0);
	}
#endif				/* __LIBELF64 */
	else {
		elf_assert(valid_class(elf->e_class));
		seterr(ERROR_UNIMPLEMENTED);
		return NULL;
	}
	if (dlen == (size_t) - 1) {
		return NULL;
	}
	dst.d_size = dlen;
	if (elf->e_rawdata != elf->e_data && dst.d_size <= src.d_size) {
		dst.d_buf = elf->e_data + scn->s_offset;
	} else if (!(dst.d_buf = malloc(dst.d_size))) {
		seterr(ERROR_MEM_SCNDATA);
		return NULL;
	} else {
		flag = 1;
	}

	/*
	 * Translate data
	 */
	if (_elf_xlatetom(elf, &dst, &src)) {
		sd->sd_memdata = (char *)dst.d_buf;
		sd->sd_data = dst;
		if (!(sd->sd_free_data = flag)) {
			elf->e_cooked = 1;
		}
		return &sd->sd_data;
	}

	if (flag) {
		free(dst.d_buf);
	}
	return NULL;
}

Elf_Data *elf_getdata(Elf_Scn *scn, Elf_Data *data)
{
	Scn_Data *sd;
	Elf *elf;

	if (!scn) {
		return NULL;
	}
	elf_assert(scn->s_magic == SCN_MAGIC);

	if (scn->s_index == SHN_UNDEF) {
		seterr(ERROR_NULLSCN);
	} else if (data) {
		for (sd = scn->s_data_1; sd; sd = sd->sd_link) {
			elf_assert(sd->sd_magic == DATA_MAGIC);
			elf_assert(sd->sd_scn == scn);

			if (data == &sd->sd_data) {
				/*
				 * sd_link allocated by elf_newdata().
				 */
				return &sd->sd_link->sd_data;	// 这里我有个疑惑，当sd->sd_link为NULL时，sd->sd_link->sd_data肯定是非法访问
												// ------> (我们不能访问0地址处的内容)
			}									// 那么sd->sd_link->sd_data的地址是合法的？ 如果合法，那么sd->sd_link->sd_link->sd_data
		}										// 的地址合法吗？
		seterr(ERROR_SCNDATAMISMATCH);			// 2014-7-17 17:07:49 解释: 由#define offsetof(type, member) ((int)&((type*)0)->member)得知，
	} else if ((sd = scn->s_data_1)) {			// 0地址处的结构体成员，我们绝对是不能访问的，但是取地址可以，这里sd->sd_link->sd_data的地址是
		elf_assert(sd->sd_magic == DATA_MAGIC);	// 0,这也正解释了为什么Scn_Data结构体中成员sd_data必须放在首位的原因
		elf_assert(sd->sd_scn == scn);			// 而返回sd->sd_link->sd_link->sd_data的地址肯定是不行的了
		elf = scn->s_elf;						// 因为sd->sd_link->sd_link已经访问了0地址的内容了，必然导致断错误
		elf_assert(elf);
		elf_assert(elf->e_magic == ELF_MAGIC);

		if (sd->sd_freeme) {
			/* allocated by elf_newdata() */
			return &sd->sd_data;
		} else if (scn->s_type == SHT_NULL) {
			seterr(ERROR_NULLSCN);
		} else if (sd->sd_memdata) {
			/* already cooked */
			return &sd->sd_data;
		} else if (scn->s_offset < 0 || scn->s_offset > elf->e_size) {
			seterr(ERROR_OUTSIDE);
		} else if (scn->s_type == SHT_NOBITS || !scn->s_size) {
			/* no data to read */
			return &sd->sd_data;
		} else if (scn->s_offset + scn->s_size > elf->e_size) {
			seterr(ERROR_TRUNC_SCN);
		} else if (valid_class(elf->e_class)) {
			return _elf_cook_scn(elf, scn, sd);
		} else {
			seterr(ERROR_UNKNOWN_CLASS);
		}
	}

	return NULL;
}
