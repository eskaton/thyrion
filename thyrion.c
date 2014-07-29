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

#define _GNU_SOURCE
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stack.h>
#include <hex_dump.h>

#include "thyrion.h"

static const dwarf_tag const dwarf_tags[] = {
   MK_TAG(DW_TAG_array_type), MK_TAG(DW_TAG_class_type),
   MK_TAG(DW_TAG_entry_point), MK_TAG(DW_TAG_enumeration_type),
   MK_TAG(DW_TAG_formal_parameter), MK_TAG(DW_TAG_imported_declaration),
   MK_TAG(DW_TAG_label), MK_TAG(DW_TAG_lexical_block),
   MK_TAG(DW_TAG_member), MK_TAG(DW_TAG_pointer_type),
   MK_TAG(DW_TAG_reference_type), MK_TAG(DW_TAG_compile_unit),
   MK_TAG(DW_TAG_string_type), MK_TAG(DW_TAG_structure_type),
   MK_TAG(DW_TAG_subroutine_type), MK_TAG(DW_TAG_typedef),
   MK_TAG(DW_TAG_union_type), MK_TAG(DW_TAG_unspecified_parameters),
   MK_TAG(DW_TAG_variant), MK_TAG(DW_TAG_common_block),
   MK_TAG(DW_TAG_common_inclusion), MK_TAG(DW_TAG_inheritance),
   MK_TAG(DW_TAG_inlined_subroutine), MK_TAG(DW_TAG_module),
   MK_TAG(DW_TAG_ptr_to_member_type), MK_TAG(DW_TAG_set_type),
   MK_TAG(DW_TAG_subrange_type), MK_TAG(DW_TAG_with_stmt),
   MK_TAG(DW_TAG_access_declaration), MK_TAG(DW_TAG_base_type),
   MK_TAG(DW_TAG_catch_block), MK_TAG(DW_TAG_const_type),
   MK_TAG(DW_TAG_constant), MK_TAG(DW_TAG_enumerator),
   MK_TAG(DW_TAG_file_type), MK_TAG(DW_TAG_friend),
   MK_TAG(DW_TAG_namelist), MK_TAG(DW_TAG_namelist_item),
   MK_TAG(DW_TAG_packed_type), MK_TAG(DW_TAG_subprogram),
   MK_TAG(DW_TAG_template_type_param), MK_TAG(DW_TAG_template_value_param),
   MK_TAG(DW_TAG_thrown_type), MK_TAG(DW_TAG_try_block),
   MK_TAG(DW_TAG_variant_part), MK_TAG(DW_TAG_variable),
   MK_TAG(DW_TAG_volatile_type), MK_TAG(DW_TAG_lo_user),
   MK_TAG(DW_TAG_hi_user), MK_TAG(0)
};

static const dwarf_att const dwarf_atts[] = {
   MK_ATT(DW_AT_sibling), MK_ATT(DW_AT_location),
   MK_ATT(DW_AT_name), MK_ATT(DW_AT_ordering),
   MK_ATT(DW_AT_byte_size), MK_ATT(DW_AT_bit_offset),
   MK_ATT(DW_AT_bit_size), MK_ATT(DW_AT_stmt_list),
   MK_ATT(DW_AT_low_pc), MK_ATT(DW_AT_high_pc),
   MK_ATT(DW_AT_language), MK_ATT(DW_AT_discr),
   MK_ATT(DW_AT_discr_value), MK_ATT(DW_AT_visibility),
   MK_ATT(DW_AT_import), MK_ATT(DW_AT_string_length),
   MK_ATT(DW_AT_common_reference), MK_ATT(DW_AT_comp_dir),
   MK_ATT(DW_AT_const_value), MK_ATT(DW_AT_containing_type),
   MK_ATT(DW_AT_default_value), MK_ATT(DW_AT_inline),
   MK_ATT(DW_AT_is_optional), MK_ATT(DW_AT_lower_bound),
   MK_ATT(DW_AT_producer), MK_ATT(DW_AT_prototyped),
   MK_ATT(DW_AT_return_addr), MK_ATT(DW_AT_start_scope),
   MK_ATT(DW_AT_stride_size), MK_ATT(DW_AT_upper_bound),
   MK_ATT(DW_AT_abstract_origin), MK_ATT(DW_AT_accessibility),
   MK_ATT(DW_AT_address_class), MK_ATT(DW_AT_artificial),
   MK_ATT(DW_AT_base_types), MK_ATT(DW_AT_calling_convention),
   MK_ATT(DW_AT_count), MK_ATT(DW_AT_data_member_location),
   MK_ATT(DW_AT_decl_column), MK_ATT(DW_AT_decl_file),
   MK_ATT(DW_AT_decl_line), MK_ATT(DW_AT_declaration),
   MK_ATT(DW_AT_discr_list), MK_ATT(DW_AT_encoding),
   MK_ATT(DW_AT_external), MK_ATT(DW_AT_frame_base),
   MK_ATT(DW_AT_friend), MK_ATT(DW_AT_identifier_case),
   MK_ATT(DW_AT_macro_info), MK_ATT(DW_AT_namelist_item),
   MK_ATT(DW_AT_priority), MK_ATT(DW_AT_segment),
   MK_ATT(DW_AT_specification), MK_ATT(DW_AT_static_link),
   MK_ATT(DW_AT_type), MK_ATT(DW_AT_use_location),
   MK_ATT(DW_AT_variable_parameter), MK_ATT(DW_AT_virtuality),
   MK_ATT(DW_AT_vtable_elem_location), MK_ATT(DW_AT_allocated), 
   MK_ATT(DW_AT_associated), MK_ATT(DW_AT_data_location), 
   MK_ATT(DW_AT_byte_stride), MK_ATT(DW_AT_entry_pc),
   MK_ATT(DW_AT_use_UTF8), MK_ATT(DW_AT_extension),
   MK_ATT(DW_AT_ranges), MK_ATT(DW_AT_trampoline),
   MK_ATT(DW_AT_call_column), MK_ATT(DW_AT_call_file),
   MK_ATT(DW_AT_call_line), MK_ATT(DW_AT_description),
   MK_ATT(DW_AT_binary_scale), MK_ATT(DW_AT_decimal_scale),
   MK_ATT(DW_AT_small), MK_ATT(DW_AT_decimal_sign),
   MK_ATT(DW_AT_digit_count), MK_ATT(DW_AT_picture_string),
   MK_ATT(DW_AT_mutable), MK_ATT(DW_AT_threads_scaled),
   MK_ATT(DW_AT_explicit), MK_ATT(DW_AT_object_pointer),
   MK_ATT(DW_AT_endianity), MK_ATT(DW_AT_elemental),
   MK_ATT(DW_AT_pure), MK_ATT(DW_AT_recursive),
   MK_ATT(DW_AT_signature), MK_ATT(DW_AT_main_subprogramm),
   MK_ATT(DW_AT_data_bit_offset), MK_ATT(DW_AT_const_expr),
   MK_ATT(DW_AT_enum_class), MK_ATT(DW_AT_linkage_name),
   MK_ATT(0)
};

static const dwarf_form const dwarf_forms[] = {
   MK_FORM(DW_FORM_addr), MK_FORM(DW_FORM_block2),
   MK_FORM(DW_FORM_block4), MK_FORM(DW_FORM_data2),
   MK_FORM(DW_FORM_data4), MK_FORM(DW_FORM_data8),
   MK_FORM(DW_FORM_string), MK_FORM(DW_FORM_block),
   MK_FORM(DW_FORM_block1), MK_FORM(DW_FORM_data1),
   MK_FORM(DW_FORM_flag), MK_FORM(DW_FORM_sdata),
   MK_FORM(DW_FORM_strp), MK_FORM(DW_FORM_udata),
   MK_FORM(DW_FORM_ref_addr), MK_FORM(DW_FORM_ref1),
   MK_FORM(DW_FORM_ref2), MK_FORM(DW_FORM_ref4),
   MK_FORM(DW_FORM_ref8), MK_FORM(DW_FORM_ref_udata),
   MK_FORM(DW_FORM_indirect), MK_FORM(0)
};

static inline void
fail(Dwarf *dwarf, const char *fmt, ...) {
   va_list args;

   if (dwarf->error) {
      free(dwarf->error); 
   }

   va_start(args, fmt);
   asprintf(&dwarf->error, fmt, args);
   va_end(args);

   longjmp(dwarf->env, 1);
}

static int
decode_uleb128(char * const buf, uint32_t *res) {
   char *pos = buf;
   uint32_t shift = 0;
   int consumed = 0;
   uint32_t byte;
   *res = 0;

   while (true) {
      byte = *pos++;
      consumed++;
      *res |= ((byte & 0x7F) << shift);
      if ((byte & 0x80) == 0) {
         break;
      }
      shift += 7;
   }

   return consumed;
}

static int
decode_sleb128(char * const buf, int32_t *res) {
   char *pos = buf;
   uint32_t shift = 0;
   int consumed = 0;
   uint8_t size = 8 * sizeof(int32_t);
   uint8_t byte;
   *res = 0;

   while (true) {
      byte = *pos++;
      consumed++;
      *res |= ((byte & 0x7F) << shift);
      shift += 7;
      if ((byte & 0x80) == 0) {
         break;
      }
   }

   if (shift < size && byte & 0x40) {
      *res |= -(1<<shift); 
   }

   return consumed;
}

static const dwarf_tag *
get_tag(dwarf_tag_id tag) {
   int i;

   for (i = 0; dwarf_tags[i].id != 0 && dwarf_tags[i].id < tag; i++); 

   if (dwarf_tags[i].id == tag) {
      return &dwarf_tags[i];
   } 

   return NULL;
}

static const dwarf_att *
get_att(dwarf_att_id att) {
   int i;

   for (i = 0; dwarf_atts[i].id != 0 && dwarf_atts[i].id < att; i++); 

   if (dwarf_atts[i].id == att) {
      return &dwarf_atts[i];
   } 

   return NULL;
}

static const dwarf_form *
get_form(dwarf_form_id form) {
   int i;

   for (i = 0; dwarf_forms[i].id != 0 && dwarf_forms[i].id < form; i++); 

   if (dwarf_forms[i].id == form) {
      return &dwarf_forms[i];
   } 

   return NULL;
}

static dwarf_abbrevs *
dwarf_read_abbrev(char *buf, long buf_len) {
   uint32_t uleb128_tmp;
   dwarf_abbrevs *abbrev = NULL;
   dwarf_abbrevs **cur_abbrev = &abbrev;
   dwarf_abbrev_tab **cur_tab = NULL;
   dwarf_att_id att_id;
   dwarf_form_id form_id;
   dwarf_att_spec **atts;
   char *buf_start = buf;
   char *buf_end = buf_start + buf_len;

   while (buf < buf_end) {
      if (!*cur_abbrev) {
         *cur_abbrev = (dwarf_abbrevs *)calloc(1, sizeof(dwarf_abbrevs));
         (*cur_abbrev)->offset = buf - buf_start;
         cur_tab = &(*cur_abbrev)->tab;
      }

      buf += decode_uleb128(buf, &uleb128_tmp);

      if (uleb128_tmp == 0) {
         cur_abbrev = &(*cur_abbrev)->next;
         continue;
      } 

      *cur_tab = (dwarf_abbrev_tab *)calloc(1, sizeof(dwarf_abbrev_tab));
      atts = &((*cur_tab)->atts);

      (*cur_tab)->id = uleb128_tmp;
      buf += decode_uleb128(buf, &uleb128_tmp);
      (*cur_tab)->tag = get_tag(uleb128_tmp); 
      (*cur_tab)->has_children = (uint8_t)*buf++;

      /* read attributes */
      while (true) {
         buf += decode_uleb128(buf, &uleb128_tmp);
         att_id = uleb128_tmp;
         buf += decode_uleb128(buf, &uleb128_tmp);
         form_id = uleb128_tmp;

         if (att_id == 0 && form_id == 0) {
            break;
         }

         *atts = calloc(1, sizeof(dwarf_att_spec));
         (*atts)->att = get_att(att_id);
         (*atts)->form = get_form(form_id);
         atts = &(*atts)->next;
      }

      cur_tab = &(*cur_tab)->next;
   }

   return abbrev;
}

static dwarf_abbrevs *
dwarf_get_abbrevs(dwarf_abbrevs *abbrevs, uint32_t offset) {
   while (abbrevs) {
      if(abbrevs->offset == offset) {
         return abbrevs;
      }
      abbrevs = abbrevs->next; 
   }
   return NULL;
}

static dwarf_abbrev_tab *
dwarf_get_abbrev_tab(dwarf_abbrev_tab *atab, uint32_t abbrev_code) {
   while (atab) {
      if (atab->id == abbrev_code) {
         return atab; 
      }
      atab = atab->next;
   }

   return NULL;
}

static dwarf_block *
dwarf_read_block(char **buf, long size_len, long buf_len) {
   dwarf_block *block = calloc(1, sizeof(dwarf_block));
   block->len = buf_len;
   block->buf = (*buf) += size_len;
   (*buf) += buf_len;
   return block;
}

static dwarf_die_att *
dwarf_read_die_att(Dwarf *dwarf, char **buf, dwarf_att_spec *att_spec, 
      dwarf_cu_header *cu_hdr) {
   uint32_t size_len;
   uint32_t buf_len;
   dwarf_die_att *die_att = calloc(1, sizeof(dwarf_die_att));
   die_att->att_spec = att_spec;

   switch (att_spec->form->id) {
      case DW_FORM_string:
         die_att->value.s_val = *buf;
         while (*(*buf)++ != '\0'); 
         break;
      case DW_FORM_strp: // fall through
      case DW_FORM_ref_addr: // fall through
         die_att->value.ul_val = *(uint32_t *)(*buf);
         (*buf) += sizeof(uint32_t);
         break;
      case DW_FORM_addr: 
         memcpy(&die_att->value.ul_val, *buf, cu_hdr->addr_size);
         (*buf) += cu_hdr->addr_size;
         break;
      case DW_FORM_block: 
         size_len = decode_uleb128(*buf, &buf_len);
         die_att->value.b_val = dwarf_read_block(buf, size_len, buf_len);
         break;
      case DW_FORM_block1: 
         die_att->value.b_val = dwarf_read_block(buf, 1, (*(uint8_t *)(*buf)));
         break;
      case DW_FORM_block2: 
         die_att->value.b_val = dwarf_read_block(buf, 2, (*(uint16_t *)(*buf)));
         break;
      case DW_FORM_block4: 
         die_att->value.b_val = dwarf_read_block(buf, 4, (*(uint32_t *)(*buf)));
         break;
      case DW_FORM_ref1: // fall through
      case DW_FORM_data1: 
         die_att->value.ul_val = *(uint8_t *)(*buf);
         (*buf) += 1;
         break;
      case DW_FORM_ref2: // fall through
      case DW_FORM_data2: 
         die_att->value.ul_val = *(uint16_t *)(*buf);
         (*buf) += 2;
         break;
      case DW_FORM_ref4: // fall through
      case DW_FORM_data4: 
         die_att->value.ul_val = *(uint32_t *)(*buf);
         (*buf) += 4;
         break;
      case DW_FORM_ref8: // fall through
      case DW_FORM_data8: 
         die_att->value.ul_val = *(uint64_t *)(*buf);
         (*buf) += 8;
         break;
      case DW_FORM_sdata: 
         (*buf) += decode_sleb128(*buf, &die_att->value.sl_val);
         break;
      case DW_FORM_ref_udata: // fall through
      case DW_FORM_udata: 
         (*buf) += decode_uleb128(*buf, &die_att->value.ul_val);
         break;
      case DW_FORM_flag: 
         die_att->value.ul_val = *(uint8_t *)(*buf);
         (*buf) += 1;
         break;
      case DW_FORM_indirect: // fall through
      default:
         fail(dwarf, "Unsupported form in DIE attribute: %s\n", 
               att_spec->form->name);
   }

   return die_att;
}

static dwarf_die *
dwarf_read_die(Dwarf *dwarf, char **buf, dwarf_abbrev_tab *die_abbrevs, 
      dwarf_cu_header *cu_hdr) {
   dwarf_att_spec *att_spec = die_abbrevs->atts;
   dwarf_die *die = calloc(1, sizeof(dwarf_die));
   dwarf_die_att **att = &die->att;
   
   die->tag = die_abbrevs->tag;

   while (att_spec) {
      *att = dwarf_read_die_att(dwarf, buf, att_spec, cu_hdr);
      att = &(*att)->next_att;
      att_spec = att_spec->next; 
   }

   return die; 
}

static dwarf_die *
dwarf_read_cu_body(Dwarf *dwarf, char **buf, uint32_t len, 
      dwarf_abbrev_tab *atab, dwarf_cu_header *cu_hdr) {
   dwarf_die *first_die;
   dwarf_die **die = &first_die;
   uint32_t abbrev_code;
   dwarf_abbrev_tab *die_abbrevs;
   stack *path = stack_create();
   long *body_end = (long *)(*buf + len);

   while ((long *)*buf < body_end) {
      *buf += decode_uleb128(*buf, &abbrev_code);

      if (abbrev_code) {
         die_abbrevs = dwarf_get_abbrev_tab(atab, abbrev_code);

         if (!die_abbrevs) {
            fail(dwarf, "Abbreviation table for id %d missing\n", 
                  abbrev_code); 
         }

         *die = dwarf_read_die(dwarf, buf, die_abbrevs, cu_hdr);

         if (die_abbrevs->has_children == yes) {
            stack_push(path, die);
            die = &(*die)->child;
         } else {
            die = &(*die)->sibling;
         }
      } else {
         if (!stack_is_empty(path)) {
            // TODO: what about this die?
            die = (dwarf_die* *)stack_pop(path); 
         }
      }
   }

   stack_destroy(path);

   return first_die;
}

static dwarf_cu *
dwarf_read_cu(Dwarf *dwarf, char *buf, uint32_t len) {
   char *buf_end = buf + len;
   dwarf_cu *first_cu; 
   dwarf_cu **cu = &first_cu;
   uint32_t body_len;
   dwarf_abbrevs *cu_abbrevs;

   while (buf < buf_end) {
      *cu = (dwarf_cu *)calloc(1, sizeof(dwarf_cu));
      memcpy(&(*cu)->hdr, buf, sizeof(dwarf_cu_header));
      buf += sizeof(dwarf_cu_header);
      body_len = (*cu)->hdr.length - sizeof((*cu)->hdr) + 
         sizeof((*cu)->hdr.length);
      cu_abbrevs = dwarf_get_abbrevs(dwarf->abbrevs, (*cu)->hdr.abbrev_off);

      if (!cu_abbrevs) {
         fail(dwarf, "Abbreviation table at offset %d missing\n", 
               (*cu)->hdr.abbrev_off); 
      }

      (*cu)->die = dwarf_read_cu_body(dwarf, &buf, body_len, cu_abbrevs->tab, 
            &(*cu)->hdr);

      cu = &(*cu)->next_cu;
   }

   return first_cu;
}

static void
skip_string(char **buf) {
   while (*(*buf)++ != '\0'); 
}

static dwarf_sprog_dir *
dwarf_read_pro_incl_dirs(char **buf) {
   dwarf_sprog_dir *first_dir = NULL;
   dwarf_sprog_dir **cur_dir = &first_dir;

   while (**buf) {
      *cur_dir = (dwarf_sprog_dir *)calloc(1, sizeof(dwarf_sprog_dir));
      (*cur_dir)->name = *buf;
      skip_string(buf);
      cur_dir = &(*cur_dir)->next;
   }

   (*buf)++;

   return first_dir;
}

static dwarf_sprog_file *
dwarf_read_file(char **buf) {
   dwarf_sprog_file *file = calloc(1, sizeof(dwarf_sprog_file));
   file->name = *buf;
   skip_string(buf);
   *buf += decode_uleb128(*buf, &file->dir_idx);
   *buf += decode_uleb128(*buf, &file->mtime);
   *buf += decode_uleb128(*buf, &file->size);
   return file;
}

static dwarf_sprog_file *
dwarf_read_pro_files(char **buf) {
   dwarf_sprog_file *first_file = NULL;
   dwarf_sprog_file **cur_file = &first_file;
//TODO: file struct correct?
   while (**buf) {
      *cur_file = dwarf_read_file(buf);
      cur_file = &(*cur_file)->next;
   }

   (*buf)++;

   return first_file;
}

static void 
dwarf_read_sprog_prologue(Dwarf *dwarf, char **buf, 
      dwarf_sprog_pro **prologue_hdl) {
   dwarf_sprog_pro *prologue;
   char *prologue_end; 
   int i;

   *prologue_hdl = calloc(1, sizeof(dwarf_sprog_pro));
   prologue = *prologue_hdl;

   prologue->total_len = *(uint32_t *)*buf;
   (*buf) += sizeof(uint32_t);
   prologue->version = *(uint16_t *)*buf;
   (*buf) += sizeof(uint16_t);
   prologue->prologue_len = *(uint32_t *)*buf;
   (*buf) += sizeof(uint32_t);
   prologue_end = *buf + prologue->prologue_len;
   prologue->min_inst_len = *(uint8_t *)*buf;
   (*buf)++;
   prologue->dflt_is_stmt = *(uint8_t *)*buf;
   (*buf)++;
   prologue->line_base = *(int8_t *)(*buf);
   (*buf)++;
   prologue->line_range = *(int8_t *)(*buf);
   (*buf)++;
   prologue->opcode_base = *(int8_t *)(*buf);
   (*buf)++;

   prologue->std_opcode_len = calloc(1, prologue->opcode_base);

   for (i=1; i<prologue->opcode_base; i++) {
      prologue->std_opcode_len[i] = (int8_t)*(*buf)++;
   }

   prologue->incl_dirs = dwarf_read_pro_incl_dirs(buf);
   prologue->files = dwarf_read_pro_files(buf);

   if (*buf != prologue_end) {
      fail(dwarf, "Invalid length of prologue in section .debug_line\n"); 
   }

}

static void
dwarf_sprog_append_file(dwarf_sprog_pro *prologue, dwarf_sprog_file *file) {
   dwarf_sprog_file *cur_file = prologue->files;
   while (cur_file) {
      cur_file = cur_file->next; 
   }
   cur_file = file;
}

static dwarf_sm_regs *
dwarf_copy_sm_reg(dwarf_sm_regs *reg) {
   dwarf_sm_regs *new_regs = calloc(1, sizeof(dwarf_sm_regs));
   memcpy(new_regs, reg, sizeof(dwarf_sm_regs));
   return new_regs;
}

static dwarf_sm_regs *
dwarf_read_sprog_sm(Dwarf *dwarf, char **buf, char **sm_hdl, size_t sm_len, 
      dwarf_sprog_pro *prologue) {
   *sm_hdl = *buf;

   dwarf_sm_regs *first_sm_regs = NULL;
   dwarf_sm_regs **cur_sm_regs = &first_sm_regs;
   uint32_t uarg1;
   int32_t arg1;
   uint32_t inst_len;

   *cur_sm_regs = (dwarf_sm_regs *)calloc(1, sizeof(dwarf_sm_regs));
   (*cur_sm_regs)->file = 1;
   (*cur_sm_regs)->line = prologue->dflt_is_stmt;
   (*cur_sm_regs)->basic_block = false;
   (*cur_sm_regs)->end_sequence = false;
   (*cur_sm_regs)->prologue_end = false;
   (*cur_sm_regs)->epilogue_begin = false;
   (*cur_sm_regs)->isa = 0;
   (*cur_sm_regs)->discriminator = 0;

   while (true) {
      uint8_t opcode = *(uint8_t *)(*buf)++;

      (*cur_sm_regs)->opcode = opcode;

      switch(opcode) {
         case 0: // extended opcode
            *buf += decode_uleb128(*buf, &inst_len); 
            (*cur_sm_regs)->ext_opcode = *(uint8_t *)(*buf);
            (*buf)++;

            switch ((*cur_sm_regs)->ext_opcode) {
               case DW_LNE_end_sequence:
                  goto end;
                  break;
               case DW_LNE_set_address:
                  (*cur_sm_regs)->address = *(uintptr_t *)(*buf); 
                  *buf += sizeof(uintptr_t);
                  break;
               case DW_LNE_define_file:
                  dwarf_sprog_append_file(prologue, dwarf_read_file(buf));
                  break;
               case DW_LNE_set_discriminator:
                  *buf += decode_uleb128(*buf, &uarg1);
                  (*cur_sm_regs)->discriminator = uarg1;
                  break;
               default:
                  fail(dwarf, "Unknown extended opcode %d\n", 
                        (*cur_sm_regs)->ext_opcode);
            }
            break;
         case DW_LNS_copy: 
            (*cur_sm_regs)->basic_block = false;
            break;
         case DW_LNS_advance_pc:
            *buf += decode_uleb128(*buf, &uarg1);
            (*cur_sm_regs)->address += uarg1 * prologue->min_inst_len;
            break;
         case DW_LNS_advance_line:
            *buf += decode_sleb128(*buf, &arg1); 
            (*cur_sm_regs)->line += arg1; 
            break;
         case DW_LNS_set_file:
            *buf += decode_uleb128(*buf, &uarg1); 
            (*cur_sm_regs)->file = uarg1;
            break;
         case DW_LNS_set_column:
            *buf += decode_uleb128(*buf, &uarg1); 
            (*cur_sm_regs)->column = uarg1;
            break;
         case DW_LNS_negate_stmt:
            (*cur_sm_regs)->is_stmt = !(*cur_sm_regs)->is_stmt;
            break;
         case DW_LNS_set_basic_block:
            (*cur_sm_regs)->basic_block = true;
            break;
         case DW_LNS_const_add_pc:
            (*cur_sm_regs)->address += (long)(255 - prologue->opcode_base) / 
               prologue->line_range;
            break;
         case DW_LNS_fixed_advance_pc:
            uarg1 = *(uint16_t *)*buf;
            *buf += 2;
            (*cur_sm_regs)->address += uarg1;
            break;
         case DW_LNS_set_prologue_end:
            (*cur_sm_regs)->prologue_end = true;
            break;
         case DW_LNS_set_epilogue_begin:
            (*cur_sm_regs)->epilogue_begin = true;
            break;
         case DW_LNS_set_isa:
            *buf += decode_uleb128(*buf, &uarg1); 
            (*cur_sm_regs)->isa = uarg1;
            break;
         default:
            if (opcode < prologue->opcode_base) {
               fail(dwarf, "Unknown opcode %d\n", opcode);
            } else {
               // special opcode 
               uint8_t adj_opcode = opcode - prologue->opcode_base;
               (*cur_sm_regs)->address += adj_opcode / prologue->line_range;
               (*cur_sm_regs)->line += prologue->line_base + 
                  (adj_opcode % prologue->line_range);
               (*cur_sm_regs)->basic_block = false;
               (*cur_sm_regs)->prologue_end = false;
               (*cur_sm_regs)->epilogue_begin = false;
               (*cur_sm_regs)->discriminator = 0;
            }
            break;
      } 

      (*cur_sm_regs)->next = dwarf_copy_sm_reg(*cur_sm_regs);
      cur_sm_regs = &(*cur_sm_regs)->next;
   }

end:
   return first_sm_regs;
}

static dwarf_sprog *
dwarf_read_sprog(Dwarf *dwarf, char *buf, uint32_t len) {
   dwarf_sprog *first_sprog = NULL;
   dwarf_sprog **cur_sprog = &first_sprog;
   char *buf_end = buf + len;

   while (buf < buf_end) {
      *cur_sprog = (dwarf_sprog *)calloc(1, sizeof(dwarf_sprog));
      dwarf_read_sprog_prologue(dwarf, &buf, &(*cur_sprog)->prologue);
      (*cur_sprog)->sm_len = (*cur_sprog)->prologue->total_len 
         + sizeof((*cur_sprog)->prologue->total_len) 
         - (*cur_sprog)->prologue->prologue_len; 
      (*cur_sprog)->sm_regs =
         dwarf_read_sprog_sm(dwarf, &buf, &(*cur_sprog)->sm, 
               (*cur_sprog)->sm_len, (*cur_sprog)->prologue);
      cur_sprog = &(*cur_sprog)->next;
   }

   return first_sprog;
}

static dwarf_str *
dwarf_read_str(char *buf, uint32_t len) {
   dwarf_str *str = calloc(1, sizeof(dwarf_str));
   str->table = buf;
   str->length = len;
   return str; 
}

static dwarf_aranges *
dwarf_read_aranges(Dwarf *dwarf, char *buf, uint32_t len) {
   dwarf_aranges *first_aranges = NULL;
   dwarf_aranges **cur_aranges = &first_aranges;
   dwarf_arange **cur_arange; 
   char *buf_end = buf + len;
   char *set_end;
   uint32_t off = 0;
   uint32_t align = 0;
   uint32_t arange_size;
   uint32_t addr_size;

   while (buf < buf_end) {
      *cur_aranges = (dwarf_aranges *)calloc(1, sizeof(dwarf_aranges));
      memcpy(&(*cur_aranges)->hdr, buf, sizeof(dwarf_ar_header));
      set_end = buf + (*cur_aranges)->hdr.length + 4 /* length field */;
      off += sizeof(dwarf_ar_header);
      buf += sizeof(dwarf_ar_header);
      addr_size = (*cur_aranges)->hdr.addr_size;
      arange_size = addr_size << 1;
      align = arange_size - (off & (arange_size - 1));
      buf += align;
      off += align;

      cur_arange = &(*cur_aranges)->arange;

      while (buf < set_end) {
         *cur_arange = calloc(1, sizeof(dwarf_arange));
         memcpy(&(*cur_arange )->address, buf, addr_size);
         memcpy(&(*cur_arange )->length, buf + addr_size, addr_size);
         buf += arange_size;
         
         if ((*cur_arange)->address == 0 && (*cur_arange)->length == 0) {
            free(*cur_arange);
            *cur_arange = NULL;
            break; 
         }

         cur_arange = &(*cur_arange)->next_ar;
      }
   }

   return first_aranges; 
}

void
dwarf_abbrev_dump_atts(dwarf_att_spec *spec) {
   bool first = true;

   while (spec) {
      if (first) {
         printf("%-30s: 0x%02x %-20s 0x%02x %-20s\n", "attributes", 
               spec->att->id, spec->att->name, spec->form->id, spec->form->name);
         first = false;
      } else {
         printf("%-30s: 0x%02x %-20s 0x%02x %-20s\n", "", 
               spec->att->id, spec->att->name, spec->form->id, spec->form->name);
      }
      spec = spec->next; 
   }
}

static void
dwarf_abbrev_dump(Dwarf *dwarf) {
   dwarf_abbrevs *abbrevs = dwarf->abbrevs;
   dwarf_abbrev_tab *tab;

   printf("Section: .debug_abbrev\n");

   while (abbrevs) {
      printf("%-30s: 0x%08x\n", "offset", abbrevs->offset);
      tab = abbrevs->tab;
      while (tab) {
         printf("%-30s: %d\n", "id", tab->id);
         printf("%-30s: 0x%02x %s\n", "tag", tab->tag->id, tab->tag->name);
         printf("%-30s: %s\n", "children?", 
               tab->has_children == yes ? "yes" : "no");
         dwarf_abbrev_dump_atts(tab->atts);
         printf("\n");
         tab = tab->next; 
      }
      abbrevs = abbrevs->next;
   }
}

static void
dwarf_value_dump(Dwarf *dwarf, const dwarf_form *form, dwarf_value value) {
   switch (form->id) {
      case DW_FORM_string:
         printf("%s", value.s_val);
         break;
      case DW_FORM_strp:
         printf("%s [0x%08x]", dwarf->str->table + value.ul_val, value.ul_val);
         break;
      case DW_FORM_ref_addr: // fall through
      case DW_FORM_addr: 
         printf("0x%08x", value.ul_val);
         break;
      case DW_FORM_block:  // fall through
      case DW_FORM_block1: // fall through
      case DW_FORM_block2: // fall through
      case DW_FORM_block4: 
         printf("%0d bytes of binary data", value.b_val->len);
         break;
      case DW_FORM_data1: // fall through
      case DW_FORM_data2: // fall through
      case DW_FORM_data4: // fall through
      case DW_FORM_data8: // fall through
      case DW_FORM_udata: 
         printf("%d", value.ul_val);
         break;
      case DW_FORM_sdata: 
         printf("%d", value.sl_val);
         break;
      case DW_FORM_flag: 
         printf("%d", (uint8_t)value.ul_val);
         break;
      case DW_FORM_ref1: // fall through
      case DW_FORM_ref2: // fall through
      case DW_FORM_ref4: // fall through
      case DW_FORM_ref8: // fall through
      case DW_FORM_ref_udata: 
         printf("%x", value.ul_val);
         break;
      case DW_FORM_indirect: 
      default:
         fprintf(stderr, "Unsupported form in DIE attribute: %s\n", form->name);
   }

}

static void
dwarf_die_dump(Dwarf *dwarf, dwarf_die *die, char *prefix) {
   dwarf_die_att *att = die->att;

   printf("%s%-30s\n", prefix, die->tag->name);

   while (att) {
      printf("%s%-30s: ", prefix, att->att_spec->att->name);
      dwarf_value_dump(dwarf, att->att_spec->form, att->value);
      printf(" (%s)\n", att->att_spec->form->name);
      att = att->next_att;
   }

   printf("\n");
}

static void
dwarf_info_dump(Dwarf *dwarf) {
   dwarf_cu *cu = dwarf->cu;
   stack *path; 
   char *prefix = "                                                                                "; 
   size_t prefix_len = strlen(prefix);
   dwarf_die *die;
   int level = 0;

   printf("Section: .debug_info\n");

   while (cu) {
      path = stack_create();

      printf("%-30s: 0x%08x\n", "length", cu->hdr.length);
      printf("%-30s: 0x%04x\n", "version", cu->hdr.version);
      printf("%-30s: 0x%08x\n", "abbrev_offset", cu->hdr.abbrev_off);
      printf("%-30s: 0x%02x\n", "addr_size", cu->hdr.addr_size);
      printf("\n");

      die = cu->die;

      while (die) {
         dwarf_die_dump(dwarf, die, prefix+prefix_len-level);
         if (die->child) {
            stack_push(path, die);
            level++;
            die = die->child;
         } else if (die->sibling) {
            die = die->sibling;
         } else if (!stack_is_empty(path)) {
            die = stack_pop(path); 
            level--;
            die = die->sibling;
         }
      }

      stack_destroy(path);
      cu = cu->next_cu; 
   }
}

static char *
dwarf_get_dir(dwarf_sprog_pro *prologue, uint32_t dir_idx) {
   dwarf_sprog_dir *cur_dir = prologue->incl_dirs;
   uint32_t i = 0;

   if (dir_idx == 0) {
      return ".";
   }

   dir_idx--;

   while (cur_dir) {
      if (i == dir_idx) {
         return cur_dir->name; 
      }
      cur_dir = cur_dir->next;
      i++;
   }

   return NULL;
}

static void
dwarf_sprog_pro_incl_dirs_dump(dwarf_sprog_dir *dir) {
   while(dir) {
      printf("%-30s: %s\n", "include_directories", dir->name); 
      dir = dir->next; 
   }
}

static void
dwarf_sprog_pro_files_dump(dwarf_sprog_pro *prologue) {
   dwarf_sprog_file *file = prologue->files;

   while(file) {
      char *dir = dwarf_get_dir(prologue, file->dir_idx);
      printf("%-30s: %s/%s (%d,%d,%d)\n", "file_names", dir, file->name, 
            file->dir_idx, file->mtime, file->size); 
      file = file->next; 
   }
}

static void
dwarf_sprog_pro_dump(dwarf_sprog_pro *prologue) {
   printf("%-30s: 0x%08x\n", "total_length", prologue->total_len);
   printf("%-30s: 0x%04x\n", "version", prologue->version);
   printf("%-30s: 0x%08x\n", "prologue_length", prologue->prologue_len);
   printf("%-30s: 0x%02x\n", "minimum_instruction_length", 
         prologue->min_inst_len);
   printf("%-30s: 0x%02x\n", "default_is_stmt", prologue->dflt_is_stmt);
   printf("%-30s: 0x%02x (%d)\n", "line_base", prologue->line_base, 
         prologue->line_base);
   printf("%-30s: 0x%02x\n", "line_range", prologue->line_range);
   printf("%-30s: 0x%02x (%d)\n", "opcode_base", prologue->opcode_base, 
         prologue->opcode_base);

   int i;
   char *opcode_lengths = calloc(1, prologue->opcode_base * 3 - 1);
   char *opcode_lengths_pos = opcode_lengths;

   for (i = 1; i < prologue->opcode_base; i++) {
      *opcode_lengths_pos++ = '0' + (int8_t)prologue->std_opcode_len[i - 1];
      if (i != prologue->opcode_base - 1) {
         *opcode_lengths_pos++ = ',';
         *opcode_lengths_pos++ = ' ';
      }
   } 

   opcode_lengths_pos = '\0';
   printf("%-30s: %s\n", "standard_opcode_length", opcode_lengths);
   free(opcode_lengths);

   dwarf_sprog_pro_incl_dirs_dump(prologue->incl_dirs);
   dwarf_sprog_pro_files_dump(prologue);
}

static char *ext_opcode_names[] = {
   NULL,
   "DW_LNE_end_sequence",
   "DW_LNE_set_address",
   "DW_LNE_define_file",
   "DW_LNE_set_discriminator"
};

// TODO: add missing opcodes
static char *opcode_names[] = {
   NULL,
   "DW_LNS_copy",
   "DW_LNS_advance_pc",
   "DW_LNS_advance_line",
   "DW_LNS_set_file",
   "DW_LNS_set_column",
   "DW_LNS_negate_stmt",
   "DW_LNS_set_basic_block",
   "DW_LNS_const_add_pc",
   "DW_LNS_fixed_advance_pc",
   "DW_LNS_set_prologue_end",
   "DW_LNS_set_epilogue_begin",
   "DW_LNS_set_isa"
};

static void
dwarf_sprog_sm_regs_dump(dwarf_sprog_pro *prologue, dwarf_sm_regs *reg) {
   char *op = NULL;
   char *sop = NULL;

   if (reg->opcode == 0) {
      // extended opcode
      op = ext_opcode_names[reg->ext_opcode];
   } else if (reg->opcode <= DW_LNS_set_isa) {
      op = opcode_names[reg->opcode];
   } else if (reg->opcode >= prologue->opcode_base) {
      sop = malloc(20);
      sprintf(sop, "special op %d", reg->opcode - prologue->opcode_base); 
   } else {
      op = "unknown"; 
   }

   if (op) {
      printf("%-25s 0x%08lx %6d %5d %4d\n", op, reg->address, reg->line, 
            reg->column, reg->file);
   } else {
      printf("%-25s 0x%08lx %6d %5d %4d\n", sop, reg->address, reg->line, 
            reg->column, reg->file);
      free(sop);
   }
}

static void
dwarf_sprog_sm_dump(dwarf_sprog_pro *prologue, dwarf_sm_regs *regs) {
   printf("%-30s:\n", "state_machine:");

   printf("%-25s %10s %6s %5s %4s\n", "operation", "address", "line", "col", 
         "file");

   while (regs) {
      dwarf_sprog_sm_regs_dump(prologue, regs);
      regs = regs->next;  
   }
   
   printf("\n");
}

static void 
dwarf_line_dump(Dwarf *dwarf) {
   dwarf_sprog *sprog = dwarf->sprog;

   printf("Section: .debug_line\n");

   while (sprog) {
      dwarf_sprog_pro_dump(sprog->prologue); 
      dwarf_sprog_sm_dump(sprog->prologue, sprog->sm_regs);
      sprog = sprog->next;
   }
}

static void
dwarf_str_dump(Dwarf *dwarf) {
   dwarf_str *str = dwarf->str;
   
   if (str) {
      printf("Section: .debug_str\n");
      hex_dump(str->table, str->length, 0);
   }
}

static void 
dwarf_arange_dump(dwarf_aranges *aranges) {
   dwarf_arange *arange = aranges->arange;

   if (aranges->hdr.addr_size == 4) {
      printf("\nAddress    Size\n");
   } else {
      printf("\nAddress            Size\n");
   }

   while (arange) {
      if (aranges->hdr.addr_size == 4) {
         printf("0x%08x %d\n", (uint32_t)arange->address, 
               (uint32_t)arange->length);
         arange = arange->next_ar;
      } else {
         printf("0x%016" PRIx64 " %" PRIx64 "\n", arange->address, 
               arange->length);
         arange = arange->next_ar;
      }
   }
}

static void
dwarf_ar_header_dump(dwarf_ar_header *hdr) {
   printf("Length:                  %d\n", hdr->length);
   printf("Version:                 %d\n", hdr->version);
   printf("Offset into .debug_info: %d\n", hdr->info_off);
   printf("Address size:            %d\n", hdr->addr_size);
   printf("Segment size:            %d\n", hdr->seg_size);
}

static void 
dwarf_aranges_dump(Dwarf *dwarf) {
   dwarf_aranges *aranges = dwarf->aranges;

   printf("Section: .debug_aranges\n");

   while (aranges) {
      dwarf_ar_header_dump(&aranges->hdr);
      dwarf_arange_dump(aranges);
      aranges = aranges->next_ars;   
   }
}

void
dwarf_dump(Dwarf *dwarf) {
   dwarf_aranges_dump(dwarf);
   printf("\n");
   dwarf_abbrev_dump(dwarf);
   printf("\n");
   dwarf_info_dump(dwarf);
   printf("\n");
   dwarf_line_dump(dwarf);
   printf("\n");
   dwarf_str_dump(dwarf);
   printf("\n");
}

int
dwarf_open(Dwarf *dwarf, char *file) {
   Elf *elf = calloc(1, sizeof(Elf));
   Elf_Scn dbg_info_data;
   Elf_Scn dbg_abbrev_data;
   Elf_Scn dbg_line_data;
   Elf_Scn dbg_str_data;
   Elf_Scn dbg_aranges_data;
   volatile int rc = 0;

   if ((rc = elf_open(elf, file))) {
      return rc;
   }

   memset(dwarf, 0, sizeof(dwarf));

   if (elf_get_scn(elf, &dbg_info_data, ".debug_info") ||
         elf_get_scn(elf, &dbg_abbrev_data, ".debug_abbrev") ||
         elf_get_scn(elf, &dbg_line_data, ".debug_line") ||
         elf_get_scn(elf, &dbg_aranges_data, ".debug_aranges")) {
      asprintf(&dwarf->error, "File contains no debug data\n"); 
      return -2;
   }


   if (!setjmp(dwarf->env)) {
      dwarf->abbrevs = dwarf_read_abbrev(dbg_abbrev_data.buf, 
            dbg_abbrev_data.size);
      dwarf->cu = dwarf_read_cu(dwarf, dbg_info_data.buf, dbg_info_data.size);
      dwarf->sprog = dwarf_read_sprog(dwarf, dbg_line_data.buf, 
            dbg_line_data.size);

      if (!elf_get_scn(elf, &dbg_str_data, ".debug_str")) {
         dwarf->str = dwarf_read_str(dbg_str_data.buf, dbg_str_data.size);
      } else {
         dwarf->str = NULL; 
      }

      dwarf->aranges = dwarf_read_aranges(dwarf, dbg_aranges_data.buf, 
            dbg_aranges_data.size);
      dwarf->elf = elf;
   }

   return 0;
}

static void
dwarf_free_aranges(dwarf_aranges *aranges) {
   dwarf_aranges *tmp_aranges;
   dwarf_arange *tmp_arange, *arange;

   while (aranges) {
      tmp_aranges = aranges->next_ars;
      arange = aranges->arange;
      while (arange) {
         tmp_arange = arange->next_ar; 
         free(arange); 
         arange = tmp_arange;
      }
      free(aranges);
      aranges = tmp_aranges;
   }
}

static void
dwarf_free_pro_incl_dirs(dwarf_sprog_dir *dir) {
   dwarf_sprog_dir *tmp;

   while (dir) {
      tmp = dir->next;
      free(dir);
      dir = tmp;
   }
}

static void
dwarf_free_pro_files(dwarf_sprog_file *file) {
   dwarf_sprog_file *tmp;

   while (file) {
      tmp = file->next;
      free(file);
      file = tmp;
   }
}

static void
dwarf_free_sprog_pro(dwarf_sprog_pro *prologue) {
   free(prologue->std_opcode_len);
   dwarf_free_pro_incl_dirs(prologue->incl_dirs);
   dwarf_free_pro_files(prologue->files);
   free(prologue);
}

static void
dwarf_free_sprog_sm_regs(dwarf_sm_regs *sm_regs) {
   dwarf_sm_regs *tmp;

   while (sm_regs) {
      tmp = sm_regs->next;
      free(sm_regs);
      sm_regs = tmp;
   }
}

static void
dwarf_free_sprog(dwarf_sprog *sprog) {
   dwarf_sprog *tmp_sprog;

   while (sprog) {
      tmp_sprog = sprog->next;
      dwarf_free_sprog_pro(sprog->prologue);
      dwarf_free_sprog_sm_regs(sprog->sm_regs);
      free(sprog);
      sprog = tmp_sprog;
   }
}

static void
dwarf_free_die_att(dwarf_die_att *att) {
   dwarf_die_att *tmp_att;

   while (att) {
      tmp_att = att->next_att;

      switch (att->att_spec->form->id) {
         case DW_FORM_block: 
         case DW_FORM_block1: 
         case DW_FORM_block2: 
         case DW_FORM_block4: 
            free(att->value.b_val);
            break;
         default:
            break;
      }

      free(att);
      att = tmp_att;
   }

}

static void
dwarf_free_die(dwarf_die *die) {
   dwarf_die *tmp_die;

   while (die) {
      tmp_die = die->sibling;
      dwarf_free_die_att(die->att);
      dwarf_free_die(die->child); 
      free(die);
      die = tmp_die;
   }

}

static void
dwarf_free_cu(dwarf_cu *cu) {
   dwarf_cu *tmp_cu;

   while (cu) {
      tmp_cu = cu->next_cu;
      dwarf_free_die(cu->die);
      free(cu);
      cu = tmp_cu;
   }
}

static void
dwarf_free_att_spec(dwarf_att_spec *atts) {
   dwarf_att_spec *tmp_atts;

   while (atts) {
      tmp_atts = atts->next;
      free(atts);
      atts = tmp_atts;
   }
}

static void
dwarf_free_abbrev_tab(dwarf_abbrev_tab *tab) {
   dwarf_abbrev_tab *tmp_tab;

   while (tab) {
      tmp_tab = tab->next;
      dwarf_free_att_spec(tab->atts);
      free(tab);
      tab = tmp_tab;
   }
}

static void
dwarf_free_abbrevs(dwarf_abbrevs *abbrevs) {
   dwarf_abbrevs *tmp_abbrevs;

   while (abbrevs) {
      tmp_abbrevs = abbrevs->next;
      dwarf_free_abbrev_tab(abbrevs->tab);
      free(abbrevs);
      abbrevs = tmp_abbrevs;
   }
}

void
dwarf_free(Dwarf *dwarf) {
   dwarf_free_aranges(dwarf->aranges);
   free(dwarf->str);
   dwarf_free_sprog(dwarf->sprog);
   dwarf_free_cu(dwarf->cu);
   dwarf_free_abbrevs(dwarf->abbrevs);
   free(dwarf->elf);
}
