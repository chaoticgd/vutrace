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
	GsRegister reg;
	int source_address;
	u8 buffer[0x10];
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
					item.reg = prim.tag.regs[j];
					item.source_address = VU1_MEMSIZE - size + pos;
					memcpy(item.buffer, &data[pos], 0x10);
					pos += 0x10;
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
