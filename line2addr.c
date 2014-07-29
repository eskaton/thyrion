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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "thyrion.h"

int
main(int argc, char **argv) {
   Dwarf dwarf;
   dwarf_sprog *sprog;
   dwarf_sprog_file *file;
   dwarf_sm_regs *sm_regs;
   uint32_t file_idx;
   uint32_t line;
   char *colon;
   char *file_name;

   if (argc != 3) {
      fprintf(stderr, "usage: %s <file>:<line> <executable>\n", argv[0]); 
      return -1;
   }   

   if ((colon = strstr(argv[1], ":"))) {
      *colon = '\0';
      file_name = argv[1]; 
      line = atoi(colon+1);
   } else {
      fprintf(stderr, "Invalid line number\n"); 
      return -1;
   }

   if (dwarf_open(&dwarf, argv[2])) {
      fprintf(stderr, "Failed to read DWARF\n"); 
      return -1;
   }

   for (sprog = dwarf.sprog; sprog != NULL; sprog = sprog->next) {
      for (file = sprog->prologue->files, file_idx = 1; file != NULL; 
            file = file->next, file_idx++) {
         if (!strcmp(file_name, file->name)) {
            for (sm_regs = sprog->sm_regs; sm_regs != NULL; 
                  sm_regs = sm_regs->next) {
               if (sm_regs->file == file_idx && sm_regs->line == line) {
                  printf("0x%08lx\n", sm_regs->address);
                  return 0;
               }
            }

            printf("Address not found\n");
            return -1;
         }
      }
   }

   dwarf_free(&dwarf);

   return 0;
}

