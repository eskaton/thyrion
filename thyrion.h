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

#include <stdint.h>
#include <stdbool.h>
#include <setjmp.h>
#include <sys/types.h>

#include "elf_util.h"

#ifndef _THYRION_H
#define _THYRION_H

#define MK_TAG(tag) {tag, #tag}
#define MK_ATT(att) {att, #att}
#define MK_FORM(form) {form, #form}

typedef unsigned long   u_long;
typedef signed long     s_long;

typedef enum {
   no = 0,
   yes = 1
} dwarf_children;

typedef enum {
   DW_TAG_array_type = 0x01,
   DW_TAG_class_type = 0x02,
   DW_TAG_entry_point = 0x03,
   DW_TAG_enumeration_type = 0x04,
   DW_TAG_formal_parameter = 0x05,
   DW_TAG_imported_declaration = 0x08,
   DW_TAG_label = 0x0a,
   DW_TAG_lexical_block = 0x0b,
   DW_TAG_member = 0x0d,
   DW_TAG_pointer_type = 0x0f,
   DW_TAG_reference_type = 0x10,
   DW_TAG_compile_unit = 0x11,
   DW_TAG_string_type = 0x12,
   DW_TAG_structure_type = 0x13,
   DW_TAG_subroutine_type = 0x15,
   DW_TAG_typedef = 0x16,
   DW_TAG_union_type = 0x17,
   DW_TAG_unspecified_parameters = 0x18,
   DW_TAG_variant = 0x19,
   DW_TAG_common_block = 0x1a,
   DW_TAG_common_inclusion = 0x1b,
   DW_TAG_inheritance = 0x1c,
   DW_TAG_inlined_subroutine = 0x1d,
   DW_TAG_module = 0x1e,
   DW_TAG_ptr_to_member_type = 0x1f,
   DW_TAG_set_type = 0x20,
   DW_TAG_subrange_type = 0x21,
   DW_TAG_with_stmt = 0x22,
   DW_TAG_access_declaration = 0x23,
   DW_TAG_base_type = 0x24,
   DW_TAG_catch_block = 0x25,
   DW_TAG_const_type = 0x26,
   DW_TAG_constant = 0x27,
   DW_TAG_enumerator = 0x28,
   DW_TAG_file_type = 0x29,
   DW_TAG_friend = 0x2a,
   DW_TAG_namelist = 0x2b,
   DW_TAG_namelist_item = 0x2c,
   DW_TAG_packed_type = 0x2d,
   DW_TAG_subprogram = 0x2e,
   DW_TAG_template_type_param = 0x2f,
   DW_TAG_template_value_param = 0x30,
   DW_TAG_thrown_type = 0x31,
   DW_TAG_try_block = 0x32,
   DW_TAG_variant_part = 0x33,
   DW_TAG_variable = 0x34,
   DW_TAG_volatile_type = 0x35,
   DW_TAG_lo_user = 0x4080,
   DW_TAG_hi_user = 0xffff
} dwarf_tag_id;

typedef enum {
   DW_AT_sibling = 0x01,
   DW_AT_location = 0x02,
   DW_AT_name = 0x03,
   DW_AT_ordering = 0x09,
   DW_AT_byte_size = 0x0b,
   DW_AT_bit_offset = 0x0c,
   DW_AT_bit_size = 0x0d,
   DW_AT_stmt_list = 0x10,
   DW_AT_low_pc = 0x11,
   DW_AT_high_pc = 0x12,
   DW_AT_language = 0x13,
   DW_AT_discr = 0x15,
   DW_AT_discr_value = 0x16,
   DW_AT_visibility = 0x17,
   DW_AT_import = 0x18,
   DW_AT_string_length = 0x19,
   DW_AT_common_reference = 0x1a,
   DW_AT_comp_dir = 0x1b,
   DW_AT_const_value = 0x1c,
   DW_AT_containing_type = 0x1d,
   DW_AT_default_value = 0x1e,
   DW_AT_inline = 0x20,
   DW_AT_is_optional = 0x21,
   DW_AT_lower_bound = 0x22,
   DW_AT_producer = 0x25,
   DW_AT_prototyped = 0x27,
   DW_AT_return_addr = 0x2a,
   DW_AT_start_scope = 0x2c,
   DW_AT_stride_size = 0x2e,
   DW_AT_upper_bound = 0x2f,
   DW_AT_abstract_origin = 0x31,
   DW_AT_accessibility = 0x32,
   DW_AT_address_class = 0x33,
   DW_AT_artificial = 0x34,
   DW_AT_base_types = 0x35,
   DW_AT_calling_convention = 0x36,
   DW_AT_count = 0x37,
   DW_AT_data_member_location = 0x38,
   DW_AT_decl_column = 0x39,
   DW_AT_decl_file = 0x3a,
   DW_AT_decl_line = 0x3b,
   DW_AT_declaration = 0x3c,
   DW_AT_discr_list = 0x3d,
   DW_AT_encoding = 0x3e,
   DW_AT_external = 0x3f,
   DW_AT_frame_base = 0x40,
   DW_AT_friend = 0x41,
   DW_AT_identifier_case = 0x42,
   DW_AT_macro_info = 0x43,
   DW_AT_namelist_item = 0x44,
   DW_AT_priority = 0x46,
   DW_AT_segment = 0x46,
   DW_AT_specification = 0x47,
   DW_AT_static_link = 0x48,
   DW_AT_type = 0x49,
   DW_AT_use_location = 0x4a,
   DW_AT_variable_parameter = 0x4b,
   DW_AT_virtuality = 0x4c,
   DW_AT_vtable_elem_location = 0x4d,
   DW_AT_allocated = 0x4e,
   DW_AT_associated = 0x4f,
   DW_AT_data_location = 0x50,
   DW_AT_byte_stride = 0x51,
   DW_AT_entry_pc = 0x52,
   DW_AT_use_UTF8 = 0x53,
   DW_AT_extension = 0x54,
   DW_AT_ranges = 0x55,
   DW_AT_trampoline = 0x56,
   DW_AT_call_column = 0x57,
   DW_AT_call_file = 0x58,
   DW_AT_call_line = 0x59,
   DW_AT_description = 0x5a,
   DW_AT_binary_scale = 0x5b,
   DW_AT_decimal_scale = 0x5c,
   DW_AT_small = 0x5d,
   DW_AT_decimal_sign = 0x5e,
   DW_AT_digit_count = 0x5f,
   DW_AT_picture_string = 0x60,
   DW_AT_mutable = 0x61,
   DW_AT_threads_scaled = 0x62,
   DW_AT_explicit = 0x63,
   DW_AT_object_pointer = 0x64,
   DW_AT_endianity = 0x65,
   DW_AT_elemental = 0x66,
   DW_AT_pure = 0x67,
   DW_AT_recursive = 0x68,
   DW_AT_signature = 0x69,
   DW_AT_main_subprogramm = 0x6a,
   DW_AT_data_bit_offset = 0x6b,
   DW_AT_const_expr = 0x6c,
   DW_AT_enum_class = 0x6d,
   DW_AT_linkage_name = 0x6e,
   DW_AT_lo_user = 0x2000,
   DW_AT_hi_user = 0x3fff
} dwarf_att_id;

typedef enum {
   DW_FORM_addr = 0x01,
   DW_FORM_block2 = 0x03,
   DW_FORM_block4 = 0x04,
   DW_FORM_data2 = 0x05,
   DW_FORM_data4 = 0x06,
   DW_FORM_data8 = 0x07,
   DW_FORM_string = 0x08,
   DW_FORM_block = 0x09,
   DW_FORM_block1 = 0x0a,
   DW_FORM_data1 = 0x0b,
   DW_FORM_flag = 0x0c,
   DW_FORM_sdata = 0x0d,
   DW_FORM_strp = 0x0e,
   DW_FORM_udata = 0x0f,
   DW_FORM_ref_addr = 0x10,
   DW_FORM_ref1 = 0x11,
   DW_FORM_ref2 = 0x12,
   DW_FORM_ref4 = 0x13,
   DW_FORM_ref8 = 0x14,
   DW_FORM_ref_udata = 0x15,
   DW_FORM_indirect = 0x16
} dwarf_form_id;

typedef enum {
   DW_LNS_copy = 0x01,
   DW_LNS_advance_pc,
   DW_LNS_advance_line,
   DW_LNS_set_file,
   DW_LNS_set_column,
   DW_LNS_negate_stmt,
   DW_LNS_set_basic_block,
   DW_LNS_const_add_pc,
   DW_LNS_fixed_advance_pc,
   DW_LNS_set_prologue_end,
   DW_LNS_set_epilogue_begin,
   DW_LNS_set_isa
} dwarf_std_opcodes;

typedef enum {
   DW_LNE_end_sequence = 0x01,
   DW_LNE_set_address,
   DW_LNE_define_file,
   DW_LNE_set_discriminator
} dwarf_ext_opcodes;

typedef struct {
   dwarf_tag_id id;
   char *name;
} dwarf_tag;

typedef struct {
   dwarf_att_id id;
   char *name;
} dwarf_att;

typedef struct {
   dwarf_form_id id;
   char *name;
} dwarf_form;

typedef struct dwarf_att_spec {
  const dwarf_att *att;
  const dwarf_form *form;
  struct dwarf_att_spec *next;
} dwarf_att_spec;

typedef struct dwarf_abbrev_tab {
   uint32_t id;
   const dwarf_tag *tag;
   dwarf_children has_children;
   struct dwarf_att_spec *atts;
   struct dwarf_abbrev_tab *next;
} dwarf_abbrev_tab;

typedef struct dwarf_abbrevs {
   uint32_t offset;
   dwarf_abbrev_tab *tab;
   struct dwarf_abbrevs *next;
} dwarf_abbrevs;

#pragma pack(push)
#pragma pack(1)

typedef struct {
   uint32_t length;
   uint16_t version;
   uint32_t abbrev_off;
   uint8_t addr_size;
} dwarf_cu_header;

typedef struct {
   uint32_t length;
   uint16_t version;
   uint32_t info_off;
   uint8_t addr_size;
   uint8_t seg_size;
} dwarf_ar_header;

#pragma pack(pop)

typedef struct {
   uint32_t len;
   char *buf;
} dwarf_block;

typedef union {
   char *s_val;
   uint32_t ul_val;
   int32_t sl_val;
   dwarf_block *b_val;
} dwarf_value;

typedef struct dwarf_die_att {
   dwarf_att_spec *att_spec;
   dwarf_value value;
   struct dwarf_die_att *next_att;
} dwarf_die_att;

typedef struct dwarf_die {
   uint32_t abbrev_code;
   const dwarf_tag *tag;
   dwarf_die_att *att;
   struct dwarf_die *child;
   struct dwarf_die *sibling;
} dwarf_die;

typedef struct dwarf_cu {
   dwarf_cu_header hdr;
   dwarf_die *die; 
   struct dwarf_cu *next_cu;
} dwarf_cu;

typedef struct dwarf_arange {
   uint64_t address;
   uint64_t length;
   struct dwarf_arange *next_ar;
} dwarf_arange;

typedef struct dwarf_aranges {
   dwarf_ar_header hdr;
   dwarf_arange *arange;
   struct dwarf_aranges *next_ars;
} dwarf_aranges;

typedef struct dwarf_sm_regs {
   uint8_t opcode;
   uint8_t ext_opcode;
   long address;
   //TODO: add op_index
   uint32_t file;
   uint32_t line;
   uint32_t column;
   bool is_stmt;
   bool basic_block;
   bool end_sequence;
   bool prologue_end;
   bool epilogue_begin;
   uint32_t isa;
   uint32_t discriminator;
   struct dwarf_sm_regs *next;
} dwarf_sm_regs;

typedef struct dwarf_sprog_dir {
   char *name;
   struct dwarf_sprog_dir *next;
} dwarf_sprog_dir;


typedef struct dwarf_sprog_file {
   char *name;
   uint32_t dir_idx;
   uint32_t mtime;
   uint32_t size;
   struct dwarf_sprog_file *next;
} dwarf_sprog_file;

typedef struct {
   uint32_t total_len;
   uint16_t version;
   uint32_t prologue_len;
   uint8_t min_inst_len;
   uint8_t dflt_is_stmt;
   int8_t line_base;
   int8_t line_range;
   int8_t opcode_base;
   int8_t *std_opcode_len;
   dwarf_sprog_dir *incl_dirs;
   dwarf_sprog_file *files;
} dwarf_sprog_pro;

typedef struct dwarf_sprog {
   dwarf_sprog_pro *prologue;
   size_t sm_len;
   char *sm;
   dwarf_sm_regs *sm_regs;
   struct dwarf_sprog *next;
} dwarf_sprog;

typedef struct {
   char *table;
   uint32_t length;
} dwarf_str;

typedef struct Dwarf {
   dwarf_abbrevs *abbrevs;
   dwarf_cu *cu;
   dwarf_sprog *sprog;
   dwarf_str *str;
   dwarf_aranges *aranges;
   char *error;
   jmp_buf env;
   Elf *elf;
} Dwarf;

void
dwarf_dump(Dwarf *dwarf);

int
dwarf_open(Dwarf *dwarf, char *file);

void
dwarf_free(Dwarf *dwarf);

#endif // _THYRION_H
