/*
 *  Copyright (c) 2013, Adrian Moser
 *  All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  * Neither the name of the author nor the
 *  names of its contributors may be used to endorse or promote products
 *  derived from this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ELF_UTIL_H_
#define _ELF_UTIL_H_

#include <elf.h>

#define EELFOPEN -1
#define EELFFMT  -2

typedef struct {
   int class;
   int fd; 
   size_t size;
   union {
      Elf32_Ehdr *hdr32;
      Elf64_Ehdr *hdr64;
   } ehdr;
   char * sh_names;
   char *buf;
} Elf;

typedef struct {
   union {
      Elf32_Shdr *hdr32;
      Elf64_Shdr *hdr64;
   } shdr;
   char *buf;
   size_t size;
} Elf_Scn;

int
elf_open(Elf *elf, char *file);

#define elf_get_scn(elf, scn, name)   \
   ((elf)->class == ELFCLASS32 ?      \
    elf_get_scn32((elf), scn, name) : \
    elf_get_scn64((elf), scn, name))

int
elf_get_scn32(Elf *elf, Elf_Scn *scn, char *name);

int
elf_get_scn64(Elf *elf, Elf_Scn *scn, char *name);

#endif // _ELF_UTIL_H_

