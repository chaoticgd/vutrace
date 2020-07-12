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

#include <cstring>
#include <iomanip>
#include <stdio.h>

#include "pcsx2disassemble.h"

int main(int argc, char **argv)
{
	if(argc < 2) {
		fprintf(stderr, "Too few arguments.\n");
		exit(1);
	}
	
	FILE *microcode = fopen(argv[1], "rb");
	if(microcode == nullptr) {
		fprintf(stderr, "Cannot open file.\n");
		exit(1);
	}
	
	u8 instruction_pair[8];
	for(u32 i = 0; fread(instruction_pair, 8, 1, microcode) == 1; i += 8) {
		printf("%s\n", disassemble(instruction_pair, i).c_str());
	}
	
	fclose(microcode);
}
