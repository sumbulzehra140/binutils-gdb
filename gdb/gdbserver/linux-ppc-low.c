/* GNU/Linux/PowerPC specific low level interface, for the remote server for
   GDB.
   Copyright (C) 1995-2015 Free Software Foundation, Inc.

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

#include "server.h"
#include "linux-low.h"

#include <elf.h>
#include <asm/ptrace.h>

#include "nat/ppc-linux.h"
#include "ax.h"
#include "tracepoint.h"

static unsigned long ppc_hwcap;


/* Defined in auto-generated file powerpc-32l.c.  */
void init_registers_powerpc_32l (void);
extern const struct target_desc *tdesc_powerpc_32l;

/* Defined in auto-generated file powerpc-altivec32l.c.  */
void init_registers_powerpc_altivec32l (void);
extern const struct target_desc *tdesc_powerpc_altivec32l;

/* Defined in auto-generated file powerpc-cell32l.c.  */
void init_registers_powerpc_cell32l (void);
extern const struct target_desc *tdesc_powerpc_cell32l;

/* Defined in auto-generated file powerpc-vsx32l.c.  */
void init_registers_powerpc_vsx32l (void);
extern const struct target_desc *tdesc_powerpc_vsx32l;

/* Defined in auto-generated file powerpc-isa205-32l.c.  */
void init_registers_powerpc_isa205_32l (void);
extern const struct target_desc *tdesc_powerpc_isa205_32l;

/* Defined in auto-generated file powerpc-isa205-altivec32l.c.  */
void init_registers_powerpc_isa205_altivec32l (void);
extern const struct target_desc *tdesc_powerpc_isa205_altivec32l;

/* Defined in auto-generated file powerpc-isa205-vsx32l.c.  */
void init_registers_powerpc_isa205_vsx32l (void);
extern const struct target_desc *tdesc_powerpc_isa205_vsx32l;

/* Defined in auto-generated file powerpc-e500l.c.  */
void init_registers_powerpc_e500l (void);
extern const struct target_desc *tdesc_powerpc_e500l;

/* Defined in auto-generated file powerpc-64l.c.  */
void init_registers_powerpc_64l (void);
extern const struct target_desc *tdesc_powerpc_64l;

/* Defined in auto-generated file powerpc-altivec64l.c.  */
void init_registers_powerpc_altivec64l (void);
extern const struct target_desc *tdesc_powerpc_altivec64l;

/* Defined in auto-generated file powerpc-cell64l.c.  */
void init_registers_powerpc_cell64l (void);
extern const struct target_desc *tdesc_powerpc_cell64l;

/* Defined in auto-generated file powerpc-vsx64l.c.  */
void init_registers_powerpc_vsx64l (void);
extern const struct target_desc *tdesc_powerpc_vsx64l;

/* Defined in auto-generated file powerpc-isa205-64l.c.  */
void init_registers_powerpc_isa205_64l (void);
extern const struct target_desc *tdesc_powerpc_isa205_64l;

/* Defined in auto-generated file powerpc-isa205-altivec64l.c.  */
void init_registers_powerpc_isa205_altivec64l (void);
extern const struct target_desc *tdesc_powerpc_isa205_altivec64l;

/* Defined in auto-generated file powerpc-isa205-vsx64l.c.  */
void init_registers_powerpc_isa205_vsx64l (void);
extern const struct target_desc *tdesc_powerpc_isa205_vsx64l;

#define ppc_num_regs 73

#ifdef __powerpc64__
/* We use a constant for FPSCR instead of PT_FPSCR, because
   many shipped PPC64 kernels had the wrong value in ptrace.h.  */
static int ppc_regmap[] =
 {PT_R0 * 8,     PT_R1 * 8,     PT_R2 * 8,     PT_R3 * 8,
  PT_R4 * 8,     PT_R5 * 8,     PT_R6 * 8,     PT_R7 * 8,
  PT_R8 * 8,     PT_R9 * 8,     PT_R10 * 8,    PT_R11 * 8,
  PT_R12 * 8,    PT_R13 * 8,    PT_R14 * 8,    PT_R15 * 8,
  PT_R16 * 8,    PT_R17 * 8,    PT_R18 * 8,    PT_R19 * 8,
  PT_R20 * 8,    PT_R21 * 8,    PT_R22 * 8,    PT_R23 * 8,
  PT_R24 * 8,    PT_R25 * 8,    PT_R26 * 8,    PT_R27 * 8,
  PT_R28 * 8,    PT_R29 * 8,    PT_R30 * 8,    PT_R31 * 8,
  PT_FPR0*8,     PT_FPR0*8 + 8, PT_FPR0*8+16,  PT_FPR0*8+24,
  PT_FPR0*8+32,  PT_FPR0*8+40,  PT_FPR0*8+48,  PT_FPR0*8+56,
  PT_FPR0*8+64,  PT_FPR0*8+72,  PT_FPR0*8+80,  PT_FPR0*8+88,
  PT_FPR0*8+96,  PT_FPR0*8+104,  PT_FPR0*8+112,  PT_FPR0*8+120,
  PT_FPR0*8+128, PT_FPR0*8+136,  PT_FPR0*8+144,  PT_FPR0*8+152,
  PT_FPR0*8+160,  PT_FPR0*8+168,  PT_FPR0*8+176,  PT_FPR0*8+184,
  PT_FPR0*8+192,  PT_FPR0*8+200,  PT_FPR0*8+208,  PT_FPR0*8+216,
  PT_FPR0*8+224,  PT_FPR0*8+232,  PT_FPR0*8+240,  PT_FPR0*8+248,
  PT_NIP * 8,    PT_MSR * 8,    PT_CCR * 8,    PT_LNK * 8,
  PT_CTR * 8,    PT_XER * 8,    PT_FPR0*8 + 256,
  PT_ORIG_R3 * 8, PT_TRAP * 8 };
#else
/* Currently, don't check/send MQ.  */
static int ppc_regmap[] =
 {PT_R0 * 4,     PT_R1 * 4,     PT_R2 * 4,     PT_R3 * 4,
  PT_R4 * 4,     PT_R5 * 4,     PT_R6 * 4,     PT_R7 * 4,
  PT_R8 * 4,     PT_R9 * 4,     PT_R10 * 4,    PT_R11 * 4,
  PT_R12 * 4,    PT_R13 * 4,    PT_R14 * 4,    PT_R15 * 4,
  PT_R16 * 4,    PT_R17 * 4,    PT_R18 * 4,    PT_R19 * 4,
  PT_R20 * 4,    PT_R21 * 4,    PT_R22 * 4,    PT_R23 * 4,
  PT_R24 * 4,    PT_R25 * 4,    PT_R26 * 4,    PT_R27 * 4,
  PT_R28 * 4,    PT_R29 * 4,    PT_R30 * 4,    PT_R31 * 4,
  PT_FPR0*4,     PT_FPR0*4 + 8, PT_FPR0*4+16,  PT_FPR0*4+24,
  PT_FPR0*4+32,  PT_FPR0*4+40,  PT_FPR0*4+48,  PT_FPR0*4+56,
  PT_FPR0*4+64,  PT_FPR0*4+72,  PT_FPR0*4+80,  PT_FPR0*4+88,
  PT_FPR0*4+96,  PT_FPR0*4+104,  PT_FPR0*4+112,  PT_FPR0*4+120,
  PT_FPR0*4+128, PT_FPR0*4+136,  PT_FPR0*4+144,  PT_FPR0*4+152,
  PT_FPR0*4+160,  PT_FPR0*4+168,  PT_FPR0*4+176,  PT_FPR0*4+184,
  PT_FPR0*4+192,  PT_FPR0*4+200,  PT_FPR0*4+208,  PT_FPR0*4+216,
  PT_FPR0*4+224,  PT_FPR0*4+232,  PT_FPR0*4+240,  PT_FPR0*4+248,
  PT_NIP * 4,    PT_MSR * 4,    PT_CCR * 4,    PT_LNK * 4,
  PT_CTR * 4,    PT_XER * 4,    PT_FPSCR * 4,
  PT_ORIG_R3 * 4, PT_TRAP * 4
 };

static int ppc_regmap_e500[] =
 {PT_R0 * 4,     PT_R1 * 4,     PT_R2 * 4,     PT_R3 * 4,
  PT_R4 * 4,     PT_R5 * 4,     PT_R6 * 4,     PT_R7 * 4,
  PT_R8 * 4,     PT_R9 * 4,     PT_R10 * 4,    PT_R11 * 4,
  PT_R12 * 4,    PT_R13 * 4,    PT_R14 * 4,    PT_R15 * 4,
  PT_R16 * 4,    PT_R17 * 4,    PT_R18 * 4,    PT_R19 * 4,
  PT_R20 * 4,    PT_R21 * 4,    PT_R22 * 4,    PT_R23 * 4,
  PT_R24 * 4,    PT_R25 * 4,    PT_R26 * 4,    PT_R27 * 4,
  PT_R28 * 4,    PT_R29 * 4,    PT_R30 * 4,    PT_R31 * 4,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  -1,            -1,            -1,            -1,
  PT_NIP * 4,    PT_MSR * 4,    PT_CCR * 4,    PT_LNK * 4,
  PT_CTR * 4,    PT_XER * 4,    -1,
  PT_ORIG_R3 * 4, PT_TRAP * 4
 };
#endif

static int
ppc_cannot_store_register (int regno)
{
  const struct target_desc *tdesc = current_process ()->tdesc;

#ifndef __powerpc64__
  /* Some kernels do not allow us to store fpscr.  */
  if (!(ppc_hwcap & PPC_FEATURE_HAS_SPE)
      && regno == find_regno (tdesc, "fpscr"))
    return 2;
#endif

  /* Some kernels do not allow us to store orig_r3 or trap.  */
  if (regno == find_regno (tdesc, "orig_r3")
      || regno == find_regno (tdesc, "trap"))
    return 2;

  return 0;
}

static int
ppc_cannot_fetch_register (int regno)
{
  return 0;
}

static void
ppc_collect_ptrace_register (struct regcache *regcache, int regno, char *buf)
{
  memset (buf, 0, sizeof (long));

  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
      /* Little-endian values always sit at the left end of the buffer.  */
      collect_register (regcache, regno, buf);
    }
  else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
      /* Big-endian values sit at the right end of the buffer.  In case of
         registers whose sizes are smaller than sizeof (long), we must use a
         padding to access them correctly.  */
      int size = register_size (regcache->tdesc, regno);

      if (size < sizeof (long))
	collect_register (regcache, regno, buf + sizeof (long) - size);
      else
	collect_register (regcache, regno, buf);
    }
  else
    perror_with_name ("Unexpected byte order");
}

static void
ppc_supply_ptrace_register (struct regcache *regcache,
			    int regno, const char *buf)
{
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
      /* Little-endian values always sit at the left end of the buffer.  */
      supply_register (regcache, regno, buf);
    }
  else if (__BYTE_ORDER == __BIG_ENDIAN)
    {
      /* Big-endian values sit at the right end of the buffer.  In case of
         registers whose sizes are smaller than sizeof (long), we must use a
         padding to access them correctly.  */
      int size = register_size (regcache->tdesc, regno);

      if (size < sizeof (long))
	supply_register (regcache, regno, buf + sizeof (long) - size);
      else
	supply_register (regcache, regno, buf);
    }
  else
    perror_with_name ("Unexpected byte order");
}


#define INSTR_SC        0x44000002
#define NR_spu_run      0x0116

/* If the PPU thread is currently stopped on a spu_run system call,
   return to FD and ADDR the file handle and NPC parameter address
   used with the system call.  Return non-zero if successful.  */
static int
parse_spufs_run (struct regcache *regcache, int *fd, CORE_ADDR *addr)
{
  CORE_ADDR curr_pc;
  int curr_insn;
  int curr_r0;

  if (register_size (regcache->tdesc, 0) == 4)
    {
      unsigned int pc, r0, r3, r4;
      collect_register_by_name (regcache, "pc", &pc);
      collect_register_by_name (regcache, "r0", &r0);
      collect_register_by_name (regcache, "orig_r3", &r3);
      collect_register_by_name (regcache, "r4", &r4);
      curr_pc = (CORE_ADDR) pc;
      curr_r0 = (int) r0;
      *fd = (int) r3;
      *addr = (CORE_ADDR) r4;
    }
  else
    {
      unsigned long pc, r0, r3, r4;
      collect_register_by_name (regcache, "pc", &pc);
      collect_register_by_name (regcache, "r0", &r0);
      collect_register_by_name (regcache, "orig_r3", &r3);
      collect_register_by_name (regcache, "r4", &r4);
      curr_pc = (CORE_ADDR) pc;
      curr_r0 = (int) r0;
      *fd = (int) r3;
      *addr = (CORE_ADDR) r4;
    }

  /* Fetch instruction preceding current NIP.  */
  if ((*the_target->read_memory) (curr_pc - 4,
				  (unsigned char *) &curr_insn, 4) != 0)
    return 0;
  /* It should be a "sc" instruction.  */
  if (curr_insn != INSTR_SC)
    return 0;
  /* System call number should be NR_spu_run.  */
  if (curr_r0 != NR_spu_run)
    return 0;

  return 1;
}

static CORE_ADDR
ppc_get_pc (struct regcache *regcache)
{
  CORE_ADDR addr;
  int fd;

  if (parse_spufs_run (regcache, &fd, &addr))
    {
      unsigned int pc;
      (*the_target->read_memory) (addr, (unsigned char *) &pc, 4);
      return ((CORE_ADDR)1 << 63)
	| ((CORE_ADDR)fd << 32) | (CORE_ADDR) (pc - 4);
    }
  else if (register_size (regcache->tdesc, 0) == 4)
    {
      unsigned int pc;
      collect_register_by_name (regcache, "pc", &pc);
      return (CORE_ADDR) pc;
    }
  else
    {
      unsigned long pc;
      collect_register_by_name (regcache, "pc", &pc);
      return (CORE_ADDR) pc;
    }
}

static void
ppc_set_pc (struct regcache *regcache, CORE_ADDR pc)
{
  CORE_ADDR addr;
  int fd;

  if (parse_spufs_run (regcache, &fd, &addr))
    {
      unsigned int newpc = pc;
      (*the_target->write_memory) (addr, (unsigned char *) &newpc, 4);
    }
  else if (register_size (regcache->tdesc, 0) == 4)
    {
      unsigned int newpc = pc;
      supply_register_by_name (regcache, "pc", &newpc);
    }
  else
    {
      unsigned long newpc = pc;
      supply_register_by_name (regcache, "pc", &newpc);
    }
}


static int
ppc_get_hwcap (unsigned long *valp)
{
  const struct target_desc *tdesc = current_process ()->tdesc;
  int wordsize = register_size (tdesc, 0);
  unsigned char *data = alloca (2 * wordsize);
  int offset = 0;

  while ((*the_target->read_auxv) (offset, data, 2 * wordsize) == 2 * wordsize)
    {
      if (wordsize == 4)
	{
	  unsigned int *data_p = (unsigned int *)data;
	  if (data_p[0] == AT_HWCAP)
	    {
	      *valp = data_p[1];
	      return 1;
	    }
	}
      else
	{
	  unsigned long *data_p = (unsigned long *)data;
	  if (data_p[0] == AT_HWCAP)
	    {
	      *valp = data_p[1];
	      return 1;
	    }
	}

      offset += 2 * wordsize;
    }

  *valp = 0;
  return 0;
}

/* Forward declaration.  */
static struct usrregs_info ppc_usrregs_info;
#ifndef __powerpc64__
static int ppc_regmap_adjusted;
#endif

static void
ppc_arch_setup (void)
{
  const struct target_desc *tdesc;
#ifdef __powerpc64__
  long msr;
  struct regcache *regcache;

  /* On a 64-bit host, assume 64-bit inferior process with no
     AltiVec registers.  Reset ppc_hwcap to ensure that the
     collect_register call below does not fail.  */
  tdesc = tdesc_powerpc_64l;
  current_process ()->tdesc = tdesc;
  ppc_hwcap = 0;

  regcache = new_register_cache (tdesc);
  fetch_inferior_registers (regcache, find_regno (tdesc, "msr"));
  collect_register_by_name (regcache, "msr", &msr);
  free_register_cache (regcache);
  if (ppc64_64bit_inferior_p (msr))
    {
      ppc_get_hwcap (&ppc_hwcap);
      if (ppc_hwcap & PPC_FEATURE_CELL)
	tdesc = tdesc_powerpc_cell64l;
      else if (ppc_hwcap & PPC_FEATURE_HAS_VSX)
	{
	  /* Power ISA 2.05 (implemented by Power 6 and newer processors)
	     increases the FPSCR from 32 bits to 64 bits. Even though Power 7
	     supports this ISA version, it doesn't have PPC_FEATURE_ARCH_2_05
	     set, only PPC_FEATURE_ARCH_2_06.  Since for now the only bits
	     used in the higher half of the register are for Decimal Floating
	     Point, we check if that feature is available to decide the size
	     of the FPSCR.  */
	  if (ppc_hwcap & PPC_FEATURE_HAS_DFP)
	    tdesc = tdesc_powerpc_isa205_vsx64l;
	  else
	    tdesc = tdesc_powerpc_vsx64l;
	}
      else if (ppc_hwcap & PPC_FEATURE_HAS_ALTIVEC)
	{
	  if (ppc_hwcap & PPC_FEATURE_HAS_DFP)
	    tdesc = tdesc_powerpc_isa205_altivec64l;
	  else
	    tdesc = tdesc_powerpc_altivec64l;
	}

      current_process ()->tdesc = tdesc;
      return;
    }
#endif

  /* OK, we have a 32-bit inferior.  */
  tdesc = tdesc_powerpc_32l;
  current_process ()->tdesc = tdesc;

  ppc_get_hwcap (&ppc_hwcap);
  if (ppc_hwcap & PPC_FEATURE_CELL)
    tdesc = tdesc_powerpc_cell32l;
  else if (ppc_hwcap & PPC_FEATURE_HAS_VSX)
    {
      if (ppc_hwcap & PPC_FEATURE_HAS_DFP)
	tdesc = tdesc_powerpc_isa205_vsx32l;
      else
	tdesc = tdesc_powerpc_vsx32l;
    }
  else if (ppc_hwcap & PPC_FEATURE_HAS_ALTIVEC)
    {
      if (ppc_hwcap & PPC_FEATURE_HAS_DFP)
	tdesc = tdesc_powerpc_isa205_altivec32l;
      else
	tdesc = tdesc_powerpc_altivec32l;
    }

  /* On 32-bit machines, check for SPE registers.
     Set the low target's regmap field as appropriately.  */
#ifndef __powerpc64__
  if (ppc_hwcap & PPC_FEATURE_HAS_SPE)
    tdesc = tdesc_powerpc_e500l;

  if (!ppc_regmap_adjusted)
    {
      if (ppc_hwcap & PPC_FEATURE_HAS_SPE)
	ppc_usrregs_info.regmap = ppc_regmap_e500;

      /* If the FPSCR is 64-bit wide, we need to fetch the whole
	 64-bit slot and not just its second word.  The PT_FPSCR
	 supplied in a 32-bit GDB compilation doesn't reflect
	 this.  */
      if (register_size (tdesc, 70) == 8)
	ppc_regmap[70] = (48 + 2*32) * sizeof (long);

      ppc_regmap_adjusted = 1;
   }
#endif
  current_process ()->tdesc = tdesc;
}

/* Correct in either endianness.
   This instruction is "twge r2, r2", which GDB uses as a software
   breakpoint.  */
static const unsigned int ppc_breakpoint = 0x7d821008;
#define ppc_breakpoint_len 4

static int
ppc_breakpoint_at (CORE_ADDR where)
{
  unsigned int insn;

  if (where & ((CORE_ADDR)1 << 63))
    {
      char mem_annex[32];
      sprintf (mem_annex, "%d/mem", (int)((where >> 32) & 0x7fffffff));
      (*the_target->qxfer_spu) (mem_annex, (unsigned char *) &insn,
				NULL, where & 0xffffffff, 4);
      if (insn == 0x3fff)
	return 1;
    }
  else
    {
      (*the_target->read_memory) (where, (unsigned char *) &insn, 4);
      if (insn == ppc_breakpoint)
	return 1;
      /* If necessary, recognize more trap instructions here.  GDB only uses
	 the one.  */
    }

  return 0;
}

static int
ppc_supports_z_point_type (char z_type)
{
  switch (z_type)
    {
    case Z_PACKET_SW_BP:
      return 1;
    case Z_PACKET_HW_BP:
    case Z_PACKET_WRITE_WP:
    case Z_PACKET_ACCESS_WP:
    default:
      return 0;
    }
}

static int
ppc_insert_point (enum raw_bkpt_type type, CORE_ADDR addr,
		  int size, struct raw_breakpoint *bp)
{
  switch (type)
    {
    case raw_bkpt_type_sw:
      return insert_memory_breakpoint (bp);

    case raw_bkpt_type_hw:
    case raw_bkpt_type_write_wp:
    case raw_bkpt_type_access_wp:
    default:
      /* Unsupported.  */
      return 1;
    }
}

static int
ppc_remove_point (enum raw_bkpt_type type, CORE_ADDR addr,
		  int size, struct raw_breakpoint *bp)
{
  switch (type)
    {
    case raw_bkpt_type_sw:
      return remove_memory_breakpoint (bp);

    case raw_bkpt_type_hw:
    case raw_bkpt_type_write_wp:
    case raw_bkpt_type_access_wp:
    default:
      /* Unsupported.  */
      return 1;
    }
}

/* Put a 32-bit INSN instruction in BUF in target endian.  */

static int
put_i32 (unsigned char *buf, uint32_t insn)
{
  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    {
      buf[3] = (insn >> 24) & 0xff;
      buf[2] = (insn >> 16) & 0xff;
      buf[1] = (insn >> 8) & 0xff;
      buf[0] = insn & 0xff;
    }
  else
    {
      buf[0] = (insn >> 24) & 0xff;
      buf[1] = (insn >> 16) & 0xff;
      buf[2] = (insn >> 8) & 0xff;
      buf[3] = insn & 0xff;
    }

  return 4;
}

/* return a 32-bit value in target endian in BUF.  */

static uint32_t
get_i32 (unsigned char *buf)
{
  uint32_t r;

  if (__BYTE_ORDER == __LITTLE_ENDIAN)
    r = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
  else
    r = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];

  return r;
}

/* Generate a ds-form instruction in BUF and return the number of bytes written

   0      6     11   16          30 32
   | OPCD | RST | RA |     DS    |XO|  */

static int
gen_ds_form (unsigned char *buf, int opcd, int rst, int ra, int ds, int xo)
{
  uint32_t insn = opcd << 26;

  insn |= (rst << 21) | (ra << 16) | (ds & 0xfffc) | (xo & 0x3);
  return put_i32 (buf, insn);
}

/* Followings are frequently used ds-form instructions.  */

#define GEN_STD(buf, rs, ra, offset)	gen_ds_form (buf, 62, rs, ra, offset, 0)
#define GEN_STDU(buf, rs, ra, offset)	gen_ds_form (buf, 62, rs, ra, offset, 1)
#define GEN_LD(buf, rt, ra, offset)	gen_ds_form (buf, 58, rt, ra, offset, 0)
#define GEN_LDU(buf, rt, ra, offset)	gen_ds_form (buf, 58, rt, ra, offset, 1)

/* Generate a d-form instruction in BUF.

   0      6     11   16             32
   | OPCD | RST | RA |       D      |  */

static int
gen_d_form (unsigned char *buf, int opcd, int rst, int ra, int si)
{
  uint32_t insn = opcd << 26;

  insn |= (rst << 21) | (ra << 16) | (si & 0xffff);
  return put_i32 (buf, insn);
}

/* Followings are frequently used d-form instructions.  */

#define GEN_ADDI(buf, rt, ra, si)	gen_d_form (buf, 14, rt, ra, si)
#define GEN_ADDIS(buf, rt, ra, si)	gen_d_form (buf, 15, rt, ra, si)
#define GEN_LI(buf, rt, si)		GEN_ADDI (buf, rt, 0, si)
#define GEN_LIS(buf, rt, si)		GEN_ADDIS (buf, rt, 0, si)
#define GEN_ORI(buf, rt, ra, si)	gen_d_form (buf, 24, rt, ra, si)
#define GEN_ORIS(buf, rt, ra, si)	gen_d_form (buf, 25, rt, ra, si)
#define GEN_LBZ(buf, rt, ra, si)	gen_d_form (buf, 34, rt, ra, si)
#define GEN_LHZ(buf, rt, ra, si)	gen_d_form (buf, 40, rt, ra, si)
#define GEN_LWZ(buf, rt, ra, si)	gen_d_form (buf, 32, rt, ra, si)
#define GEN_STW(buf, rt, ra, si)	gen_d_form (buf, 36, rt, ra, si)
/* Assume bf = cr7.  */
#define GEN_CMPWI(buf, ra, si)   gen_d_form (buf, 11, 28, ra, si)
#define GEN_CMPDI(buf, ra, si)   gen_d_form (buf, 11, 28 | 1, ra, si)
#define GEN_CMPLWI(buf, ra, ui)  gen_d_form (buf, 10, 28, ra, ui)
#define GEN_CMPLDI(buf, ra, ui)  gen_d_form (buf, 10, 28 | 1, ra, ui)

/* Generate a xfx-form instruction in BUF and return the number of bytes
   written.

   0      6     11         21        31 32
   | OPCD | RST |    RI    |    XO   |/|  */

static int
gen_xfx_form (unsigned char *buf, int opcd, int rst, int ri, int xo)
{
  uint32_t insn = opcd << 26;
  unsigned int n = ((ri & 0x1f) << 5) | ((ri >> 5) & 0x1f);

  insn |= (rst << 21) | (n << 11) | (xo << 1);
  return put_i32 (buf, insn);
}

/* Followings are frequently used xfx-form instructions.  */

#define GEN_MFSPR(buf, rt, spr)		gen_xfx_form (buf, 31, rt, spr, 339)
#define GEN_MTSPR(buf, rt, spr)		gen_xfx_form (buf, 31, rt, spr, 467)

/* Generate a x-form instruction in BUF and return the number of bytes written.

   0      6     11   16   21       31 32
   | OPCD | RST | RA | RB |   XO   |RC|  */

static int
gen_x_form (unsigned char *buf, int opcd, int rst, int ra, int rb,
	    int xo, int rc)
{
  uint32_t insn = opcd << 26;

  insn |= (rst << 21) | (ra << 16) | (rb << 11) | (xo << 1) | rc;
  return put_i32 (buf, insn);
}

/* Followings are frequently used x-form instructions.  */

#define GEN_AND(buf, ra, rs, rb)	gen_x_form (buf, 31, rs, ra, rb, 28, 0)
#define GEN_OR(buf, ra, rs, rb)		gen_x_form (buf, 31, rs, ra, rb, 444, 0)
#define GEN_XOR(buf, ra, rs, rb)	gen_x_form (buf, 31, rs, ra, rb, 316, 0)
#define GEN_NOR(buf, ra, rs, rb)	gen_x_form (buf, 31, rs, ra, rb, 124, 0)
#define GEN_SLD(buf, ra, rs, rb)	gen_x_form (buf, 31, rs, ra, rb, 27, 0)
#define GEN_SRAD(buf, ra, rs, rb)	gen_x_form (buf, 31, rs, ra, rb, 794, 0)
#define GEN_SRD(buf, ra, rs, rb)	gen_x_form (buf, 31, rs, ra, rb, 539, 0)
#define GEN_MR(buf, ra, rs)		GEN_OR (buf, ra, rs, rs)
#define GEN_EXTSB(buf, ra, rs)		gen_x_form (buf, 31, rs, ra, 0, 954, 0)
#define GEN_EXTSH(buf, ra, rs)		gen_x_form (buf, 31, rs, ra, 0, 922, 0)
#define GEN_EXTSW(buf, ra, rs)		gen_x_form (buf, 31, rs, ra, 0, 986, 0)
#define GEN_LWARX(buf, rt, ra, rb)	gen_x_form (buf, 31, rt, ra, rb, 20, 0)
#define GEN_STWCX(buf, rs, ra, rb)	gen_x_form (buf, 31, rs, ra, rb, 150, 1)
/* Assume bf = cr7.  */
#define GEN_CMPW(buf, ra, rb)    gen_x_form (buf, 31, 28, ra, rb, 0, 0)
#define GEN_CMPD(buf, ra, rb)    gen_x_form (buf, 31, 28 | 1, ra, rb, 0, 0)
#define GEN_CMPLW(buf, ra, rb)   gen_x_form (buf, 31, 28, ra, rb, 32, 0)
#define GEN_CMPLD(buf, ra, rb)   gen_x_form (buf, 31, 28 | 1, ra, rb, 32, 0)

/* Generate a md-form instruction in BUF and return the number of bytes written.

   0      6    11   16   21   27   30 31 32
   | OPCD | RS | RA | sh | mb | XO |sh|Rc|  */

static int
gen_md_form (unsigned char *buf, int opcd, int rs, int ra, int sh, int mb,
	     int xo, int rc)
{
  uint32_t insn = opcd << 26;
  unsigned int n = ((mb & 0x1f) << 1) | ((mb >> 5) & 0x1);
  unsigned int sh0_4 = sh & 0x1f;
  unsigned int sh5 = (sh >> 5) & 1;

  insn |= (rs << 21) | (ra << 16) | (sh0_4 << 11) | (n << 5) | (sh5 << 1)
	  | (xo << 2);
  return put_i32 (buf, insn);
}

/* The following are frequently used md-form instructions.  */

#define GEN_RLDICL(buf, ra, rs ,sh, mb) \
				gen_md_form (buf, 30, rs, ra, sh, mb, 0, 0)
#define GEN_RLDICR(buf, ra, rs ,sh, mb) \
				gen_md_form (buf, 30, rs, ra, sh, mb, 1, 0)

/* Generate a i-form instruction in BUF and return the number of bytes written.

   0      6                          30 31 32
   | OPCD |            LI            |AA|LK|  */

static int
gen_i_form (unsigned char *buf, int opcd, int li, int aa, int lk)
{
  uint32_t insn = opcd << 26;

  insn |= (li & 0x3fffffc) | (aa & 1) | (lk & 1);
  return put_i32 (buf, insn);
}

/* The following are frequently used i-form instructions.  */

#define GEN_B(buf, li)		gen_i_form (buf, 18, li, 0, 0)
#define GEN_BL(buf, li)		gen_i_form (buf, 18, li, 0, 1)

/* Generate a b-form instruction in BUF and return the number of bytes written.

   0      6    11   16               30 31 32
   | OPCD | BO | BI |      BD        |AA|LK|  */

static int
gen_b_form (unsigned char *buf, int opcd, int bo, int bi, int bd,
	    int aa, int lk)
{
  uint32_t insn = opcd << 26;

  insn |= (bo << 21) | (bi << 16) | (bd & 0xfffc) | (aa & 1) | (lk & 1);
  return put_i32 (buf, insn);
}

/* The following are frequently used b-form instructions.  */
/* Assume bi = cr7.  */
#define GEN_BLT(buf, bd)  gen_b_form (buf, 16, 0xc, (7 << 2) | 0, bd, 0 ,0)
#define GEN_BGT(buf, bd)  gen_b_form (buf, 16, 0xc, (7 << 2) | 1, bd, 0 ,0)
#define GEN_BEQ(buf, bd)  gen_b_form (buf, 16, 0xc, (7 << 2) | 2, bd, 0 ,0)
#define GEN_BGE(buf, bd)  gen_b_form (buf, 16, 0x4, (7 << 2) | 0, bd, 0 ,0)
#define GEN_BLE(buf, bd)  gen_b_form (buf, 16, 0x4, (7 << 2) | 1, bd, 0 ,0)
#define GEN_BNE(buf, bd)  gen_b_form (buf, 16, 0x4, (7 << 2) | 2, bd, 0 ,0)

/* GEN_LOAD and GEN_STORE generate 64- or 32-bit load/store for ppc64 or ppc32
   respectively.  They are primary used for save/restore GPRs in jump-pad,
   not used for bytecode compiling.  */

#if defined __PPC64__
#define GEN_LOAD(buf, rt, ra, si)	GEN_LD (buf, rt, ra, si)
#define GEN_STORE(buf, rt, ra, si)	GEN_STD (buf, rt, ra, si)
#else
#define GEN_LOAD(buf, rt, ra, si)	GEN_LWZ (buf, rt, ra, si)
#define GEN_STORE(buf, rt, ra, si)	GEN_STW (buf, rt, ra, si)
#endif

static void
emit_insns (unsigned char *buf, int n)
{
  write_inferior_memory (current_insn_ptr, buf, n);
  current_insn_ptr += n;
}

#define EMIT_ASM(NAME, INSNS)						\
  do									\
    {									\
      extern unsigned char start_bcax_ ## NAME [], end_bcax_ ## NAME [];		\
      emit_insns (start_bcax_ ## NAME,					\
		  end_bcax_ ## NAME - start_bcax_ ## NAME);		\
      __asm__ (".section .text.__ppcbcax\n\t"				\
	       "start_bcax_" #NAME ":\n\t"				\
	       INSNS "\n\t"						\
	       "end_bcax_" #NAME ":\n\t"				\
	       ".previous\n\t");					\
    } while (0)

/* Generate a sequence of instructions to load IMM in the register REG.
   Write the instructions in BUF and return the number of bytes written.  */

static int
gen_limm (unsigned char *buf, int reg, uint64_t imm)
{
  int i = 0;

  if ((imm >> 8) == 0)
    {
      /* li	reg, imm[7:0] */
      i += GEN_LI (buf + i, reg, imm);
    }
  else if ((imm >> 16) == 0)
    {
      /* li	reg, 0
	 ori	reg, reg, imm[15:0] */
      i += GEN_LI (buf + i, reg, 0);
      i += GEN_ORI (buf + i, reg, reg, imm);
    }
  else if ((imm >> 32) == 0)
    {
      /* lis	reg, imm[31:16]
	 ori	reg, reg, imm[15:0]
	 rldicl	reg, reg, 0, 32 */
      i += GEN_LIS (buf + i, reg, (imm >> 16) & 0xffff);
      i += GEN_ORI (buf + i, reg, reg, imm & 0xffff);
      i += GEN_RLDICL (buf + i, reg, reg, 0, 32);
    }
  else
    {
      /* lis    reg, <imm[63:48]>
	 ori    reg, reg, <imm[48:32]>
	 rldicr reg, reg, 32, 31
	 oris   reg, reg, <imm[31:16]>
	 ori    reg, reg, <imm[15:0]> */
      i += GEN_LIS (buf + i, reg, ((imm >> 48) & 0xffff));
      i += GEN_ORI (buf + i, reg, reg, ((imm >> 32) & 0xffff));
      i += GEN_RLDICR (buf + i, reg, reg, 32, 31);
      i += GEN_ORIS (buf + i, reg, reg, ((imm >> 16) & 0xffff));
      i += GEN_ORI (buf + i, reg, reg, (imm & 0xffff));
    }

  return i;
}

/* Generate a sequence for atomically exchange at location LOCK.
   This code sequence clobbers r6, r7, r8, r9.  */

static int
gen_atomic_xchg (unsigned char *buf, CORE_ADDR lock, int old_value, int new_value)
{
  int i = 0;
  const int r_lock = 6;
  const int r_old = 7;
  const int r_new = 8;
  const int r_tmp = 9;

  /*

  1: lwsync
  2: lwarx   TMP, 0, LOCK
     cmpwi   TMP, OLD
     bne     1b
     stwcx.  NEW, 0, LOCK
     bne     2b */


  i += gen_limm (buf + i, r_lock, lock);
  i += gen_limm (buf + i, r_new, new_value);
  i += gen_limm (buf + i, r_old, old_value);

  i += put_i32 (buf + i, 0x7c2004ac);	/* lwsync */
  i += GEN_LWARX (buf + i, r_tmp, 0, r_lock);
  i += GEN_CMPW (buf + i, r_tmp, r_old);
  i += GEN_BNE (buf + i, -12);
  i += GEN_STWCX (buf + i, r_new, 0, r_lock);
  i += GEN_BNE (buf + i, -16);

  return i;
}

/* Generate a sequence of instructions for calling a function
   at address of FN.  Return the number of bytes are written in BUF.

   FIXME: For ppc64be, FN should be the address to the function
   descriptor, so we should load 8(FN) to R2, 16(FN) to R11
   and then call the function-entry at 0(FN).  However, current GDB
   implicitly convert the address to function descriptor to the actual
   function. See qSymbol handling in remote.c.  Although it seems we
   can successfully call however, things go wrong when callee trying
   to access global variable.  */

static int
gen_call (unsigned char *buf, CORE_ADDR fn)
{
  int i = 0;

  /* Must be called by r12 for caller to calculate TOC address. */
  i += gen_limm (buf + i, 12, fn);
  i += GEN_MTSPR (buf + i, 12, 9);		/* mtctr  r12 */
  i += put_i32 (buf + i, 0x4e800421);		/* bctrl */

  return i;
}

/* Implement supports_tracepoints hook of target_ops.
   Always return true.  */

static int
ppc_supports_tracepoints (void)
{
  return 1;
}

/* Implement install_fast_tracepoint_jump_pad of target_ops.
   See target.h for details.  */

static int
ppc_install_fast_tracepoint_jump_pad (CORE_ADDR tpoint, CORE_ADDR tpaddr,
				      CORE_ADDR collector,
				      CORE_ADDR lockaddr,
				      ULONGEST orig_size,
				      CORE_ADDR *jump_entry,
				      CORE_ADDR *trampoline,
				      ULONGEST *trampoline_size,
				      unsigned char *jjump_pad_insn,
				      ULONGEST *jjump_pad_insn_size,
				      CORE_ADDR *adjusted_insn_addr,
				      CORE_ADDR *adjusted_insn_addr_end,
				      char *err)
{
  unsigned char buf[1028];
  int i, j, offset;
  CORE_ADDR buildaddr = *jump_entry;
#if __PPC64__
  const int rsz = 8;
#else
  const int rsz = 4;
#endif
  const int frame_size = (((37 * rsz) + 112) + 0xf) & ~0xf;

  /* Stack frame layout for this jump pad,

     High	CTR   -8(sp)
		LR   -16(sp)
		XER
		CR
		R31
		R29
		...
		R1
		R0
     Low	PC/<tpaddr>

     The code flow of thie jump pad,

     1. Save GPR and SPR
     3. Adjust SP
     4. Prepare argument
     5. Call gdb_collector
     6. Restore SP
     7. Restore GPR and SPR
     8. Build a jump for back to the program
     9. Copy/relocate original instruction
    10. Build a jump for replacing orignal instruction.  */

  i = 0;
  for (j = 0; j < 32; j++)
    i += GEN_STORE (buf + i, j, 1, (-rsz * 36 + j * rsz));

  /* Save PC<tpaddr>  */
  i += gen_limm (buf + i, 3, tpaddr);
  i += GEN_STORE (buf + i, 3, 1, (-rsz * 37));

  /* Save CR, XER, LR, and CTR.  */
  i += put_i32 (buf + i, 0x7c600026);		/* mfcr   r3 */
  i += GEN_MFSPR (buf + i, 4, 1);		/* mfxer  r4 */
  i += GEN_MFSPR (buf + i, 5, 8);		/* mflr   r5 */
  i += GEN_MFSPR (buf + i, 6, 9);		/* mfctr  r6 */
  i += GEN_STORE (buf + i, 3, 1, -4 * rsz);	/* std    r3, -32(r1) */
  i += GEN_STORE (buf + i, 4, 1, -3 * rsz);	/* std    r4, -24(r1) */
  i += GEN_STORE (buf + i, 5, 1, -2 * rsz);	/* std    r5, -16(r1) */
  i += GEN_STORE (buf + i, 6, 1, -1 * rsz);	/* std    r6, -8(r1) */

  /* Adjust stack pointer.  */
  i += GEN_ADDI (buf + i, 1, 1, -frame_size);	/* subi   r1,r1,FRAME_SIZE */

  /* Setup arguments to collector.  */

  /* Set r4 to collected registers.  */
  i += GEN_ADDI (buf + i, 4, 1, frame_size - rsz * 37);
  /* Set r3 to TPOINT.  */
  i += gen_limm (buf + i, 3, tpoint);

  i += gen_atomic_xchg (buf + i, lockaddr, 0, 1);
  /* Call to collector.  */
  i += gen_call (buf + i, collector);
  i += gen_atomic_xchg (buf + i, lockaddr, 1, 0);

  /* Restore stack and registers.  */
  i += GEN_ADDI (buf + i, 1, 1, frame_size);	/* addi	r1,r1,FRAME_SIZE */
  i += GEN_LOAD (buf + i, 3, 1, -4 * rsz);	/* ld	r3, -32(r1) */
  i += GEN_LOAD (buf + i, 4, 1, -3 * rsz);	/* ld	r4, -24(r1) */
  i += GEN_LOAD (buf + i, 5, 1, -2 * rsz);	/* ld	r5, -16(r1) */
  i += GEN_LOAD (buf + i, 6, 1, -1 * rsz);	/* ld	r6, -8(r1) */
  i += put_i32 (buf + i, 0x7c6ff120);		/* mtcr	r3 */
  i += GEN_MTSPR (buf + i, 4, 1);		/* mtxer  r4 */
  i += GEN_MTSPR (buf + i, 5, 8);		/* mtlr   r5 */
  i += GEN_MTSPR (buf + i, 6, 9);		/* mtctr  r6 */
  for (j = 0; j < 32; j++)
    i += GEN_LOAD (buf + i, j, 1, (-rsz * 36 + j * rsz));

  /* Remember the address for inserting original instruction.
     Patch the instruction later.  */
  *adjusted_insn_addr = buildaddr + i;
  i += 4;

  /* Finally, write a jump back to the program.  */
  offset = (tpaddr + 4) - (buildaddr + i);
  if (offset >= (1 << 26) || offset < -(1 << 26))
    {
      sprintf (err, "E.Jump back from jump pad too far from tracepoint "
		    "(offset 0x%x > 26-bit).", offset);
      return 1;
    }
  /* b <tpaddr+4> */
  i += GEN_B (buf + i, offset);
  write_inferior_memory (buildaddr, buf, i);

  /* Now, insert the original instruction to execute in the jump pad.  */
  *adjusted_insn_addr_end = *adjusted_insn_addr;
  relocate_instruction (adjusted_insn_addr_end, tpaddr);
  /* Verify the relocation size.  */
  if (*adjusted_insn_addr_end - *adjusted_insn_addr != 4)
    {
      sprintf (err, "E.Unexpected instruction length "
		    "when relocate instruction. %d != 4",
		    (int) (*adjusted_insn_addr_end - *adjusted_insn_addr));
      return 1;
    }

  /* The jump pad is now built.  Wire in a jump to our jump pad.  This
     is always done last (by our caller actually), so that we can
     install fast tracepoints with threads running.  This relies on
     the agent's atomic write support.  */
  offset = buildaddr - tpaddr;
  if (offset >= (1 << 25) || offset < -(1 << 25))
    {
      sprintf (err, "E.Jump back from jump pad too far from tracepoint "
		    "(offset 0x%x > 26-bit).", offset);
      return 1;
    }
  /* b <jentry> */
  i += GEN_B (jjump_pad_insn, offset);
  *jjump_pad_insn_size = 4;

  *jump_entry = buildaddr + i;

  gdb_assert (i < sizeof (buf));

  return 0;
}

static int
ppc_get_min_fast_tracepoint_insn_len ()
{
  return 4;
}

enum
{
  /* basic stack frame
     + room for callee saved registers
     + initial bytecode execution stack  */
  bc_framesz = (48 + 8 * 8) + (4 * 8) + 64,
};

static void
ppc64_emit_prologue (void)
{
  /* r31 is the frame-base for restoring stack-pointer.
     r30 is the stack-pointer for bytecode machine.
	 It should point to next-empty, so we can use LDU for pop.
     r3  is used for cache of TOP value.  It is the first argument,
	 pointer to CTX.
     r4  is the second argument, pointer to the result.  */

  unsigned char buf[10 * 4];
  int i = 0;

  i += GEN_MFSPR (buf, 0, 8);		/* mflr	r0 */
  i += GEN_STD (buf + i, 0, 1, 16);	/* std	r0, 16(r1) */
  i += GEN_STD (buf + i, 31, 1, -8);	/* std	r31, -8(r1) */
  i += GEN_STD (buf + i, 30, 1, -16);	/* std	r30, -16(r1) */
  i += GEN_STD (buf + i, 4, 1, -24);	/* std	r4, -24(r1) */
  i += GEN_STD (buf + i, 3, 1, -32);	/* std	r3, -32(r1) */
  i += GEN_ADDI (buf + i, 30, 1, -40);	/* addi	r30, r1, -40 */
  i += GEN_LI (buf + i, 3, 0);		/* li	r3, 0 */
					/* stdu	r1, -(frame_size)(r1) */
  i += GEN_STDU (buf + i, 1, 1, -bc_framesz);
  i += GEN_MR (buf + i, 31, 1);		/* mr	r31, r1 */

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
  gdb_assert (i <= sizeof (buf));
}


static void
ppc64_emit_epilogue (void)
{
  unsigned char buf[9 * 4];
  int i = 0;

  /* Restore SP.			add	r1, r31, frame_size */
  i += GEN_ADDI (buf, 1, 31, bc_framesz);

  /* *result = $r3  */
  i += GEN_LD (buf + i, 4, 1, -24);	/* ld	r4, -24(r1) */
  i += GEN_STD (buf + i, 3, 4, 0);	/* std	r3, 0(r4) */
  /* Return 0 for no-error.  */
  i += GEN_LI (buf + i, 3, 0);		/* li	r3, 0 */
  i += GEN_LD (buf + i, 0, 1, 16);	/* ld	r0, 16(r1) */
  i += GEN_LD (buf + i, 31, 1, -8);	/* ld	r31, -8(r1) */
  i += GEN_LD (buf + i, 30, 1, -16);	/* ld	r30, -16(r1) */
  i += GEN_MTSPR (buf + i, 0, 8);	/* mtlr	r0 */
  i += put_i32 (buf + i, 0x4e800020);	/* blr */

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
  gdb_assert (i <= sizeof (buf));
}

static void
ppc64_emit_add (void)
{
  EMIT_ASM (ppc64_add,
	    "ldu  4, 8(30)\n\t"
	    "add  3, 4, 3\n\t");
}

static void
ppc64_emit_sub (void)
{
  EMIT_ASM (ppc64_sub,
	    "ldu  4, 8(30)\n\t"
	    "sub  3, 4, 3\n\t");
}

static void
ppc64_emit_mul (void)
{
  EMIT_ASM (ppc64_mul,
	    "ldu    4, 8(30)\n\t"
	    "mulld  3, 4, 3\n\t");
}

static void
ppc64_emit_lsh (void)
{
  EMIT_ASM (ppc64_lsh,
	    "ldu  4, 8(30)\n\t"
	    "sld  3, 4, 3\n\t");
}

static void
ppc64_emit_rsh_signed (void)
{
  EMIT_ASM (ppc64_rsha,
	    "ldu   4, 8(30)\n\t"
	    "srad  3, 4, 3\n\t");
}

static void
ppc64_emit_rsh_unsigned (void)
{
  EMIT_ASM (ppc64_rshl,
	    "ldu  4, 8(30)\n\t"
	    "srd  3, 4, 3\n\t");
}

static void
ppc64_emit_ext (int arg)
{
  switch (arg)
    {
    case 8:
      EMIT_ASM (ppc64_ext8, "extsb  3, 3\n\t");
      break;
    case 16:
      EMIT_ASM (ppc64_ext16, "extsh  3, 3\n\t");
      break;
    case 32:
      EMIT_ASM (ppc64_ext32, "extsw  3, 3\n\t");
      break;
    default:
      emit_error = 1;
    }
}

static void
ppc64_emit_zero_ext (int arg)
{
  switch (arg)
    {
    case 8:
      EMIT_ASM (ppc64_zext8, "rldicl 3,3,0,56\n\t");
      break;
    case 16:
      EMIT_ASM (ppc64_zext16, "rldicl 3,3,0,48\n\t");
      break;
    case 32:
      EMIT_ASM (ppc64_zext32, "rldicl 3,3,0,32\n\t");
      break;
    default:
      emit_error = 1;
    }
}

static void
ppc64_emit_log_not (void)
{
  EMIT_ASM (ppc64_log_not,
	    "cntlzd  3, 3\n\t"
	    "srdi    3, 3, 6\n\t");
}

static void
ppc64_emit_bit_and (void)
{
  EMIT_ASM (ppc64_bit_and,
	    "ldu  4, 8(30)\n\t"
	    "and  3, 4, 3\n\t");
}

static void
ppc64_emit_bit_or (void)
{
  EMIT_ASM (ppc64_bit_or,
	    "ldu  4, 8(30)\n\t"
	    "or   3, 4, 3\n\t");
}

static void
ppc64_emit_bit_xor (void)
{
  EMIT_ASM (ppc64_bit_xor,
	    "ldu  4, 8(30)\n\t"
	    "xor  3, 4, 3\n\t");
}

static void
ppc64_emit_bit_not (void)
{
  EMIT_ASM (ppc64_bit_not,
	    "nor  3, 3, 3\n\t");
}

static void
ppc64_emit_equal (void)
{
  EMIT_ASM (ppc64_equal,
	    "ldu     4, 8(30)\n\t"
	    "xor     3, 3, 4\n\t"
	    "cntlzd  3, 3\n\t"
	    "srdi    3, 3, 6\n\t");
}

static void
ppc64_emit_less_signed (void)
{
  EMIT_ASM (ppc64_less_signed,
	    "ldu     4, 8(30)\n\t"
	    "cmpd    7, 3, 4\n\t"
	    "mfocrf  3, 1\n\t"
	    "rlwinm  3, 3, 29, 31, 31\n\t");
}

static void
ppc64_emit_less_unsigned (void)
{
  EMIT_ASM (ppc64_less_unsigned,
	    "ldu     4, 8(30)\n\t"
	    "cmpld   7, 3, 4\n\t"
	    "mfocrf  3, 1\n\t"
	    "rlwinm  3, 3, 29, 31, 31\n\t");
}

static void
ppc64_emit_ref (int size)
{
  switch (size)
    {
    case 1:
      EMIT_ASM (ppc64_ref8, "lbz   3, 0(3)\n\t");
      break;
    case 2:
      EMIT_ASM (ppc64_ref16, "lhz   3, 0(3)\n\t");
      break;
    case 4:
      EMIT_ASM (ppc64_ref32, "lwz   3, 0(3)\n\t");
      break;
    case 8:
      EMIT_ASM (ppc64_ref64, "ld    3, 0(3)\n\t");
      break;
    }
}

static void
ppc64_emit_if_goto (int *offset_p, int *size_p)
{
  unsigned char buf[4 * 4];
  int i = 0;

  i += GEN_MR (buf + i, 4, 3);		/* mr    r4, r3 */
  i += GEN_LDU (buf + i, 3, 30, 8);	/* ldu   r3, 8(r30) */
  i += GEN_CMPDI (buf + i, 4, 0);	/* cmpdi cr7, r4, 0 */
  i += GEN_BNE (buf + i, 0);		/* bne   cr7, <addr14> */

  if (offset_p)
    *offset_p = 12;
  if (size_p)
    *size_p = 14;

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

static void
ppc64_emit_goto (int *offset_p, int *size_p)
{
  unsigned char buf[4];
  int i = 0;

  i += GEN_B (buf, 0);			/* b    <addr24> */

  if (offset_p)
    *offset_p = 0;
  if (size_p)
    *size_p = 24;

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

static void
ppc64_emit_const (LONGEST num)
{
  unsigned char buf[5 * 4];
  int i = 0;

  i += gen_limm (buf + i, 3, num);

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

static void
ppc64_emit_reg (int reg)
{
  unsigned char buf[8 * 8];
  int i = 0;

  i += GEN_MR (buf, 3, reg);	/* mr	r3, reg */
  i += gen_call (buf, get_raw_reg_func_addr ());

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

static void
ppc64_emit_pop (void)
{
  unsigned char buf[4];
  int i = 0;

  i += GEN_LDU (buf, 3, 30, 8);	/* ldu	r3, 8(r30) */

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

static void
ppc64_emit_stack_flush (void)
{
  unsigned char buf[8 * 4];
  int i = 0;

  /* Make bytecode stack is big enought.  Expand as need.  */

  /* addi	r4, r30, -(112 + 8)
     cmpd	cr7, r4, r1
     bgt	1f
   - ld		r4, 0(r1)
   | addi	r1, r1, -64
   \ st		r4, 0(r1)
  1: st		r3, 0(r30)
     addi	r30, r30, -8 */

  i += GEN_ADDI (buf + i, 4, 30, -(112 + 8));
  i += GEN_CMPD (buf + i, 4, 1);
  i += GEN_BGT (buf + i, 16);
  {
    /* Expand stack.  */
    i += GEN_LD (buf + i, 4, 1, 0);
    i += GEN_ADDI (buf + i, 1, 1, -64);
    i += GEN_STD (buf + i, 4, 1, 0);
  }
  /* Push TOP in stack.  */
  i += GEN_STD (buf + i, 3, 30, 0);
  i += GEN_ADDI (buf + i, 30, 30, -8);

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

static void
ppc64_emit_swap (void)
{
  EMIT_ASM (ppc64_swap,
	    "ld	4, 8(30)\n\t"
	    "std	3, 8(30)\n\t"
	    "mr	3, 4\n\t");
}

static void
ppc64_emit_stack_adjust (int n)
{
  unsigned char buf[4];
  int i = 0;

  i += GEN_ADDI (buf, 30, 30, n << 3);	/* addi	r30, r30, (n << 3) */

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

static void
ppc64_emit_call (CORE_ADDR fn)
{
  unsigned char buf[8 * 4];
  int i = 0;

  i += gen_call (buf + i, fn);

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

/* FN's prototype is `LONGEST(*fn)(int)'.  */

static void
ppc64_emit_int_call_1 (CORE_ADDR fn, int arg1)
{
  unsigned char buf[8 * 4];
  int i = 0;

  /* Setup argument.  arg1 is a 16-bit value.  */
  i += GEN_LI (buf, 3, arg1);		/* li	r3, arg1 */
  i += gen_call (buf + i, fn);

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

/* FN's prototype is `void(*fn)(int,LONGEST)'.  */

static void
ppc64_emit_void_call_2 (CORE_ADDR fn, int arg1)
{
  unsigned char buf[12 * 4];
  int i = 0;

  /* Save TOP */
  i += GEN_STD (buf, 3, 31, bc_framesz + 24);

  /* Setup argument.  arg1 is a 16-bit value.  */
  i += GEN_MR (buf + i, 3, 4);		/* mr	r4, r3 */
  i += GEN_LI (buf + i, 3, arg1);	/* li	r3, arg1 */
  i += gen_call (buf + i, fn);

  /* Restore TOP */
  i += GEN_LD (buf, 3, 31, bc_framesz + 24);

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

void
ppc64_emit_eq_goto (int *offset_p, int *size_p)
{
  unsigned char buf[4 * 4];
  int i = 0;

  i += GEN_LDU (buf + i, 4, 30, 8);	/* ldu	r4, 8(r30) */
  i += GEN_CMPD (buf + i, 3, 4);	/* cmpd	cr7, r3, r4 */
  i += GEN_BEQ (buf + i, 0);		/* beq	cr7, <addr14> */
  /* Cache top.  */
  i += GEN_LDU (buf + i, 3, 30, 8);	/* ldu	r3, 8(r30) */

  if (offset_p)
    *offset_p = 8;
  if (size_p)
    *size_p = 14;

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

void
ppc64_emit_ne_goto (int *offset_p, int *size_p)
{
  unsigned char buf[4 * 4];
  int i = 0;

  i += GEN_LDU (buf + i, 4, 30, 8);	/* ldu	r4, 8(r30) */
  i += GEN_CMPD (buf + i, 3, 4);	/* cmpd	cr7, r3, r4 */
  i += GEN_BNE (buf + i, 0);		/* bne	cr7, <addr14> */
  /* Cache top.  */
  i += GEN_LDU (buf + i, 3, 30, 8);	/* ldu	r3, 8(r30) */

  if (offset_p)
    *offset_p = 8;
  if (size_p)
    *size_p = 14;

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

void
ppc64_emit_lt_goto (int *offset_p, int *size_p)
{
  unsigned char buf[4 * 4];
  int i = 0;

  i += GEN_LDU (buf + i, 4, 30, 8);	/* ldu	r4, 8(r30) */
  i += GEN_CMPD (buf + i, 3, 4);	/* cmpd	cr7, r3, r4 */
  i += GEN_BLT (buf + i, 0);		/* blt	cr7, <addr14> */
  /* Cache top.  */
  i += GEN_LDU (buf + i, 3, 30, 8);	/* ldu	r3, 8(r30) */

  if (offset_p)
    *offset_p = 8;
  if (size_p)
    *size_p = 14;

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

void
ppc64_emit_le_goto (int *offset_p, int *size_p)
{
  unsigned char buf[4 * 4];
  int i = 0;

  i += GEN_LDU (buf + i, 4, 30, 8);	/* ldu	r4, 8(r30) */
  i += GEN_CMPD (buf + i, 3, 4);	/* cmpd	cr7, r3, r4 */
  i += GEN_BLE (buf + i, 0);		/* ble	cr7, <addr14> */
  /* Cache top.  */
  i += GEN_LDU (buf + i, 3, 30, 8);	/* ldu	r3, 8(r30) */

  if (offset_p)
    *offset_p = 8;
  if (size_p)
    *size_p = 14;

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

void
ppc64_emit_gt_goto (int *offset_p, int *size_p)
{
  unsigned char buf[4 * 4];
  int i = 0;

  i += GEN_LDU (buf + i, 4, 30, 8);	/* ldu	r4, 8(r30) */
  i += GEN_CMPD (buf + i, 3, 4);	/* cmpd	cr7, r3, r4 */
  i += GEN_BGT (buf + i, 0);		/* bgt	cr7, <addr14> */
  /* Cache top.  */
  i += GEN_LDU (buf + i, 3, 30, 8);	/* ldu	r3, 8(r30) */

  if (offset_p)
    *offset_p = 8;
  if (size_p)
    *size_p = 14;

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

void
ppc64_emit_ge_goto (int *offset_p, int *size_p)
{
  unsigned char buf[4 * 4];
  int i = 0;

  i += GEN_LDU (buf + i, 4, 30, 8);	/* ldu	r4, 8(r30) */
  i += GEN_CMPD (buf + i, 3, 4);	/* cmpd	cr7, r3, r4 */
  i += GEN_BGE (buf + i, 0);		/* bge	cr7, <addr14> */
  /* Cache top.  */
  i += GEN_LDU (buf + i, 3, 30, 8);	/* ldu	r3, 8(r30) */

  if (offset_p)
    *offset_p = 8;
  if (size_p)
    *size_p = 14;

  write_inferior_memory (current_insn_ptr, buf, i);
  current_insn_ptr += i;
}

static void
ppc_write_goto_address (CORE_ADDR from, CORE_ADDR to, int size)
{
  int rel = to - from;
  uint32_t insn;
  int opcd;
  unsigned char buf[4];

  read_inferior_memory (from, buf, 4);
  insn = get_i32 (buf);
  opcd = (insn >> 26) & 0x3f;

  switch (size)
    {
    case 14:
      if (opcd != 16)
	emit_error = 1;
      insn |= (rel & 0xfffc);
      break;
    case 24:
      if (opcd != 18)
	emit_error = 1;
      insn |= (rel & 0x3fffffc);
      break;
    }

  put_i32 (buf, insn);
  write_inferior_memory (from, buf, 4);
}

struct emit_ops ppc64_emit_ops_vector =
{
  ppc64_emit_prologue,
  ppc64_emit_epilogue,
  ppc64_emit_add,
  ppc64_emit_sub,
  ppc64_emit_mul,
  ppc64_emit_lsh,
  ppc64_emit_rsh_signed,
  ppc64_emit_rsh_unsigned,
  ppc64_emit_ext,
  ppc64_emit_log_not,
  ppc64_emit_bit_and,
  ppc64_emit_bit_or,
  ppc64_emit_bit_xor,
  ppc64_emit_bit_not,
  ppc64_emit_equal,
  ppc64_emit_less_signed,
  ppc64_emit_less_unsigned,
  ppc64_emit_ref,
  ppc64_emit_if_goto,
  ppc64_emit_goto,
  ppc_write_goto_address,
  ppc64_emit_const,
  ppc64_emit_call,
  ppc64_emit_reg,
  ppc64_emit_pop,
  ppc64_emit_stack_flush,
  ppc64_emit_zero_ext,
  ppc64_emit_swap,
  ppc64_emit_stack_adjust,
  ppc64_emit_int_call_1,
  ppc64_emit_void_call_2,
  ppc64_emit_eq_goto,
  ppc64_emit_ne_goto,
  ppc64_emit_lt_goto,
  ppc64_emit_le_goto,
  ppc64_emit_gt_goto,
  ppc64_emit_ge_goto
};

__attribute__ ((unused))
static struct emit_ops *
ppc_emit_ops (void)
{
#if __PPC64__
  return &ppc64_emit_ops_vector;
#else
  return NULL;
#endif
}

static int
ppc_supports_range_stepping (void)
{
  return 1;
}

/* Provide only a fill function for the general register set.  ps_lgetregs
   will use this for NPTL support.  */

static void ppc_fill_gregset (struct regcache *regcache, void *buf)
{
  int i;

  for (i = 0; i < 32; i++)
    ppc_collect_ptrace_register (regcache, i, (char *) buf + ppc_regmap[i]);

  for (i = 64; i < 70; i++)
    ppc_collect_ptrace_register (regcache, i, (char *) buf + ppc_regmap[i]);

  for (i = 71; i < 73; i++)
    ppc_collect_ptrace_register (regcache, i, (char *) buf + ppc_regmap[i]);
}

#define SIZEOF_VSXREGS 32*8

static void
ppc_fill_vsxregset (struct regcache *regcache, void *buf)
{
  int i, base;
  char *regset = buf;

  if (!(ppc_hwcap & PPC_FEATURE_HAS_VSX))
    return;

  base = find_regno (regcache->tdesc, "vs0h");
  for (i = 0; i < 32; i++)
    collect_register (regcache, base + i, &regset[i * 8]);
}

static void
ppc_store_vsxregset (struct regcache *regcache, const void *buf)
{
  int i, base;
  const char *regset = buf;

  if (!(ppc_hwcap & PPC_FEATURE_HAS_VSX))
    return;

  base = find_regno (regcache->tdesc, "vs0h");
  for (i = 0; i < 32; i++)
    supply_register (regcache, base + i, &regset[i * 8]);
}

#define SIZEOF_VRREGS 33*16+4

static void
ppc_fill_vrregset (struct regcache *regcache, void *buf)
{
  int i, base;
  char *regset = buf;

  if (!(ppc_hwcap & PPC_FEATURE_HAS_ALTIVEC))
    return;

  base = find_regno (regcache->tdesc, "vr0");
  for (i = 0; i < 32; i++)
    collect_register (regcache, base + i, &regset[i * 16]);

  collect_register_by_name (regcache, "vscr", &regset[32 * 16 + 12]);
  collect_register_by_name (regcache, "vrsave", &regset[33 * 16]);
}

static void
ppc_store_vrregset (struct regcache *regcache, const void *buf)
{
  int i, base;
  const char *regset = buf;

  if (!(ppc_hwcap & PPC_FEATURE_HAS_ALTIVEC))
    return;

  base = find_regno (regcache->tdesc, "vr0");
  for (i = 0; i < 32; i++)
    supply_register (regcache, base + i, &regset[i * 16]);

  supply_register_by_name (regcache, "vscr", &regset[32 * 16 + 12]);
  supply_register_by_name (regcache, "vrsave", &regset[33 * 16]);
}

struct gdb_evrregset_t
{
  unsigned long evr[32];
  unsigned long long acc;
  unsigned long spefscr;
};

static void
ppc_fill_evrregset (struct regcache *regcache, void *buf)
{
  int i, ev0;
  struct gdb_evrregset_t *regset = buf;

  if (!(ppc_hwcap & PPC_FEATURE_HAS_SPE))
    return;

  ev0 = find_regno (regcache->tdesc, "ev0h");
  for (i = 0; i < 32; i++)
    collect_register (regcache, ev0 + i, &regset->evr[i]);

  collect_register_by_name (regcache, "acc", &regset->acc);
  collect_register_by_name (regcache, "spefscr", &regset->spefscr);
}

static void
ppc_store_evrregset (struct regcache *regcache, const void *buf)
{
  int i, ev0;
  const struct gdb_evrregset_t *regset = buf;

  if (!(ppc_hwcap & PPC_FEATURE_HAS_SPE))
    return;

  ev0 = find_regno (regcache->tdesc, "ev0h");
  for (i = 0; i < 32; i++)
    supply_register (regcache, ev0 + i, &regset->evr[i]);

  supply_register_by_name (regcache, "acc", &regset->acc);
  supply_register_by_name (regcache, "spefscr", &regset->spefscr);
}

static struct regset_info ppc_regsets[] = {
  /* List the extra register sets before GENERAL_REGS.  That way we will
     fetch them every time, but still fall back to PTRACE_PEEKUSER for the
     general registers.  Some kernels support these, but not the newer
     PPC_PTRACE_GETREGS.  */
  { PTRACE_GETVSXREGS, PTRACE_SETVSXREGS, 0, SIZEOF_VSXREGS, EXTENDED_REGS,
  ppc_fill_vsxregset, ppc_store_vsxregset },
  { PTRACE_GETVRREGS, PTRACE_SETVRREGS, 0, SIZEOF_VRREGS, EXTENDED_REGS,
    ppc_fill_vrregset, ppc_store_vrregset },
  { PTRACE_GETEVRREGS, PTRACE_SETEVRREGS, 0, 32 * 4 + 8 + 4, EXTENDED_REGS,
    ppc_fill_evrregset, ppc_store_evrregset },
  { 0, 0, 0, 0, GENERAL_REGS, ppc_fill_gregset, NULL },
  { 0, 0, 0, -1, -1, NULL, NULL }
};

static struct usrregs_info ppc_usrregs_info =
  {
    ppc_num_regs,
    ppc_regmap,
  };

static struct regsets_info ppc_regsets_info =
  {
    ppc_regsets, /* regsets */
    0, /* num_regsets */
    NULL, /* disabled_regsets */
  };

static struct regs_info regs_info =
  {
    NULL, /* regset_bitmap */
    &ppc_usrregs_info,
    &ppc_regsets_info
  };

static const struct regs_info *
ppc_regs_info (void)
{
  return &regs_info;
}

struct linux_target_ops the_low_target = {
  ppc_arch_setup,
  ppc_regs_info,
  ppc_cannot_fetch_register,
  ppc_cannot_store_register,
  NULL, /* fetch_register */
  ppc_get_pc,
  ppc_set_pc,
  (const unsigned char *) &ppc_breakpoint,
  ppc_breakpoint_len,
  NULL, /* breakpoint_reinsert_addr */
  0, /* decr_pc_after_break */
  ppc_breakpoint_at,
  ppc_supports_z_point_type, /* supports_z_point_type */
  ppc_insert_point,
  ppc_remove_point,
  NULL, /* stopped_by_watchpoint */
  NULL, /* stopped_data_address */
  ppc_collect_ptrace_register,
  ppc_supply_ptrace_register,
  NULL, /* siginfo_fixup */
  NULL, /* linux_new_process */
  NULL, /* linux_new_thread */
  NULL, /* linux_prepare_to_resume */
  NULL, /* linux_process_qsupported */
  ppc_supports_tracepoints,
  NULL, /* get_thread_area */
  ppc_install_fast_tracepoint_jump_pad,
#if __PPC64__
  ppc_emit_ops,
#else
  NULL,
#endif
  ppc_get_min_fast_tracepoint_insn_len,
  ppc_supports_range_stepping,
};

void
initialize_low_arch (void)
{
  /* Initialize the Linux target descriptions.  */

  init_registers_powerpc_32l ();
  init_registers_powerpc_altivec32l ();
  init_registers_powerpc_cell32l ();
  init_registers_powerpc_vsx32l ();
  init_registers_powerpc_isa205_32l ();
  init_registers_powerpc_isa205_altivec32l ();
  init_registers_powerpc_isa205_vsx32l ();
  init_registers_powerpc_e500l ();
  init_registers_powerpc_64l ();
  init_registers_powerpc_altivec64l ();
  init_registers_powerpc_cell64l ();
  init_registers_powerpc_vsx64l ();
  init_registers_powerpc_isa205_64l ();
  init_registers_powerpc_isa205_altivec64l ();
  init_registers_powerpc_isa205_vsx64l ();

  initialize_regsets_info (&ppc_regsets_info);
}
