/* Target-dependent code for OpenVMS IA-64.

   Copyright (C) 2012 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "frame-unwind.h"
#include "ia64-tdep.h"
#include "osabi.h"
#include "gdbtypes.h"
#include "gdbcore.h"

#ifdef HAVE_LIBUNWIND_IA64_H

/* Libunwind callback accessor function to acquire procedure unwind-info.  */

static int
ia64_vms_find_proc_info_x (unw_addr_space_t as, unw_word_t ip,
                           unw_proc_info_t *pi,
                           int need_unwind_info, void *arg)
{
  enum bfd_endian byte_order = gdbarch_byte_order (target_gdbarch);
  unw_dyn_info_t di;
  int ret;
  gdb_byte buf[32];
  const char *annex = core_addr_to_string (ip);
  LONGEST res;
  CORE_ADDR table_addr;
  unsigned int info_len;

  res = target_read (&current_target, TARGET_OBJECT_OPENVMS_UIB,
                     annex + 2, buf, 0, sizeof (buf));

  if (res != sizeof (buf))
    return -UNW_ENOINFO;

  pi->format = UNW_INFO_FORMAT_REMOTE_TABLE;
  pi->start_ip = extract_unsigned_integer (buf + 0, 8, byte_order);
  pi->end_ip = extract_unsigned_integer (buf + 8, 8, byte_order);
  pi->gp = extract_unsigned_integer (buf + 24, 8, byte_order);
  table_addr = extract_unsigned_integer (buf + 16, 8, byte_order);

  if (table_addr == 0)
    {
      /* No unwind data.  */
      pi->unwind_info = NULL;
      pi->unwind_info_size = 0;
      return 0;
    }

  res = target_read_memory (table_addr, buf, 8);
  if (res != 0)
    return -UNW_ENOINFO;

  /* Check version.  */
  if (extract_unsigned_integer (buf + 6, 2, byte_order) != 1)
    return -UNW_EBADVERSION;
  info_len = extract_unsigned_integer (buf + 0, 4, byte_order);
  pi->unwind_info_size = 8 * info_len;

  /* Read info.  */
  pi->unwind_info = xmalloc (pi->unwind_info_size);

  res = target_read_memory (table_addr + 8,
                            pi->unwind_info, pi->unwind_info_size);
  if (res != 0)
    {
      xfree (pi->unwind_info);
      pi->unwind_info = NULL;
      return -UNW_ENOINFO;
    }

  /* FIXME: Handle OSSD (OS Specific Data).  This extension to ia64 unwind
     information by OpenVMS is currently not handled by libunwind, but
     looks to be used only in very specific context, and is not generated by
     GCC.  */

  pi->lsda = table_addr + 8 + pi->unwind_info_size;
  if (extract_unsigned_integer (buf + 4, 2, byte_order) & 3)
    {
      pi->lsda += 8;
      /* There might be an handler, but this is not used for unwinding.  */
      pi->handler = 0;
    }

  return 0;
}

/* Libunwind callback accessor function for cleanup.  */

static void
ia64_vms_put_unwind_info (unw_addr_space_t as,
                          unw_proc_info_t *pip, void *arg)
{
  /* Nothing required for now.  */
}

/* Libunwind callback accessor function to get head of the dynamic
   unwind-info registration list.  */

static int
ia64_vms_get_dyn_info_list (unw_addr_space_t as,
                            unw_word_t *dilap, void *arg)
{
  return -UNW_ENOINFO;
}

/* Set of libunwind callback acccessor functions.  */
static unw_accessors_t ia64_vms_unw_accessors;
static unw_accessors_t ia64_vms_unw_rse_accessors;

/* Set of ia64 gdb libunwind-frame callbacks and data for generic
   libunwind-frame code to use.  */
static struct libunwind_descr ia64_vms_libunwind_descr;

#endif /* HAVE_LIBUNWIND_IA64_H */

static void
ia64_openvms_init_abi (struct gdbarch_info info, struct gdbarch *gdbarch)
{
  set_gdbarch_long_double_format (gdbarch, floatformats_ia64_quad);

#ifdef HAVE_LIBUNWIND_IA64_H
  /* Override the default descriptor.  */
  ia64_vms_unw_accessors = ia64_unw_accessors;
  ia64_vms_unw_accessors.find_proc_info = ia64_vms_find_proc_info_x;
  ia64_vms_unw_accessors.put_unwind_info = ia64_vms_put_unwind_info;
  ia64_vms_unw_accessors.get_dyn_info_list_addr = ia64_vms_get_dyn_info_list;

  ia64_vms_unw_rse_accessors = ia64_unw_rse_accessors;
  ia64_vms_unw_rse_accessors.find_proc_info = ia64_vms_find_proc_info_x;
  ia64_vms_unw_rse_accessors.put_unwind_info = ia64_vms_put_unwind_info;
  ia64_vms_unw_rse_accessors.get_dyn_info_list_addr = ia64_vms_get_dyn_info_list;

  ia64_vms_libunwind_descr = ia64_libunwind_descr;
  ia64_vms_libunwind_descr.accessors = &ia64_vms_unw_accessors;
  ia64_vms_libunwind_descr.special_accessors = &ia64_vms_unw_rse_accessors;

  libunwind_frame_set_descr (gdbarch, &ia64_vms_libunwind_descr);
#endif
}

/* Provide a prototype to silence -Wmissing-prototypes.  */
extern initialize_file_ftype _initialize_ia64_hpux_tdep;

void
_initialize_ia64_hpux_tdep (void)
{
  gdbarch_register_osabi (bfd_arch_ia64, 0, GDB_OSABI_OPENVMS,
			  ia64_openvms_init_abi);
}
