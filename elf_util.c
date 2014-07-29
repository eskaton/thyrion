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

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "elf_util.h"

int
elf_get_scn32(Elf *elf, Elf_Scn *scn, char *name) {
   Elf32_Shdr *shdr = (Elf32_Shdr *)(elf->buf + elf->ehdr.hdr32->e_shoff);
   int i = 0;

   do {
      shdr++; 

      if (!strcmp(elf->sh_names + shdr->sh_name, name)) {
         scn->shdr.hdr32 = shdr; 
         scn->buf = elf->buf + shdr->sh_offset;
         scn->size = shdr->sh_size;
         return 0;
      }
   } while (++i < elf->ehdr.hdr32->e_shnum);

   return -1;
}

int
elf_get_scn64(Elf *elf, Elf_Scn *scn, char *name) {
   Elf64_Shdr *shdr = (Elf64_Shdr *)(elf->buf + elf->ehdr.hdr64->e_shoff);
   int i = 0;

   do {
      shdr++; 

      if (!strcmp(elf->sh_names + shdr->sh_name, name)) {
         scn->shdr.hdr64 = shdr; 
         scn->buf = elf->buf + shdr->sh_offset;
         scn->size = shdr->sh_size;
         return 0;
      }
   } while (++i < elf->ehdr.hdr64->e_shnum);

   return -1;
}

int
elf_open(Elf *elf, char *file) {
   struct stat sb;

   if ((elf->fd = open(file, O_RDONLY)) < 0) {
      return EELFOPEN;
   } 

   if (stat(file, &sb)) {
      perror("stat"); 
      return EELFOPEN;
   }

   elf->size = sb.st_size;

   if ((elf->buf = mmap(NULL, elf->size, PROT_READ, MAP_PRIVATE, elf->fd, 0)) ==
         MAP_FAILED) {
      return EELFOPEN;
   }

   if (elf->buf[EI_MAG0] != 0x7f || elf->buf[EI_MAG1] != 'E' ||
       elf->buf[EI_MAG2] != 'L' || elf->buf[EI_MAG3] != 'F') {
      return EELFFMT; 
   }

   if (elf->buf[EI_CLASS] == ELFCLASS32) {
      elf->class = ELFCLASS32;
      Elf32_Ehdr *ehdr = elf->ehdr.hdr32 = (Elf32_Ehdr *)elf->buf;

      if (!ehdr->e_shoff || ehdr->e_shoff >= elf->size - ehdr->e_shentsize) {
         return EELFFMT; 
      }

      elf->sh_names = elf->buf + ((Elf32_Shdr *)(elf->buf + ehdr->e_shoff + 
            ehdr->e_shstrndx * ehdr->e_shentsize))->sh_offset;
   } else if (elf->buf[EI_CLASS] == ELFCLASS64) {
      elf->class = ELFCLASS64;
      Elf64_Ehdr *ehdr = elf->ehdr.hdr64 = (Elf64_Ehdr *)elf->buf;

      if (!ehdr->e_shoff || ehdr->e_shoff >= elf->size - ehdr->e_shentsize) {
         return EELFFMT; 
      }

      elf->sh_names = elf->buf + ((Elf64_Shdr *)(elf->buf + ehdr->e_shoff + 
            ehdr->e_shstrndx * ehdr->e_shentsize))->sh_offset;
   } else {
      return EELFFMT; 
   }

   return 0;
}

