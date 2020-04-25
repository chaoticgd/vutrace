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

#define vu_lower(mnenomic) void mVU_##mnenomic (std::string &result, uint32_t insn, uint32_t pc)
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

static inline u32 branchAddr(uint32_t insn, uint32_t pc)
{
	return ((((pc + 2) + (_Imm11_ * 2)) & VU1_PROGSIZE) * 4);
}

vu_lower(DIV) { mVUlog("DIV Q, vf%02d%s, vf%02d%s", _Fs_, _Fsf_String, _Ft_, _Ftf_String); }
vu_lower(SQRT) { mVUlog("SQRT Q, vf%02d%s", _Ft_, _Ftf_String); }
vu_lower(RSQRT) { mVUlog("RSQRT Q, vf%02d%s, vf%02d%s", _Fs_, _Fsf_String, _Ft_, _Ftf_String); }
vu_lower(EATAN) { mVUlog("EATAN P"); }
vu_lower(EATANxy) { mVUlog("EATANxy P"); }
vu_lower(EATANxz) { mVUlog("EATANxz P"); }
vu_lower(EEXP) { mVUlog("EEXP P"); }
vu_lower(ELENG) { mVUlog("ELENG P"); }
vu_lower(ERCPR) { mVUlog("ERCPR P"); }
vu_lower(ERLENG) { mVUlog("ERLENG P"); }
vu_lower(ERSADD) { mVUlog("ERSADD P"); }
vu_lower(ERSQRT) { mVUlog("ERSQRT P"); }
vu_lower(ESADD) { mVUlog("ESADD P"); }
vu_lower(ESIN) { mVUlog("ESIN P"); }
vu_lower(ESQRT) { mVUlog("ESQRT P"); }
vu_lower(ESUM) { mVUlog("ESUM P"); }
vu_lower(FCAND) { mVUlog("FCAND vi01, $%x", _Imm24_); }
vu_lower(FCEQ) { mVUlog("FCEQ vi01, $%x", _Imm24_); }
vu_lower(FCGET) { mVUlog("FCGET vi%02d", _Ft_);	   }
vu_lower(FCOR) { mVUlog("FCOR vi01, $%x", _Imm24_); }
vu_lower(FCSET) { mVUlog("FCSET $%x", _Imm24_); }
vu_lower(FMAND) { mVUlog("FMAND vi%02d, vi%02d", _Ft_, _Fs_); }
vu_lower(FMEQ) { mVUlog("FMEQ vi%02d, vi%02d", _Ft_, _Fs_); }
vu_lower(FMOR) { mVUlog("FMOR vi%02d, vi%02d", _Ft_, _Fs_); }
vu_lower(FSAND) { mVUlog("FSAND vi%02d, $%x", _Ft_, _Imm12_); }
vu_lower(FSOR) { mVUlog("FSOR vi%02d, $%x", _Ft_, _Imm12_); }
vu_lower(FSEQ) { mVUlog("FSEQ vi%02d, $%x", _Ft_, _Imm12_); }
vu_lower(FSSET) { mVUlog("FSSET $%x", _Imm12_); }
vu_lower(IADD) { mVUlog("IADD vi%02d, vi%02d, vi%02d", _Fd_, _Fs_, _Ft_); }
vu_lower(IADDI) { mVUlog("IADDI vi%02d, vi%02d, %d", _Ft_, _Fs_, _Imm5_); }
vu_lower(IADDIU) { mVUlog("IADDIU vi%02d, vi%02d, %d", _Ft_, _Fs_, _Imm15_); }
vu_lower(IAND) { mVUlog("IAND vi%02d, vi%02d, vi%02d", _Fd_, _Fs_, _Ft_); }
vu_lower(IOR) { mVUlog("IOR vi%02d, vi%02d, vi%02d", _Fd_, _Fs_, _Ft_); }
vu_lower(ISUB) { mVUlog("ISUB vi%02d, vi%02d, vi%02d", _Fd_, _Fs_, _Ft_); }
vu_lower(ISUBIU) { mVUlog("ISUBIU vi%02d, vi%02d, %d", _Ft_, _Fs_, _Imm15_); }
vu_lower(MFIR) { mVUlog("MFIR.%s vf%02d, vi%02d", _XYZW_String, _Ft_, _Fs_); }
vu_lower(MFP) { mVUlog("MFP.%s vf%02d, P", _XYZW_String, _Ft_); }
vu_lower(MOVE) { mVUlog("MOVE.%s vf%02d, vf%02d", _XYZW_String, _Ft_, _Fs_); }
vu_lower(MR32) { mVUlog("MR32.%s vf%02d, vf%02d", _XYZW_String, _Ft_, _Fs_); }
vu_lower(MTIR) { mVUlog("MTIR vi%02d, vf%02d%s", _Ft_, _Fs_, _Fsf_String); }
vu_lower(ILW) { mVUlog("ILW.%s vi%02d, vi%02d + %d", _XYZW_String, _Ft_, _Fs_, _Imm11_); }
vu_lower(ILWR) { mVUlog("ILWR.%s vi%02d, vi%02d", _XYZW_String, _Ft_, _Fs_); }
vu_lower(ISW) { mVUlog("ISW.%s vi%02d, vi%02d + %d", _XYZW_String, _Ft_, _Fs_, _Imm11_);  }
vu_lower(ISWR) { mVUlog("ISWR.%s vi%02d, vi%02d", _XYZW_String, _Ft_, _Fs_); }
vu_lower(LQ) { mVUlog("LQ.%s vf%02d, vi%02d + %d", _XYZW_String, _Ft_, _Fs_, _Imm11_); }
vu_lower(LQD) { mVUlog("LQD.%s vf%02d, --vi%02d", _XYZW_String, _Ft_, _Is_); }
vu_lower(LQI) { mVUlog("LQI.%s vf%02d, vi%02d++", _XYZW_String, _Ft_, _Fs_); }
vu_lower(SQ) { mVUlog("SQ.%s vf%02d, vi%02d + %d", _XYZW_String, _Fs_, _Ft_, _Imm11_); }
vu_lower(SQD) { mVUlog("SQD.%s vf%02d, --vi%02d", _XYZW_String, _Fs_, _Ft_); }
vu_lower(SQI) { mVUlog("SQI.%s vf%02d, vi%02d++", _XYZW_String, _Fs_, _Ft_); }
vu_lower(RINIT) { mVUlog("RINIT R, vf%02d%s", _Fs_, _Fsf_String); }
vu_lower(RGET) { mVUlog("RGET.%s vf%02d, R", _XYZW_String, _Ft_); }
vu_lower(RNEXT) { mVUlog("RNEXT.%s vf%02d, R", _XYZW_String, _Ft_); }
vu_lower(RXOR) { mVUlog("RXOR R, vf%02d%s", _Fs_, _Fsf_String); }
vu_lower(WAITP) { mVUlog("WAITP"); }
vu_lower(WAITQ) { mVUlog("WAITQ"); }
vu_lower(XTOP) { mVUlog("XTOP vi%02d", _Ft_); }
vu_lower(XITOP) { mVUlog("XITOP vi%02d", _Ft_); }
vu_lower(XGKICK) { mVUlog("XGKICK vi%02d", _Fs_); }
vu_lower(B) { mVUlog("B [%04x]", branchAddr(insn, pc)); }
vu_lower(BAL) { mVUlog("BAL vi%02d [%04x]", _Ft_, branchAddr(insn, pc)); }
vu_lower(IBEQ) { mVUlog("IBEQ vi%02d, vi%02d [%04x]", _Ft_, _Fs_, branchAddr(insn, pc)); }
vu_lower(IBGEZ) { mVUlog("IBGEZ vi%02d [%04x]", _Fs_, branchAddr(insn, pc)); }
vu_lower(IBGTZ) { mVUlog("IBGTZ vi%02d [%04x]", _Fs_, branchAddr(insn, pc)); }
vu_lower(IBLEZ) { mVUlog("IBLEZ vi%02d [%04x]", _Fs_, branchAddr(insn, pc)); }
vu_lower(IBLTZ) { mVUlog("IBLTZ vi%02d [%04x]", _Fs_, branchAddr(insn, pc)); }
vu_lower(IBNE) { mVUlog("IBNE vi%02d, vi%02d [%04x]", _Ft_, _Fs_, branchAddr(insn, pc)); }
vu_lower(JR) { mVUlog("JR [vi%02d]", _Fs_); }
vu_lower(JALR) { mVUlog("JALR vi%02d, [vi%02d]", _Ft_, _Fs_); }

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

#endif
