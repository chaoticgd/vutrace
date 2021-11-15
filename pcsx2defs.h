/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2010  PCSX2 Dev Team
 *  Copyright (C) 2020 chaoticgd
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PCSX2_DEFS
#define PCSX2_DEFS

#include <stdint.h>

typedef int64_t s64;
typedef uint64_t u64;
typedef int32_t s32;
typedef uint32_t u32;
typedef int16_t s16;
typedef uint16_t u16;
typedef int8_t s8;
typedef uint8_t u8;
typedef unsigned int uint;

static const uint VU1_MEMSIZE	= 0x4000;		// 16kb
static const uint VU1_PROGSIZE	= 0x4000;		// 16kb
static const uint TPC = 26;

union VECTOR {
	struct {
		float x,y,z,w;
	} f;
	struct {
		u32 x,y,z,w;
	} i;

	float F[4];

	u64 UD[2];      //128 bits
	s64 SD[2];
	u32 UL[4];
	s32 SL[4];
	u16 US[8];
	s16 SS[8];
	u8  UC[16];
	s8  SC[16];
};

struct REG_VI {
	union {
		float F;
		s32   SL;
		u32	  UL;
		s16   SS[2];
		u16   US[2];
		s8    SC[4];
		u8    UC[4];
	};
	u32 padding[3]; // needs padding to make them 128bit; VU0 maps VU1's VI regs as 128bits to addr 0x4xx0 in
					// VU0 mem, with only lower 16 bits valid, and the upper 112bits are hardwired to 0 (cottonvibes)
};

struct fdivPipe {
	int enable;
	REG_VI reg;
	u32 sCycle;
	u32 Cycle;
	u32 statusflag;
};

struct efuPipe {
	int enable;
	REG_VI reg;
	u32 sCycle;
	u32 Cycle;
};

struct fmacPipe {
	int enable;
	int reg;
	int xyzw;
	u32 sCycle;
	u32 Cycle;
	u32 macflag;
	u32 statusflag;
	u32 clipflag;
};

struct ialuPipe {
	int enable;
	int reg;
	u32 sCycle;
	u32 Cycle;
};

struct VURegs
{
	VECTOR	VF[32]; // VF and VI need to be first in this struct for proper mapping
	REG_VI	VI[32]; // needs to be 128bit x 32 (cottonvibes)

	VECTOR ACC;
	REG_VI q;
	REG_VI p;

	uint idx;		// VU index (0 or 1)

	// flags/cycle are needed by VIF dma code, so they have to be here (for now)
	// We may replace these by accessors in the future, if merited.
	u32 cycle;
	u32 flags;

	// Current opcode being interpreted or recompiled (this var is used by Interps and superVU
	// but not microVU.  Would like to have it local to their respective classes... someday)
	u32 code;

	// branch/branchpc are used by interpreter only, but making them local to the interpreter
	// classes requires considerable code refactoring.  Maybe later. >_<
	u32 branch;
	u32 branchpc;
	u32 delaybranchpc;
	bool takedelaybranch;

	// MAC/Status flags -- these are used by interpreters and superVU, but are kind of hacky
	// and shouldn't be relied on for any useful/valid info.  Would like to move them out of
	// this struct eventually.
	u32 macflag;
	u32 statusflag;
	u32 clipflag;

	u32 Mem;
	u32 Micro;

	u32 ebit;

	u8 VIBackupCycles;
	u32 VIOldValue;
	u32 VIRegNumber;

	fmacPipe fmac[8];
	fdivPipe fdiv;
	efuPipe efu;
	ialuPipe ialu[8];
};

#endif
