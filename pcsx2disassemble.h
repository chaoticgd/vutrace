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

// Adapted from microVU.

#ifndef PCSX2_DISASSEMBLE
#define PCSX2_DISASSEMBLE

#include <string>
#include <stdint.h>

#include "pcsx2defs.h"

#define mVUop(mnenomic) void mVU_##mnenomic (std::string &result, uint32_t insn, uint32_t pc)
#define mVUlog( ...) \
	char buffer[1024]; \
	memset(buffer, 0, 1024); \
	sprintf(buffer, __VA_ARGS__); \
	result += std::string(buffer);

void mVUunknown(std::string &result, uint32_t insn, uint32_t pc)
{
	result = "<BAD INSTRUCTION>";
}

#define _Ft_ ((insn >> 16) & 0x1F)  // The ft part of the instruction register
#define _Fs_ ((insn >> 11) & 0x1F)  // The fs part of the instruction register
#define _Fd_ ((insn >>  6) & 0x1F)  // The fd part of the instruction register

#define _It_ ((insn >> 16) & 0xF)   // The it part of the instruction register
#define _Is_ ((insn >> 11) & 0xF)   // The is part of the instruction register
#define _Id_ ((insn >>  6) & 0xF)   // The id part of the instruction register

#define _X	 ((insn>>24) & 0x1)
#define _Y	 ((insn>>23) & 0x1)
#define _Z	 ((insn>>22) & 0x1)
#define _W	 ((insn>>21) & 0x1)

#define _X_Y_Z_W	(((insn >> 21 ) & 0xF))
#define _XYZW_SS	(_X+_Y+_Z+_W==1)
#define _XYZW_SS2	(_XYZW_SS && (_X_Y_Z_W != 8))
#define _XYZW_PS	(_X_Y_Z_W == 0xf)
#define _XYZWss(x)	((x==8) || (x==4) || (x==2) || (x==1))

#define _bc_	 (insn & 0x3)
#define _bc_x	((insn & 0x3) == 0)
#define _bc_y	((insn & 0x3) == 1)
#define _bc_z	((insn & 0x3) == 2)
#define _bc_w	((insn & 0x3) == 3)

#define _Fsf_	((insn >> 21) & 0x03)
#define _Ftf_	((insn >> 23) & 0x03)

#define _Imm5_	((s16) (((insn & 0x400) ? 0xfff0 : 0) | ((insn >> 6) & 0xf)))
#define _Imm11_	((s32)  ((insn & 0x400) ? (0xfffffc00 |  (insn & 0x3ff)) : (insn & 0x3ff)))
#define _Imm12_	((u32)((((insn >> 21) & 0x1) << 11)   |  (insn & 0x7ff)))
#define _Imm15_	((u32) (((insn >> 10) & 0x7800)       |  (insn & 0x7ff)))
#define _Imm24_	((u32)   (insn & 0xffffff))

#define _Fsf_String	 ((_Fsf_ == 3) ? "w" : ((_Fsf_ == 2) ? "z" : ((_Fsf_ == 1) ? "y" : "x")))
#define _Ftf_String	 ((_Ftf_ == 3) ? "w" : ((_Ftf_ == 2) ? "z" : ((_Ftf_ == 1) ? "y" : "x")))
#define xyzwStr(x,s) (_X_Y_Z_W == x) ? s :
#define _XYZW_String (xyzwStr(1, "w") (xyzwStr(2, "z") (xyzwStr(3, "zw") (xyzwStr(4, "y") (xyzwStr(5, "yw") (xyzwStr(6, "yz") (xyzwStr(7, "yzw") (xyzwStr(8, "x") (xyzwStr(9, "xw") (xyzwStr(10, "xz") (xyzwStr(11, "xzw") (xyzwStr(12, "xy") (xyzwStr(13, "xyw") (xyzwStr(14, "xyz") "xyzw"))))))))))))))
#define _BC_String	 (_bc_x ? "x" : (_bc_y ? "y" : (_bc_z ? "z" : "w")))
#define mVUlogFtFs() { mVUlog(".%s vf%02d, vf%02d", _XYZW_String, _Ft_, _Fs_); }
#define mVUlogFd()	 { mVUlog(".%s vf%02d, vf%02d", _XYZW_String, _Fd_, _Fs_); }
#define mVUlogACC()	 { mVUlog(".%s ACC, vf%02d", _XYZW_String, _Fs_); }
#define mVUlogFt()	 { mVUlog(", vf%02d", _Ft_); }
#define mVUlogBC()	 { mVUlog(", vf%02d%s", _Ft_, _BC_String); }
#define mVUlogI()	 { mVUlog(", I"); }
#define mVUlogQ()	 { mVUlog(", Q"); }
#define mVUlogCLIP() { mVUlog("w.xyz vf%02d, vf%02dw", _Fs_, _Ft_); }


static inline u32 branchAddr(uint32_t insn, uint32_t pc)
{
	return ((((pc / 4 + 2) + (_Imm11_ * 2)) & (VU1_PROGSIZE / 4 - 1)) * 4);
}

// ======== LOWER INSTRUCTIONS ========

mVUop(DIV) { mVUlog("DIV Q, vf%02d%s, vf%02d%s", _Fs_, _Fsf_String, _Ft_, _Ftf_String); }
mVUop(SQRT) { mVUlog("SQRT Q, vf%02d%s", _Ft_, _Ftf_String); }
mVUop(RSQRT) { mVUlog("RSQRT Q, vf%02d%s, vf%02d%s", _Fs_, _Fsf_String, _Ft_, _Ftf_String); }
mVUop(EATAN) { mVUlog("EATAN P"); }
mVUop(EATANxy) { mVUlog("EATANxy P"); }
mVUop(EATANxz) { mVUlog("EATANxz P"); }
mVUop(EEXP) { mVUlog("EEXP P"); }
mVUop(ELENG) { mVUlog("ELENG P"); }
mVUop(ERCPR) { mVUlog("ERCPR P"); }
mVUop(ERLENG) { mVUlog("ERLENG P"); }
mVUop(ERSADD) { mVUlog("ERSADD P"); }
mVUop(ERSQRT) { mVUlog("ERSQRT P"); }
mVUop(ESADD) { mVUlog("ESADD P"); }
mVUop(ESIN) { mVUlog("ESIN P"); }
mVUop(ESQRT) { mVUlog("ESQRT P"); }
mVUop(ESUM) { mVUlog("ESUM P"); }
mVUop(FCAND) { mVUlog("FCAND vi01, $%x", _Imm24_); }
mVUop(FCEQ) { mVUlog("FCEQ vi01, $%x", _Imm24_); }
mVUop(FCGET) { mVUlog("FCGET vi%02d", _Ft_);	   }
mVUop(FCOR) { mVUlog("FCOR vi01, $%x", _Imm24_); }
mVUop(FCSET) { mVUlog("FCSET $%x", _Imm24_); }
mVUop(FMAND) { mVUlog("FMAND vi%02d, vi%02d", _Ft_, _Fs_); }
mVUop(FMEQ) { mVUlog("FMEQ vi%02d, vi%02d", _Ft_, _Fs_); }
mVUop(FMOR) { mVUlog("FMOR vi%02d, vi%02d", _Ft_, _Fs_); }
mVUop(FSAND) { mVUlog("FSAND vi%02d, $%x", _Ft_, _Imm12_); }
mVUop(FSOR) { mVUlog("FSOR vi%02d, $%x", _Ft_, _Imm12_); }
mVUop(FSEQ) { mVUlog("FSEQ vi%02d, $%x", _Ft_, _Imm12_); }
mVUop(FSSET) { mVUlog("FSSET $%x", _Imm12_); }
mVUop(IADD) { mVUlog("IADD vi%02d, vi%02d, vi%02d", _Fd_, _Fs_, _Ft_); }
mVUop(IADDI) { mVUlog("IADDI vi%02d, vi%02d, %d", _Ft_, _Fs_, _Imm5_); }
mVUop(IADDIU) { mVUlog("IADDIU vi%02d, vi%02d, %d", _Ft_, _Fs_, _Imm15_); }
mVUop(IAND) { mVUlog("IAND vi%02d, vi%02d, vi%02d", _Fd_, _Fs_, _Ft_); }
mVUop(IOR) { mVUlog("IOR vi%02d, vi%02d, vi%02d", _Fd_, _Fs_, _Ft_); }
mVUop(ISUB) { mVUlog("ISUB vi%02d, vi%02d, vi%02d", _Fd_, _Fs_, _Ft_); }
mVUop(ISUBIU) { mVUlog("ISUBIU vi%02d, vi%02d, %d", _Ft_, _Fs_, _Imm15_); }
mVUop(MFIR) { mVUlog("MFIR.%s vf%02d, vi%02d", _XYZW_String, _Ft_, _Fs_); }
mVUop(MFP) { mVUlog("MFP.%s vf%02d, P", _XYZW_String, _Ft_); }
mVUop(MOVE) { mVUlog("MOVE.%s vf%02d, vf%02d", _XYZW_String, _Ft_, _Fs_); }
mVUop(MR32) { mVUlog("MR32.%s vf%02d, vf%02d", _XYZW_String, _Ft_, _Fs_); }
mVUop(MTIR) { mVUlog("MTIR vi%02d, vf%02d%s", _Ft_, _Fs_, _Fsf_String); }
mVUop(ILW) { mVUlog("ILW.%s vi%02d, vi%02d + %d", _XYZW_String, _Ft_, _Fs_, _Imm11_); }
mVUop(ILWR) { mVUlog("ILWR.%s vi%02d, vi%02d", _XYZW_String, _Ft_, _Fs_); }
mVUop(ISW) { mVUlog("ISW.%s vi%02d, vi%02d + %d", _XYZW_String, _Ft_, _Fs_, _Imm11_);  }
mVUop(ISWR) { mVUlog("ISWR.%s vi%02d, vi%02d", _XYZW_String, _Ft_, _Fs_); }
mVUop(LQ) { mVUlog("LQ.%s vf%02d, vi%02d + %d", _XYZW_String, _Ft_, _Fs_, _Imm11_); }
mVUop(LQD) { mVUlog("LQD.%s vf%02d, --vi%02d", _XYZW_String, _Ft_, _Is_); }
mVUop(LQI) { mVUlog("LQI.%s vf%02d, vi%02d++", _XYZW_String, _Ft_, _Fs_); }
mVUop(SQ) { mVUlog("SQ.%s vf%02d, vi%02d + %d", _XYZW_String, _Fs_, _Ft_, _Imm11_); }
mVUop(SQD) { mVUlog("SQD.%s vf%02d, --vi%02d", _XYZW_String, _Fs_, _Ft_); }
mVUop(SQI) { mVUlog("SQI.%s vf%02d, vi%02d++", _XYZW_String, _Fs_, _Ft_); }
mVUop(RINIT) { mVUlog("RINIT R, vf%02d%s", _Fs_, _Fsf_String); }
mVUop(RGET) { mVUlog("RGET.%s vf%02d, R", _XYZW_String, _Ft_); }
mVUop(RNEXT) { mVUlog("RNEXT.%s vf%02d, R", _XYZW_String, _Ft_); }
mVUop(RXOR) { mVUlog("RXOR R, vf%02d%s", _Fs_, _Fsf_String); }
mVUop(WAITP) { mVUlog("WAITP"); }
mVUop(WAITQ) { mVUlog("WAITQ"); }
mVUop(XTOP) { mVUlog("XTOP vi%02d", _Ft_); }
mVUop(XITOP) { mVUlog("XITOP vi%02d", _Ft_); }
mVUop(XGKICK) { mVUlog("XGKICK vi%02d", _Fs_); }
mVUop(B) { mVUlog("B [%04x]", branchAddr(insn, pc)); }
mVUop(BAL) { mVUlog("BAL vi%02d [%04x]", _Ft_, branchAddr(insn, pc)); }
mVUop(IBEQ) { mVUlog("IBEQ vi%02d, vi%02d [%04x]", _Ft_, _Fs_, branchAddr(insn, pc)); }
mVUop(IBGEZ) { mVUlog("IBGEZ vi%02d [%04x]", _Fs_, branchAddr(insn, pc)); }
mVUop(IBGTZ) { mVUlog("IBGTZ vi%02d [%04x]", _Fs_, branchAddr(insn, pc)); }
mVUop(IBLEZ) { mVUlog("IBLEZ vi%02d [%04x]", _Fs_, branchAddr(insn, pc)); }
mVUop(IBLTZ) { mVUlog("IBLTZ vi%02d [%04x]", _Fs_, branchAddr(insn, pc)); }
mVUop(IBNE) { mVUlog("IBNE vi%02d, vi%02d [%04x]", _Ft_, _Fs_, branchAddr(insn, pc)); }
mVUop(JR) { mVUlog("JR [vi%02d]", _Fs_); }
mVUop(JALR) { mVUlog("JALR vi%02d, [vi%02d]", _Ft_, _Fs_); }

typedef void (*Fnptr_mVUrecInst)(std::string &result, uint32_t insn, uint32_t pc);

void mVULowerOP(std::string &result, uint32_t insn, uint32_t pc);

static const Fnptr_mVUrecInst mVULOWER_OPCODE[128] = {
	mVU_LQ		, mVU_SQ		, mVUunknown	, mVUunknown,
	mVU_ILW		, mVU_ISW		, mVUunknown	, mVUunknown,
	mVU_IADDIU	, mVU_ISUBIU	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVU_FCEQ	, mVU_FCSET		, mVU_FCAND		, mVU_FCOR,
	mVU_FSEQ	, mVU_FSSET		, mVU_FSAND		, mVU_FSOR,	
	mVU_FMEQ	, mVUunknown	, mVU_FMAND		, mVU_FMOR,	
	mVU_FCGET	, mVUunknown	, mVUunknown	, mVUunknown,
	mVU_B		, mVU_BAL		, mVUunknown	, mVUunknown,
	mVU_JR		, mVU_JALR		, mVUunknown	, mVUunknown,
	mVU_IBEQ	, mVU_IBNE		, mVUunknown	, mVUunknown,
	mVU_IBLTZ	, mVU_IBGTZ		, mVU_IBLEZ		, mVU_IBGEZ,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVULowerOP	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
};

static const Fnptr_mVUrecInst mVULowerOP_T3_00_OPCODE[32] = {
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVU_MOVE	, mVU_LQI		, mVU_DIV		, mVU_MTIR,	
	mVU_RNEXT	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVU_MFP		, mVU_XTOP		, mVU_XGKICK,
	mVU_ESADD	, mVU_EATANxy	, mVU_ESQRT		, mVU_ESIN,	
};

static const Fnptr_mVUrecInst mVULowerOP_T3_01_OPCODE[32] = {
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVU_MR32	, mVU_SQI		, mVU_SQRT		, mVU_MFIR,	
	mVU_RGET	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVU_XITOP		, mVUunknown,
	mVU_ERSADD	, mVU_EATANxz	, mVU_ERSQRT	, mVU_EATAN,
};

static const Fnptr_mVUrecInst mVULowerOP_T3_10_OPCODE[32] = {
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVU_LQD		, mVU_RSQRT		, mVU_ILWR,	
	mVU_RINIT	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVU_ELENG	, mVU_ESUM		, mVU_ERCPR		, mVU_EEXP,	
};

const Fnptr_mVUrecInst mVULowerOP_T3_11_OPCODE [32] = {
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVU_SQD		, mVU_WAITQ		, mVU_ISWR,	
	mVU_RXOR	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVU_ERLENG	, mVUunknown	, mVU_WAITP		, mVUunknown,
};

void mVULowerOP_T3_00(std::string &result, uint32_t insn, uint32_t pc)
{
	mVULowerOP_T3_00_OPCODE[(insn >> 6) & 0x1f](result, insn, pc);
}

void mVULowerOP_T3_01(std::string &result, uint32_t insn, uint32_t pc)
{
	mVULowerOP_T3_01_OPCODE[(insn >> 6) & 0x1f](result, insn, pc);
}

void mVULowerOP_T3_10(std::string &result, uint32_t insn, uint32_t pc)
{
	mVULowerOP_T3_10_OPCODE[(insn >> 6) & 0x1f](result, insn, pc);
}

void mVULowerOP_T3_11(std::string &result, uint32_t insn, uint32_t pc)
{
	mVULowerOP_T3_11_OPCODE[(insn >> 6) & 0x1f](result, insn, pc);
}

static const Fnptr_mVUrecInst mVULowerOP_OPCODE[64] = {
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVU_IADD	, mVU_ISUB		, mVU_IADDI		, mVUunknown,
	mVU_IAND	, mVU_IOR		, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVULowerOP_T3_00, mVULowerOP_T3_01, mVULowerOP_T3_10, mVULowerOP_T3_11,
};

void mVULowerOP(std::string &result, uint32_t insn, uint32_t pc)
{
	mVULowerOP_OPCODE[insn & 0x3f](result, insn, pc);
}

std::string disassemble_lower(uint32_t insn, uint32_t pc)
{
	std::string result;
	mVULOWER_OPCODE[insn >> 25](result, insn, pc);
	return result;
}

// ======== UPPER INSTRUCTIONS ========

enum microOpcode {
	// Upper Instructions
	opABS, opCLIP, opOPMULA, opOPMSUB, opNOP, 
	opADD, opADDi, opADDq, opADDx, opADDy, opADDz, opADDw, 
	opADDA, opADDAi, opADDAq, opADDAx, opADDAy, opADDAz, opADDAw, 
	opSUB, opSUBi, opSUBq, opSUBx, opSUBy, opSUBz, opSUBw,
	opSUBA, opSUBAi, opSUBAq, opSUBAx, opSUBAy, opSUBAz, opSUBAw, 
	opMUL, opMULi, opMULq, opMULx, opMULy, opMULz, opMULw, 
	opMULA, opMULAi, opMULAq, opMULAx, opMULAy, opMULAz, opMULAw, 
	opMADD, opMADDi, opMADDq, opMADDx, opMADDy, opMADDz, opMADDw, 
	opMADDA, opMADDAi, opMADDAq, opMADDAx, opMADDAy, opMADDAz, opMADDAw, 
	opMSUB, opMSUBi, opMSUBq, opMSUBx, opMSUBy, opMSUBz, opMSUBw, 
	opMSUBA, opMSUBAi, opMSUBAq, opMSUBAx, opMSUBAy, opMSUBAz, opMSUBAw, 
	opMAX, opMAXi, opMAXx, opMAXy, opMAXz, opMAXw, 
	opMINI, opMINIi, opMINIx, opMINIy, opMINIz, opMINIw, 
	opFTOI0, opFTOI4, opFTOI12, opFTOI15,
	opITOF0, opITOF4, opITOF12, opITOF15,
	// Lower Instructions
	opDIV, opSQRT, opRSQRT, 
	opIADD, opIADDI, opIADDIU, 
	opIAND, opIOR, 
	opISUB, opISUBIU, 
	opMOVE, opMFIR, opMTIR, opMR32, opMFP,
	opLQ, opLQD, opLQI, 
	opSQ, opSQD, opSQI, 
	opILW, opISW, opILWR, opISWR, 
	opRINIT, opRGET, opRNEXT, opRXOR, 
	opWAITQ, opWAITP,
	opFSAND, opFSEQ, opFSOR, opFSSET,
	opFMAND, opFMEQ, opFMOR, 
	opFCAND, opFCEQ, opFCOR, opFCSET, opFCGET, 
	opIBEQ, opIBGEZ, opIBGTZ, opIBLTZ, opIBLEZ, opIBNE, 
	opB, opBAL, opJR, opJALR, 
	opESADD, opERSADD, opELENG, opERLENG, 
	opEATANxy, opEATANxz, opESUM, opERCPR, 
	opESQRT, opERSQRT, opESIN, opEATAN, 
	opEEXP, opXITOP, opXTOP, opXGKICK,
	opLastOpcode
};

static const char microOpcodeName[][16] = {
	// Upper Instructions
	"ABS", "CLIP", "OPMULA", "OPMSUB", "NOP", 
	"ADD", "ADDi", "ADDq", "ADDx", "ADDy", "ADDz", "ADDw", 
	"ADDA", "ADDAi", "ADDAq", "ADDAx", "ADDAy", "ADDAz", "ADDAw", 
	"SUB", "SUBi", "SUBq", "SUBx", "SUBy", "SUBz", "SUBw",
	"SUBA", "SUBAi", "SUBAq", "SUBAx", "SUBAy", "SUBAz", "SUBAw", 
	"MUL", "MULi", "MULq", "MULx", "MULy", "MULz", "MULw", 
	"MULA", "MULAi", "MULAq", "MULAx", "MULAy", "MULAz", "MULAw", 
	"MADD", "MADDi", "MADDq", "MADDx", "MADDy", "MADDz", "MADDw", 
	"MADDA", "MADDAi", "MADDAq", "MADDAx", "MADDAy", "MADDAz", "MADDAw", 
	"MSUB", "MSUBi", "MSUBq", "MSUBx", "MSUBy", "MSUBz", "MSUBw", 
	"MSUBA", "MSUBAi", "MSUBAq", "MSUBAx", "MSUBAy", "MSUBAz", "MSUBAw", 
	"MAX", "MAXi", "MAXx", "MAXy", "MAXz", "MAXw", 
	"MINI", "MINIi", "MINIx", "MINIy", "MINIz", "MINIw", 
	"FTOI0", "FTOI4", "FTOI12", "FTOI15", 
	"ITOF0", "ITOF4", "ITOF12", "ITOF15", 
	// Lower Instructions
	"DIV", "SQRT", "RSQRT", 
	"IADD", "IADDI", "IADDIU", 
	"IAND", "IOR", 
	"ISUB", "ISUBIU", 
	"MOVE", "MFIR", "MTIR", "MR32", "MFP",
	"LQ", "LQD", "LQI", 
	"SQ", "SQD", "SQI", 
	"ILW", "ISW", "ILWR", "ISWR", 
	"RINIT", "RGET", "RNEXT", "RXOR", 
	"WAITQ", "WAITP",
	"FSAND", "FSEQ", "FSOR", "FSSET",
	"FMAND", "FMEQ", "FMOR", 
	"FCAND", "FCEQ", "FCOR", "FCSET", "FCGET", 
	"IBEQ", "IBGEZ", "IBGTZ", "IBLTZ", "IBLEZ", "IBNE", 
	"B", "BAL", "JR", "JALR", 
	"ESADD", "ERSADD", "ELENG", "ERLENG", 
	"EATANxy", "EATANxz", "ESUM", "ERCPR", 
	"ESQRT", "ERSQRT", "ESIN", "EATAN", 
	"EEXP", "XITOP", "XTOP", "XGKICK"
};

static void mVU_printOP(std::string &result, u32 insn, int opCase, microOpcode opEnum, bool isACC)
{
	mVUlog("%s", microOpcodeName[opEnum]);
	if (opCase == 1) { if (isACC) { mVUlogACC(); } else { mVUlogFd(); } mVUlogFt(); }
	if (opCase == 2) { if (isACC) { mVUlogACC(); } else { mVUlogFd(); } mVUlogBC(); }
	if (opCase == 3) { if (isACC) { mVUlogACC(); } else { mVUlogFd(); } mVUlogI();  }
	if (opCase == 4) { if (isACC) { mVUlogACC(); } else { mVUlogFd(); } mVUlogQ();  }
}

static void mVU_FMACa(std::string &result, uint32_t insn, int opCase, int opType, bool isACC, microOpcode opEnum, int clampType)
{
	 mVU_printOP(result, insn, opCase, opEnum, isACC);
}

static void mVU_FMACb(std::string &result, uint32_t insn, int opCase, int opType, microOpcode opEnum, int clampType)
{
	mVU_printOP(result, insn, opCase, opEnum, true);
}

static void mVU_FMACc(std::string &result, uint32_t insn, int opCase, microOpcode opEnum, int clampType)
{
	mVU_printOP(result, insn, opCase, opEnum, false);
}

static void mVU_FMACd(std::string &result, uint32_t insn, int opCase, microOpcode opEnum, int clampType)
{
	mVU_printOP(result, insn, opCase, opEnum, false);
}

#define upper_op(mnenomic) void mnenomic (std::string &result, uint32_t insn, uint32_t pc)

upper_op(mVU_ABS)
{
	mVUlog("ABS"); mVUlogFtFs();
}

upper_op(mVU_OPMULA)
{
	mVUlog("OPMULA"); mVUlogACC(); mVUlogFt();
}

upper_op(mVU_OPMSUB)
{
	mVUlog("OPMSUB"); mVUlogFd(); mVUlogFt();
}

static void mVU_FTOIx(std::string &result, uint32_t insn, microOpcode opEnum)
{
	mVUlog("%s", microOpcodeName[opEnum]); mVUlogFtFs();
}

static void mVU_ITOFx(std::string &result, uint32_t insn, microOpcode opEnum)
{
	mVUlog("%s", microOpcodeName[opEnum]); mVUlogFtFs();
}

upper_op(mVU_CLIP)
{
	mVUlog("CLIP"); mVUlogCLIP();
}

#define isCOP2 0

enum clampModes {
	cFt  = 0x01, // Clamp Ft / I-reg / Q-reg
	cFs  = 0x02, // Clamp Fs
	cACC = 0x04, // Clamp ACC
};

upper_op(mVU_ADD)    { mVU_FMACa(result, insn, 1, 0, false, opADD,    0);  }
upper_op(mVU_ADDi)   { mVU_FMACa(result, insn, 3, 5, false, opADDi,   0);  }
upper_op(mVU_ADDq)   { mVU_FMACa(result, insn, 4, 0, false, opADDq,   0);  }
upper_op(mVU_ADDx)   { mVU_FMACa(result, insn, 2, 0, false, opADDx,   0);  }
upper_op(mVU_ADDy)   { mVU_FMACa(result, insn, 2, 0, false, opADDy,   0);  }
upper_op(mVU_ADDz)   { mVU_FMACa(result, insn, 2, 0, false, opADDz,   0);  }
upper_op(mVU_ADDw)   { mVU_FMACa(result, insn, 2, 0, false, opADDw,   0);  }
upper_op(mVU_ADDA)   { mVU_FMACa(result, insn, 1, 0, true,  opADDA,   0);  }
upper_op(mVU_ADDAi)  { mVU_FMACa(result, insn, 3, 0, true,  opADDAi,  0);  }
upper_op(mVU_ADDAq)  { mVU_FMACa(result, insn, 4, 0, true,  opADDAq,  0);  }
upper_op(mVU_ADDAx)  { mVU_FMACa(result, insn, 2, 0, true,  opADDAx,  0);  }
upper_op(mVU_ADDAy)  { mVU_FMACa(result, insn, 2, 0, true,  opADDAy,  0);  }
upper_op(mVU_ADDAz)  { mVU_FMACa(result, insn, 2, 0, true,  opADDAz,  0);  }
upper_op(mVU_ADDAw)  { mVU_FMACa(result, insn, 2, 0, true,  opADDAw,  0);  }
upper_op(mVU_SUB)    { mVU_FMACa(result, insn, 1, 1, false, opSUB,  (_XYZW_PS)?(cFs|cFt):0);   } // Clamp (Kingdom Hearts I (VU0))
upper_op(mVU_SUBi)   { mVU_FMACa(result, insn, 3, 1, false, opSUBi, (_XYZW_PS)?(cFs|cFt):0);   } // Clamp (Kingdom Hearts I (VU0))
upper_op(mVU_SUBq)   { mVU_FMACa(result, insn, 4, 1, false, opSUBq, (_XYZW_PS)?(cFs|cFt):0);   } // Clamp (Kingdom Hearts I (VU0))
upper_op(mVU_SUBx)   { mVU_FMACa(result, insn, 2, 1, false, opSUBx, (_XYZW_PS)?(cFs|cFt):0);   } // Clamp (Kingdom Hearts I (VU0))
upper_op(mVU_SUBy)   { mVU_FMACa(result, insn, 2, 1, false, opSUBy, (_XYZW_PS)?(cFs|cFt):0);   } // Clamp (Kingdom Hearts I (VU0))
upper_op(mVU_SUBz)   { mVU_FMACa(result, insn, 2, 1, false, opSUBz, (_XYZW_PS)?(cFs|cFt):0);   } // Clamp (Kingdom Hearts I (VU0))
upper_op(mVU_SUBw)   { mVU_FMACa(result, insn, 2, 1, false, opSUBw, (_XYZW_PS)?(cFs|cFt):0);   } // Clamp (Kingdom Hearts I (VU0))
upper_op(mVU_SUBA)   { mVU_FMACa(result, insn, 1, 1, true,  opSUBA,   0);  }
upper_op(mVU_SUBAi)  { mVU_FMACa(result, insn, 3, 1, true,  opSUBAi,  0);  }
upper_op(mVU_SUBAq)  { mVU_FMACa(result, insn, 4, 1, true,  opSUBAq,  0);  }
upper_op(mVU_SUBAx)  { mVU_FMACa(result, insn, 2, 1, true,  opSUBAx,  0);  }
upper_op(mVU_SUBAy)  { mVU_FMACa(result, insn, 2, 1, true,  opSUBAy,  0);  }
upper_op(mVU_SUBAz)  { mVU_FMACa(result, insn, 2, 1, true,  opSUBAz,  0);  }
upper_op(mVU_SUBAw)  { mVU_FMACa(result, insn, 2, 1, true,  opSUBAw,  0);  }
upper_op(mVU_MUL)    { mVU_FMACa(result, insn, 1, 2, false, opMUL,  (_XYZW_PS)?(cFs|cFt):cFs); } // Clamp (TOTA, DoM, Ice Age (VU0))
upper_op(mVU_MULi)   { mVU_FMACa(result, insn, 3, 2, false, opMULi, (_XYZW_PS)?(cFs|cFt):cFs); } // Clamp (TOTA, DoM, Ice Age (VU0))
upper_op(mVU_MULq)   { mVU_FMACa(result, insn, 4, 2, false, opMULq, (_XYZW_PS)?(cFs|cFt):cFs); } // Clamp (TOTA, DoM, Ice Age (VU0))
upper_op(mVU_MULx)   { mVU_FMACa(result, insn, 2, 2, false, opMULx, (_XYZW_PS)?(cFs|cFt):cFs); } // Clamp (TOTA, DoM, Ice Age (vu0))
upper_op(mVU_MULy)   { mVU_FMACa(result, insn, 2, 2, false, opMULy, (_XYZW_PS)?(cFs|cFt):cFs); } // Clamp (TOTA, DoM, Ice Age (VU0))
upper_op(mVU_MULz)   { mVU_FMACa(result, insn, 2, 2, false, opMULz, (_XYZW_PS)?(cFs|cFt):cFs); } // Clamp (TOTA, DoM, Ice Age (VU0))
upper_op(mVU_MULw)   { mVU_FMACa(result, insn, 2, 2, false, opMULw, (_XYZW_PS)?(cFs|cFt):cFs); } // Clamp (TOTA, DoM, Ice Age (VU0))
upper_op(mVU_MULA)   { mVU_FMACa(result, insn, 1, 2, true,  opMULA,   0);  }
upper_op(mVU_MULAi)  { mVU_FMACa(result, insn, 3, 2, true,  opMULAi,  0);  }
upper_op(mVU_MULAq)  { mVU_FMACa(result, insn, 4, 2, true,  opMULAq,  0);  }
upper_op(mVU_MULAx)  { mVU_FMACa(result, insn, 2, 2, true,  opMULAx,  cFs);} // Clamp (TOTA, DoM, ...)
upper_op(mVU_MULAy)  { mVU_FMACa(result, insn, 2, 2, true,  opMULAy,  cFs);} // Clamp (TOTA, DoM, ...)
upper_op(mVU_MULAz)  { mVU_FMACa(result, insn, 2, 2, true,  opMULAz,  cFs);} // Clamp (TOTA, DoM, ...)
upper_op(mVU_MULAw)  { mVU_FMACa(result, insn, 2, 2, true, opMULAw, (_XYZW_PS) ? (cFs | cFt) : cFs); } // Clamp (TOTA, DoM, ...)- Ft for Superman - Shadow Of Apokolips
upper_op(mVU_MADD)   { mVU_FMACc(result, insn, 1,			opMADD, 0); }
upper_op(mVU_MADDi)  { mVU_FMACc(result, insn, 3,			opMADDi, 0); }
upper_op(mVU_MADDq)  { mVU_FMACc(result, insn, 4,			opMADDq, 0); }
upper_op(mVU_MADDx)  { mVU_FMACc(result, insn, 2,			opMADDx, cFs); } // Clamp (TOTA, DoM, ...)
upper_op(mVU_MADDy)  { mVU_FMACc(result, insn, 2,			opMADDy, cFs); } // Clamp (TOTA, DoM, ...)
upper_op(mVU_MADDz)  { mVU_FMACc(result, insn, 2,			opMADDz, cFs); } // Clamp (TOTA, DoM, ...)
upper_op(mVU_MADDw)  { mVU_FMACc(result, insn, 2,           opMADDw, (isCOP2)?(cACC|cFt|cFs):cFs);} // Clamp (ICO (COP2), TOTA, DoM)
upper_op(mVU_MADDA)  { mVU_FMACb(result, insn, 1, 0,        opMADDA,  0);  }
upper_op(mVU_MADDAi) { mVU_FMACb(result, insn, 3, 0,        opMADDAi, 0);  }
upper_op(mVU_MADDAq) { mVU_FMACb(result, insn, 4, 0,        opMADDAq, 0);  }
upper_op(mVU_MADDAx) { mVU_FMACb(result, insn, 2, 0,        opMADDAx, cFs);} // Clamp (TOTA, DoM, ...)
upper_op(mVU_MADDAy) { mVU_FMACb(result, insn, 2, 0,        opMADDAy, cFs);} // Clamp (TOTA, DoM, ...)
upper_op(mVU_MADDAz) { mVU_FMACb(result, insn, 2, 0,        opMADDAz, cFs);} // Clamp (TOTA, DoM, ...)
upper_op(mVU_MADDAw) { mVU_FMACb(result, insn, 2, 0,        opMADDAw, cFs);} // Clamp (TOTA, DoM, ...)
upper_op(mVU_MSUB)   { mVU_FMACd(result, insn, 1,			 opMSUB, (isCOP2) ? cFs : 0); } // Clamp ( Superman - Shadow Of Apokolips)
upper_op(mVU_MSUBi)  { mVU_FMACd(result, insn, 3,           opMSUBi,  0);  }
upper_op(mVU_MSUBq)  { mVU_FMACd(result, insn, 4,           opMSUBq,  0);  }
upper_op(mVU_MSUBx)  { mVU_FMACd(result, insn, 2,           opMSUBx,  0);  }
upper_op(mVU_MSUBy)  { mVU_FMACd(result, insn, 2,           opMSUBy,  0);  }
upper_op(mVU_MSUBz)  { mVU_FMACd(result, insn, 2,           opMSUBz,  0);  }
upper_op(mVU_MSUBw)  { mVU_FMACd(result, insn, 2,           opMSUBw,  0);  }
upper_op(mVU_MSUBA)  { mVU_FMACb(result, insn, 1, 1,        opMSUBA,  0);  }
upper_op(mVU_MSUBAi) { mVU_FMACb(result, insn, 3, 1,        opMSUBAi, 0);  }
upper_op(mVU_MSUBAq) { mVU_FMACb(result, insn, 4, 1,        opMSUBAq, 0);  }
upper_op(mVU_MSUBAx) { mVU_FMACb(result, insn, 2, 1,        opMSUBAx, 0);  }
upper_op(mVU_MSUBAy) { mVU_FMACb(result, insn, 2, 1,        opMSUBAy, 0);  }
upper_op(mVU_MSUBAz) { mVU_FMACb(result, insn, 2, 1,        opMSUBAz, 0);  }
upper_op(mVU_MSUBAw) { mVU_FMACb(result, insn, 2, 1,        opMSUBAw, 0);  }
upper_op(mVU_MAX)    { mVU_FMACa(result, insn, 1, 3, false, opMAX,    0);  }
upper_op(mVU_MAXi)   { mVU_FMACa(result, insn, 3, 3, false, opMAXi,   0);  }
upper_op(mVU_MAXx)   { mVU_FMACa(result, insn, 2, 3, false, opMAXx,   0);  }
upper_op(mVU_MAXy)   { mVU_FMACa(result, insn, 2, 3, false, opMAXy,   0);  }
upper_op(mVU_MAXz)   { mVU_FMACa(result, insn, 2, 3, false, opMAXz,   0);  }
upper_op(mVU_MAXw)   { mVU_FMACa(result, insn, 2, 3, false, opMAXw,   0);  }
upper_op(mVU_MINI)   { mVU_FMACa(result, insn, 1, 4, false, opMINI,   0);  }
upper_op(mVU_MINIi)  { mVU_FMACa(result, insn, 3, 4, false, opMINIi,  0);  }
upper_op(mVU_MINIx)  { mVU_FMACa(result, insn, 2, 4, false, opMINIx,  0);  }
upper_op(mVU_MINIy)  { mVU_FMACa(result, insn, 2, 4, false, opMINIy,  0);  }
upper_op(mVU_MINIz)  { mVU_FMACa(result, insn, 2, 4, false, opMINIz,  0);  }
upper_op(mVU_MINIw)  { mVU_FMACa(result, insn, 2, 4, false, opMINIw,  0);  }
upper_op(mVU_FTOI0)  { mVU_FTOIx(result, insn,                  opFTOI0);      }
upper_op(mVU_FTOI4)  { mVU_FTOIx(result, insn,        opFTOI4);      }
upper_op(mVU_FTOI12) { mVU_FTOIx(result, insn,       opFTOI12);     }
upper_op(mVU_FTOI15) { mVU_FTOIx(result, insn,       opFTOI15);     }
upper_op(mVU_ITOF0)  { mVU_ITOFx(result, insn,                  opITOF0);      }
upper_op(mVU_ITOF4)  { mVU_ITOFx(result, insn,        opITOF4);      }
upper_op(mVU_ITOF12) { mVU_ITOFx(result, insn,       opITOF12);     }
upper_op(mVU_ITOF15) { mVU_ITOFx(result, insn,       opITOF15);     }
upper_op(mVU_NOP)    { mVUlog("NOP"); }

upper_op(mVU_UPPER_FD_00);
upper_op(mVU_UPPER_FD_01);
upper_op(mVU_UPPER_FD_10);
upper_op(mVU_UPPER_FD_11);

static const Fnptr_mVUrecInst mVU_UPPER_OPCODE[64] = {
	mVU_ADDx	, mVU_ADDy		, mVU_ADDz		, mVU_ADDw,	
	mVU_SUBx	, mVU_SUBy		, mVU_SUBz		, mVU_SUBw,	
	mVU_MADDx	, mVU_MADDy		, mVU_MADDz		, mVU_MADDw,
	mVU_MSUBx	, mVU_MSUBy		, mVU_MSUBz		, mVU_MSUBw,
	mVU_MAXx	, mVU_MAXy		, mVU_MAXz		, mVU_MAXw,
	mVU_MINIx	, mVU_MINIy		, mVU_MINIz		, mVU_MINIw,
	mVU_MULx	, mVU_MULy		, mVU_MULz		, mVU_MULw,	
	mVU_MULq	, mVU_MAXi		, mVU_MULi		, mVU_MINIi,
	mVU_ADDq	, mVU_MADDq		, mVU_ADDi		, mVU_MADDi,
	mVU_SUBq	, mVU_MSUBq		, mVU_SUBi		, mVU_MSUBi,
	mVU_ADD		, mVU_MADD		, mVU_MUL		, mVU_MAX,	
	mVU_SUB		, mVU_MSUB		, mVU_OPMSUB	, mVU_MINI,	
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVU_UPPER_FD_00, mVU_UPPER_FD_01, mVU_UPPER_FD_10, mVU_UPPER_FD_11,
};

static const Fnptr_mVUrecInst mVU_UPPER_FD_00_TABLE [32] = {
	mVU_ADDAx	, mVU_SUBAx		, mVU_MADDAx	, mVU_MSUBAx,
	mVU_ITOF0	, mVU_FTOI0		, mVU_MULAx		, mVU_MULAq,
	mVU_ADDAq	, mVU_SUBAq		, mVU_ADDA		, mVU_SUBA,	
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
};

static const Fnptr_mVUrecInst mVU_UPPER_FD_01_TABLE [32] = {
	mVU_ADDAy	, mVU_SUBAy		, mVU_MADDAy	, mVU_MSUBAy,
	mVU_ITOF4	, mVU_FTOI4		, mVU_MULAy		, mVU_ABS,	
	mVU_MADDAq	, mVU_MSUBAq	, mVU_MADDA		, mVU_MSUBA,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
};

static const Fnptr_mVUrecInst mVU_UPPER_FD_10_TABLE [32] = {
	mVU_ADDAz	, mVU_SUBAz		, mVU_MADDAz	, mVU_MSUBAz,
	mVU_ITOF12	, mVU_FTOI12	, mVU_MULAz		, mVU_MULAi,
	mVU_ADDAi	, mVU_SUBAi		, mVU_MULA		, mVU_OPMULA,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
};

static const Fnptr_mVUrecInst mVU_UPPER_FD_11_TABLE [32] = {
	mVU_ADDAw	, mVU_SUBAw		, mVU_MADDAw	, mVU_MSUBAw,
	mVU_ITOF15	, mVU_FTOI15	, mVU_MULAw		, mVU_CLIP,
	mVU_MADDAi	, mVU_MSUBAi	, mVUunknown	, mVU_NOP,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
	mVUunknown	, mVUunknown	, mVUunknown	, mVUunknown,
};

upper_op(mVU_UPPER_FD_00)	{ mVU_UPPER_FD_00_TABLE		[((insn >> 6) & 0x1f)](result, insn, pc); }
upper_op(mVU_UPPER_FD_01)	{ mVU_UPPER_FD_01_TABLE		[((insn >> 6) & 0x1f)](result, insn, pc); }
upper_op(mVU_UPPER_FD_10)	{ mVU_UPPER_FD_10_TABLE		[((insn >> 6) & 0x1f)](result, insn, pc); }
upper_op(mVU_UPPER_FD_11)	{ mVU_UPPER_FD_11_TABLE		[((insn >> 6) & 0x1f)](result, insn, pc); }

upper_op(mVUopU)			{ mVU_UPPER_OPCODE			[ (insn & 0x3f) ](result, insn, pc); } // Gets Upper Opcode


std::string disassemble_upper(uint32_t insn, uint32_t pc)
{
	std::string result;
	mVUopU(result, insn, pc);
	return result;
}

#endif
