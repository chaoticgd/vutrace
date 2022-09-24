/*
	vutrace - Hacky VU tracer/debugger.
	Copyright (C) 2020 chaoticgd

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef GIF_H
#define GIF_H

#include <vector>
#include <cstring>
#include <stdio.h>

#include "pcsx2defs.h"

enum GifFlag
{
	GIFFLAG_PACKED  = 0b00,
	GIFFLAG_REGLIST = 0b01,
	GIFFLAG_IMAGE   = 0b10,
	GIFFLAG_DISABLE = 0b11
};

enum GsRegister
{
	GSREG_PRIM     = 0x0,
	GSREG_RGBAQ    = 0x1,
	GSREG_ST       = 0x2,
	GSREG_UV       = 0x3,
	GSREG_XYZF2    = 0x4,
	GSREG_XYZ2     = 0x5,
	GSREG_TEX0_1   = 0x6,
	GSREG_TEX0_2   = 0x7,
	GSREG_CLAMP_1  = 0x8,
	GSREG_CLAMP_2  = 0x9,
	GSREG_FOG      = 0xa,
	GSREG_RESERVED = 0xb,
	GSREG_XYZF3    = 0xc,
	GSREG_XYZ3     = 0xd,
	GSREG_AD       = 0xe,
	GSREG_NOP      = 0xf
};

enum GIF_A_D_REG
{
	GIF_A_D_REG_PRIM       = 0x00,
	GIF_A_D_REG_RGBAQ      = 0x01,
	GIF_A_D_REG_ST         = 0x02,
	GIF_A_D_REG_UV         = 0x03,
	GIF_A_D_REG_XYZF2      = 0x04,
	GIF_A_D_REG_XYZ2       = 0x05,
	GIF_A_D_REG_TEX0_1     = 0x06,
	GIF_A_D_REG_TEX0_2     = 0x07,
	GIF_A_D_REG_CLAMP_1    = 0x08,
	GIF_A_D_REG_CLAMP_2    = 0x09,
	GIF_A_D_REG_FOG        = 0x0a,
	GIF_A_D_REG_XYZF3      = 0x0c,
	GIF_A_D_REG_XYZ3       = 0x0d,
	GIF_A_D_REG_NOP        = 0x0f,
	GIF_A_D_REG_TEX1_1     = 0x14,
	GIF_A_D_REG_TEX1_2     = 0x15,
	GIF_A_D_REG_TEX2_1     = 0x16,
	GIF_A_D_REG_TEX2_2     = 0x17,
	GIF_A_D_REG_XYOFFSET_1 = 0x18,
	GIF_A_D_REG_XYOFFSET_2 = 0x19,
	GIF_A_D_REG_PRMODECONT = 0x1a,
	GIF_A_D_REG_PRMODE     = 0x1b,
	GIF_A_D_REG_TEXCLUT    = 0x1c,
	GIF_A_D_REG_SCANMSK    = 0x22,
	GIF_A_D_REG_MIPTBP1_1  = 0x34,
	GIF_A_D_REG_MIPTBP1_2  = 0x35,
	GIF_A_D_REG_MIPTBP2_1  = 0x36,
	GIF_A_D_REG_MIPTBP2_2  = 0x37,
	GIF_A_D_REG_TEXA       = 0x3b,
	GIF_A_D_REG_FOGCOL     = 0x3d,
	GIF_A_D_REG_TEXFLUSH   = 0x3f,
	GIF_A_D_REG_SCISSOR_1  = 0x40,
	GIF_A_D_REG_SCISSOR_2  = 0x41,
	GIF_A_D_REG_ALPHA_1    = 0x42,
	GIF_A_D_REG_ALPHA_2    = 0x43,
	GIF_A_D_REG_DIMX       = 0x44,
	GIF_A_D_REG_DTHE       = 0x45,
	GIF_A_D_REG_COLCLAMP   = 0x46,
	GIF_A_D_REG_TEST_1     = 0x47,
	GIF_A_D_REG_TEST_2     = 0x48,
	GIF_A_D_REG_PABE       = 0x49,
	GIF_A_D_REG_FBA_1      = 0x4a,
	GIF_A_D_REG_FBA_2      = 0x4b,
	GIF_A_D_REG_FRAME_1    = 0x4c,
	GIF_A_D_REG_FRAME_2    = 0x4d,
	GIF_A_D_REG_ZBUF_1     = 0x4e,
	GIF_A_D_REG_ZBUF_2     = 0x4f,
	GIF_A_D_REG_BITBLTBUF  = 0x50,
	GIF_A_D_REG_TRXPOS     = 0x51,
	GIF_A_D_REG_TRXREG     = 0x52,
	GIF_A_D_REG_TRXDIR     = 0x53,
	GIF_A_D_REG_HWREG      = 0x54,
	GIF_A_D_REG_SIGNAL     = 0x60,
	GIF_A_D_REG_FINISH     = 0x61,
	GIF_A_D_REG_LABEL      = 0x62,
};

enum GsPrimitiveType
{
	GSPRIM_POINT          = 0b000,
	GSPRIM_LINE           = 0b001,
	GSPRIM_LINE_STRIP     = 0b010,
	GSPRIM_TRIANGLE       = 0b011,
	GSPRIM_TRIANGLE_STRIP = 0b100,
	GSPRIM_TRIANGLE_FAN   = 0b101,
	GSPRIM_PRITE          = 0b110,
	GSPRIM_HALT_AND_CATCH_FIRE = 0b111
};

enum GsShadingMethod
{
	GSSHADE_FLAT = 0b0,
	GSSHADE_GOURAUD = 0b1
};

enum GsTexCoords
{
	GSFST_STQ = 0b0,
	GSFST_UV = 0b1
};

enum GsContext
{
	GSCTXT_1 = 0b0,
	GSCTXT_2 = 0b1
};

struct GsPrimRegister
{
	GsPrimitiveType prim;
	GsShadingMethod iip;
	bool tme; // texture mapping
	bool fge; // fogging
	bool abe; // alpha blending
	bool aa1; // antialiasing
	GsTexCoords fst;
	GsContext ctxt;
	bool fix; // fragment value control
};

struct GifTag
{
	int nloop;
	int eop;
	GsPrimRegister prim;
	int pre;
	GifFlag flag;
	std::vector<GsRegister> regs;
};

struct GsPackedData
{
	u8 buffer[0x10];
	int source_address;
	GsRegister reg;
	union {
		struct {
			GIF_A_D_REG addr;
			uint64_t data;
		} ad;
		struct {
			s16 x, y;
			u32 z;
			u8 f;
			bool adc;
		} xyzf2;
	};
};

struct GsRegListData
{
	int source_address;
	u64 value;
};

struct GsPrimitive
{
	GifTag tag;
	std::vector<GsPackedData> packed_data;
	std::vector<GsRegListData> reglist_data;
};

struct GsPacket
{
	std::vector<GsPrimitive> primitives;
};

GsPacket read_gs_packet(u8 *data, int size);
GifTag read_gif_tag(u64 high_part, u64 low_part);
void interpret_packed_data(GsPackedData &item);
int bit_range(u64 val, int lo, int hi);

GsPacket read_gs_packet(u8 *data, int size)
{
	int pos = 0;
	
	GsPacket packet;
	do {
		GsPrimitive prim;
		
		if(pos + 0x10 >= size) {
			fprintf(stderr, "GIFtag overflowed VU memory!\n");
			return packet;
		}
		u64 low_tag = *(u64*) &data[pos];
		pos += 8;
		u64 high_tag = *(u64*) &data[pos];
		pos += 8;
		
		prim.tag = read_gif_tag(high_tag, low_tag);
		
		if(prim.tag.flag == GIFFLAG_PACKED) {
			for(int i = 0; i < prim.tag.nloop; i++) {
				for(size_t j = 0; j < prim.tag.regs.size(); j++) {
					GsPackedData item;
					if(pos + 0x10 >= size) {
						fprintf(stderr, "GS packet data overflowed VU memory!\n");
						return packet;
					}
					memcpy(item.buffer, &data[pos], 0x10);
					item.source_address = VU1_MEMSIZE - size + pos;
					pos += 0x10;
					item.reg = prim.tag.regs[j];
					interpret_packed_data(item);
					prim.packed_data.push_back(item);
				}
			}
		} else {
			fprintf(stderr, "Unsupported GIF flag!\n");
			return packet;
		}
		
		packet.primitives.push_back(prim);
	} while(packet.primitives.back().tag.eop == 0);
	return packet;
}

GifTag read_gif_tag(u64 high_part, u64 low_part)
{
	int prim_raw = bit_range(low_part, 47, 57);
	
	GsPrimRegister prim;
	prim.prim = (GsPrimitiveType) bit_range(prim_raw, 0, 2);
	prim.iip = (GsShadingMethod) bit_range(prim_raw, 3, 3);
	prim.tme = bit_range(prim_raw, 4, 4);
	prim.fge = bit_range(prim_raw, 5, 5);
	prim.abe = bit_range(prim_raw, 6, 6);
	prim.aa1 = bit_range(prim_raw, 7, 7);
	prim.fst = (GsTexCoords) bit_range(prim_raw, 8, 8);
	prim.ctxt = (GsContext) bit_range(prim_raw, 9, 9);
	prim.fix = bit_range(prim_raw, 10, 10);
	
	GifTag tag;
	tag.nloop = bit_range(low_part, 0, 14);
	tag.eop = bit_range(low_part, 15, 15);
	tag.pre = bit_range(low_part, 46, 46);
	tag.prim = prim;
	tag.flag = (GifFlag) bit_range(low_part, 58, 59);
	int nregs = bit_range(low_part, 60, 63);
	for(int i = 0; i < nregs; i++) {
		tag.regs.push_back((GsRegister) bit_range(high_part, i * 4, i * 4 + 3));
	}
	return tag;
}

void interpret_packed_data(GsPackedData &item)
{
	u64 lo = *(u64*) &item.buffer[0];
	u64 hi = *(u64*) &item.buffer[8];
	
	switch(item.reg) {
		case GSREG_AD: {
			item.ad.addr = (GIF_A_D_REG) bit_range(hi, 0, 7);
			item.ad.data = lo;
			break;
		}
		case GSREG_XYZF2: {
			item.xyzf2.x = bit_range(lo, 0, 15);
			item.xyzf2.y = bit_range(lo, 32, 47);
			item.xyzf2.z = bit_range(hi, 4, 27);
			item.xyzf2.f = bit_range(hi, 36, 43);
			item.xyzf2.adc = bit_range(hi, 47, 47);
		}
	}
}

const char *gif_flag_name(GifFlag flag)
{
	switch(flag) {
		case GIFFLAG_PACKED: return "PACKED";
		case GIFFLAG_REGLIST: return "REGLIST";
		case GIFFLAG_IMAGE: return "IMAGE";
		case GIFFLAG_DISABLE: return "DISABLE";
		default: return "ERR";
	}
}

const char *gs_register_name(GsRegister reg)
{
	switch(reg) {
		case GSREG_PRIM: return "PRIM";
		case GSREG_RGBAQ: return "RGBAQ";
		case GSREG_ST: return "ST";
		case GSREG_UV: return "UV";
		case GSREG_XYZF2: return "XYZF2";
		case GSREG_XYZ2: return "XYZ2";
		case GSREG_TEX0_1: return "TEX0_1";
		case GSREG_TEX0_2: return "TEX0_2";
		case GSREG_CLAMP_1: return "CLAMP_1";
		case GSREG_CLAMP_2: return "CLAMP_2";
		case GSREG_FOG: return "FOG";
		case GSREG_RESERVED: return "RESERVED";
		case GSREG_XYZF3: return "XYZF3";
		case GSREG_XYZ3: return "XYZ3";
		case GSREG_AD: return "AD";
		case GSREG_NOP: return "NOP";
		default: return "ERR";
	}
}

const char* gif_ad_register_name(GIF_A_D_REG reg) {
	switch(reg) {
		case GIF_A_D_REG_PRIM: return "PRIM";
		case GIF_A_D_REG_RGBAQ: return "RGBAQ";
		case GIF_A_D_REG_ST: return "ST";
		case GIF_A_D_REG_UV: return "UV";
		case GIF_A_D_REG_XYZF2: return "XYZF2";
		case GIF_A_D_REG_XYZ2: return "XYZ2";
		case GIF_A_D_REG_TEX0_1: return "TEX0_1";
		case GIF_A_D_REG_TEX0_2: return "TEX0_2";
		case GIF_A_D_REG_CLAMP_1: return "CLAMP_1";
		case GIF_A_D_REG_CLAMP_2: return "CLAMP_2";
		case GIF_A_D_REG_FOG: return "FOG";
		case GIF_A_D_REG_XYZF3: return "XYZF3";
		case GIF_A_D_REG_XYZ3: return "XYZ3";
		case GIF_A_D_REG_NOP: return "NOP";
		case GIF_A_D_REG_TEX1_1: return "TEX1_1";
		case GIF_A_D_REG_TEX1_2: return "TEX1_2";
		case GIF_A_D_REG_TEX2_1: return "TEX2_1";
		case GIF_A_D_REG_TEX2_2: return "TEX2_2";
		case GIF_A_D_REG_XYOFFSET_1: return "XYOFFSET_1";
		case GIF_A_D_REG_XYOFFSET_2: return "XYOFFSET_2";
		case GIF_A_D_REG_PRMODECONT: return "PRMODECONT";
		case GIF_A_D_REG_PRMODE: return "PRMODE";
		case GIF_A_D_REG_TEXCLUT: return "TEXCLUT";
		case GIF_A_D_REG_SCANMSK: return "SCANMSK";
		case GIF_A_D_REG_MIPTBP1_1: return "MIPTBP1_1";
		case GIF_A_D_REG_MIPTBP1_2: return "MIPTBP1_2";
		case GIF_A_D_REG_MIPTBP2_1: return "MIPTBP2_1";
		case GIF_A_D_REG_MIPTBP2_2: return "MIPTBP2_2";
		case GIF_A_D_REG_TEXA: return "TEXA";
		case GIF_A_D_REG_FOGCOL: return "FOGCOL";
		case GIF_A_D_REG_TEXFLUSH: return "TEXFLUSH";
		case GIF_A_D_REG_SCISSOR_1: return "SCISSOR_1";
		case GIF_A_D_REG_SCISSOR_2: return "SCISSOR_2";
		case GIF_A_D_REG_ALPHA_1: return "ALPHA_1";
		case GIF_A_D_REG_ALPHA_2: return "ALPHA_2";
		case GIF_A_D_REG_DIMX: return "DIMX";
		case GIF_A_D_REG_DTHE: return "DTHE";
		case GIF_A_D_REG_COLCLAMP: return "COLCLAMP";
		case GIF_A_D_REG_TEST_1: return "TEST_1";
		case GIF_A_D_REG_TEST_2: return "TEST_2";
		case GIF_A_D_REG_PABE: return "PABE";
		case GIF_A_D_REG_FBA_1: return "FBA_1";
		case GIF_A_D_REG_FBA_2: return "FBA_2";
		case GIF_A_D_REG_FRAME_1: return "FRAME_1";
		case GIF_A_D_REG_FRAME_2: return "FRAME_2";
		case GIF_A_D_REG_ZBUF_1: return "ZBUF_1";
		case GIF_A_D_REG_ZBUF_2: return "ZBUF_2";
		case GIF_A_D_REG_BITBLTBUF: return "BITBLTBUF";
		case GIF_A_D_REG_TRXPOS: return "TRXPOS";
		case GIF_A_D_REG_TRXREG: return "TRXREG";
		case GIF_A_D_REG_TRXDIR: return "TRXDIR";
		case GIF_A_D_REG_HWREG: return "HWREG";
		case GIF_A_D_REG_SIGNAL: return "SIGNAL";
		case GIF_A_D_REG_FINISH: return "FINISH";
		case GIF_A_D_REG_LABEL: return "LABEL";
		default: return "ERR";
	}
}

const char *gs_primitive_type_name(GsPrimitiveType prim)
{
	switch(prim) {
		case GSPRIM_POINT: return "POINT";
		case GSPRIM_LINE: return "LINE";
		case GSPRIM_LINE_STRIP: return "LINE_STRIP";
		case GSPRIM_TRIANGLE: return "TRIANGLE";
		case GSPRIM_TRIANGLE_STRIP: return "TRIANGLE_STRIP";
		case GSPRIM_TRIANGLE_FAN: return "TRIANGLE_FAN";
		case GSPRIM_PRITE: return "SPRITE";
		default: return "ERR";
	}
}

int bit_range(u64 val, int lo, int hi)
{
	return (val >> lo) & ((1 << (hi - lo + 1)) - 1);
}

#endif
