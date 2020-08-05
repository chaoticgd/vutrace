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

#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include "pcsx2defs.h"
#include "pcsx2disassemble.h"
#include "gif.h"

static const int INSN_PAIR_SIZE = 8;

struct Snapshot
{
	VURegs registers;
	u8 memory[VU1_MEMSIZE];
	u8 program[VU1_PROGSIZE];
	int disassembly;
	u32 read_addr = 0;
	u32 read_size = 0;
	u32 write_addr = 0;
	u32 write_size = 0;
};

struct Instruction
{
	bool is_executed = false;
	std::map<u32, std::size_t> branch_to_times;
	std::map<u32, std::size_t> branch_from_times;
	std::size_t times_executed = 0;
};

struct AppState
{
	std::size_t current_snapshot = 0;
	std::vector<Snapshot> snapshots;
	bool snapshots_scroll_to = false;
	bool disassembly_scroll_to = false;
	std::vector<Instruction> instructions;
	std::string disassembly_highlight;
	std::string trace_file_path;
	bool comments_loaded = false;
	std::string comment_file_path;
	std::array<std::string, VU1_PROGSIZE / INSN_PAIR_SIZE> comments;
};

struct MessageBoxState
{
	bool is_open = false;
	std::string text;
};

void update_gui(AppState &app);
void snapshots_window(AppState &app);
void registers_window(AppState &app);
void memory_window(AppState &app);
void disassembly_window(AppState &app);
void gs_packet_window(AppState &app);
bool walk_until_pc_equal(AppState &app, u32 target_pc, int step); // Add step to the current snapshot index until pc == target_pc. If not found return false.
void parse_trace(AppState &app, std::string trace_file_path);
void parse_comment_file(AppState &app, std::string comment_file_path);
void save_comment_file(AppState &app);
std::string disassemble(u8 *program, u32 address);
bool is_xgkick(u32 lower);
void init_gui(GLFWwindow **window);
void begin_docking();
void create_dock_layout(GLFWwindow *window);
void alert(MessageBoxState &state, const char *title);
bool prompt(MessageBoxState &state, const char *title);
std::vector<u8> decode_hex(const std::string &in);
std::string to_hex(size_t n);
size_t from_hex(const std::string& in);

int main(int argc, char **argv)
{
	if(argc != 2) {
		fprintf(stderr, "You must specify a trace file.\n");
		return 1;
	}
	
	GLFWwindow *window;
	int width, height;
	init_gui(&window);
	
	AppState app;
	parse_trace(app, argv[1]);
	
	ImGuiContext &g = *GImGui;

	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		if(g.InputTextState.ID == 0 || g.InputTextState.ID != ImGui::GetActiveID()) {
			if(ImGui::IsKeyPressed('W') && app.current_snapshot > 0) {
				app.current_snapshot--;
				app.snapshots_scroll_to = true;
				app.disassembly_scroll_to = true;
			}
			if(ImGui::IsKeyPressed('S') && app.current_snapshot < app.snapshots.size() - 1) {
				app.current_snapshot++;
				app.snapshots_scroll_to = true;
				app.disassembly_scroll_to = true;
			}
			
			u32 pc = app.snapshots[app.current_snapshot].registers.VI[TPC].UL;
			if(ImGui::IsKeyPressed('A')) {
				walk_until_pc_equal(app, pc, -1);
			}
			if(ImGui::IsKeyPressed('D')) {
				walk_until_pc_equal(app, pc, 1);
			}
		}

		begin_docking();
		update_gui(app);
		static bool is_first_frame = true;
		if(is_first_frame) {
			create_dock_layout(window);
			is_first_frame = false;
		}
		ImGui::End(); // docking

		ImGui::Render();
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}
		
	glfwDestroyWindow(window);

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}

void update_gui(AppState &app)
{
	if(ImGui::Begin("Snapshots"))   snapshots_window(app);   ImGui::End();
	if(ImGui::Begin("Registers"))   registers_window(app);   ImGui::End();
	if(ImGui::Begin("Memory"))      memory_window(app);      ImGui::End();
	if(ImGui::Begin("Disassembly")) disassembly_window(app); ImGui::End();
	if(ImGui::Begin("GS Packet"))   gs_packet_window(app);   ImGui::End();
}

void snapshots_window(AppState &app)
{
	
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Iter:");
	ImGui::SameLine();
	u32 pc = app.snapshots[app.current_snapshot].registers.VI[TPC].UL;
	if(ImGui::Button(" < ")) {
		walk_until_pc_equal(app, pc, -1);
	}
	ImGui::SameLine();
	if(ImGui::Button(" > ")) {
		walk_until_pc_equal(app, pc, 1);
	}
	
	std::function<bool(Snapshot &)> filter;
	
	if(ImGui::BeginTabBar("tabs")) {
		if(ImGui::BeginTabItem("All")) {
			filter = [&](Snapshot &snapshot) { return true; };
			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("XGKICK")) {
			filter = [&](Snapshot &snapshot) {
				u32 pc = snapshot.registers.VI[TPC].UL;
				u32 lower = *(u32*) &snapshot.program[pc];
				return is_xgkick(lower);
			};
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	
	ImVec2 size = ImGui::GetWindowSize();
	size.x -= 16;
	size.y -= 82;
	ImGui::PushItemWidth(-1);
	if(ImGui::ListBoxHeader("##snapshots", size)) {
		for(std::size_t i = 0; i < app.snapshots.size(); i++) {
			Snapshot& snap = app.snapshots[i];
			Snapshot next_snap;
			if(i < app.snapshots.size() - 1) {
				next_snap = app.snapshots[i + 1];
			}
			bool is_selected = i == app.current_snapshot;
			
			if(!filter(snap)) {
				continue;
			}
			
			std::stringstream ss;
			ss << i;
			if(next_snap.read_size > 0) {
				ss << " READ 0x" << std::hex << next_snap.read_addr;
			} else if(next_snap.write_size > 0) {
				ss << " WRITE 0x" << std::hex << next_snap.write_addr;
			}
			
			u32 pc = snap.registers.VI[TPC].UL;
			std::string disassembly = disassemble(&snap.program[pc], pc);
			
			bool is_highlighted =
			app.disassembly_highlight.size() > 0 &&
				disassembly.find(app.disassembly_highlight) != std::string::npos;
		
			if(is_highlighted) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 0).Value);
			}
			if(ImGui::Selectable(ss.str().c_str(), is_selected)) {
				app.current_snapshot = i;
				app.disassembly_scroll_to = true;
			}
			if(is_highlighted) {
				ImGui::PopStyleColor();
			}
			
			if(app.snapshots_scroll_to && is_selected) {
				ImGui::SetScrollHere(0.5);
				app.snapshots_scroll_to = false;
			}
		}
		ImGui::ListBoxFooter();
	}
	ImGui::PopItemWidth();
}

void registers_window(AppState &app)
{
	Snapshot &current = app.snapshots[app.current_snapshot];
	VURegs &regs = current.registers;
	
	static bool show_as_hex = false;
	
	ImGui::Columns(2);
		ImGui::Checkbox("Show as Hex", &show_as_hex);
	
		for(int i = 0; i < 32; i++) {
			VECTOR value = regs.VF[i];
			if(show_as_hex) {
				ImGui::Text("vf%02d = %08x %08x %08x %08x",
					i, value.UL[0], value.UL[1], value.UL[2], value.UL[3]);
			} else {
				ImGui::Text("vf%02d = %.4f %.4f %.4f %.4f",
					i, value.F[0], value.F[1], value.F[2], value.F[3]);
			}
		}
	ImGui::NextColumn();
	ImGui::SetColumnWidth(1, 192);
	ImGui::SetColumnOffset(1, ImGui::GetWindowSize().x - 192);
		static const char *integer_register_names[] = {
			"vi00", "vi01", "vi02", "vi03",
			"vi04", "vi05", "vi06", "vi07",
			"vi08", "vi09", "vi10", "vi11",
			"vi12", "vi13", "vi14", "vi15",
			"Status", "MACflag", "ClipFlag", "c2c19",
			"R", "I", "Q", "c2c23",
			"c2c24", "c2c25", "TPC", "CMSAR0",
			"FBRST", "VPU-STAT", "c2c30", "CMSAR1",
		};
	
		for(int i = 0; i < 32; i++) {
			ImGui::Text("%s = 0x%x = %hd", integer_register_names[i], regs.VI[i].UL, regs.VI[i].UL);
		}
	ImGui::Columns(1);
}

void memory_window(AppState &app)
{
	Snapshot &current = app.snapshots[app.current_snapshot];
	Snapshot *last;
	if(app.current_snapshot > 0) {
		last = &app.snapshots[app.current_snapshot - 1];
	} else {
		last = &current;
	}
	
	static MessageBoxState found_bytes;
	alert(found_bytes, "Found Bytes");
	
	static MessageBoxState find_bytes;
	if(prompt(find_bytes, "Find Bytes") && !found_bytes.is_open) {
		std::vector<u8> value_raw = decode_hex(find_bytes.text);
		for(std::size_t i = 0; i < VU1_MEMSIZE - value_raw.size(); i++) {
			if(memcmp(value_raw.data(), &current.memory[i], value_raw.size()) == 0) {
				found_bytes.is_open = true;
				found_bytes.text = "Found match at 0x" + to_hex(i);
				break;
			}
		}
		if(!found_bytes.is_open) {
			found_bytes.is_open = true;
			found_bytes.text = "No match found";
		}
	}
	
	static const int ROW_SIZE = 32;
	
	ImGui::BeginChild("rows_outer");
	if(ImGui::BeginChild("rows", ImVec2(0, (VU1_MEMSIZE / ROW_SIZE) * 18))) {
		ImDrawList *dl = ImGui::GetWindowDrawList();
		
		for(int i = 0; i < VU1_MEMSIZE / ROW_SIZE; i++) {
			float pos_y = ImGui::GetItemRectMin().y + i * 18.f;
			
			static ImColor row_header_col = ImColor(1.f, 1.f, 1.f);
			std::stringstream row_header;
			row_header << std::hex << std::setfill('0') << std::setw(5) << i * ROW_SIZE;
			dl->AddText(ImVec2(ImGui::GetItemRectMin().x + 8, pos_y), row_header_col, row_header.str().c_str());
			
			u8 *data = current.memory + i * ROW_SIZE;
			u8 *last_data = last->memory + i * ROW_SIZE;
			for(int j = 0; j < ROW_SIZE; j++) {
				int val = data[j];
				std::stringstream hex;
				if(val < 0x10) hex << "0";
				hex << std::hex << val;
				
				ImVec2 hex_pos {
					56 + ImGui::GetItemRectMin().x + (j * 5) / 4 * 18.f, pos_y
				};
				ImColor hex_col = ImColor(0.8f, 0.8f, 0.8f);
				if(val != last_data[j]) {
					hex_col = ImColor(1.f, 0.5f, 0.5f);
				}
				dl->AddText(hex_pos, hex_col, hex.str().c_str());
			}
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}

void disassembly_window(AppState &app)
{
	Snapshot &current = app.snapshots[app.current_snapshot];
	
	ImGui::PushItemWidth(ImGui::GetWindowWidth() - 363);
	ImGui::InputText("Highlight", &app.disassembly_highlight);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	
	static MessageBoxState export_box;
	if(prompt(export_box, "Export Disassembly")) {
		std::ofstream disassembly_out_file(export_box.text);
		for(std::size_t i = 0; i < VU1_PROGSIZE; i+= INSN_PAIR_SIZE) {
			disassembly_out_file << disassemble(&current.program[i], i);
			if(app.comments.at(i / INSN_PAIR_SIZE).size() > 0) {
				disassembly_out_file << "; ";
			}
			disassembly_out_file << app.comments.at(i / INSN_PAIR_SIZE);
			disassembly_out_file << "\n";
		}
	}
	
	ImGui::SameLine();
	static MessageBoxState comment_box;
	if(prompt(comment_box, "Load Comment File")) {
		parse_comment_file(app, comment_box.text);
	}
	
	ImGui::BeginChild("disasm");
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, 768);
	
	for(std::size_t i = 0; i < VU1_PROGSIZE; i += INSN_PAIR_SIZE) {
		ImGui::PushID(i);
		
		Instruction instruction = app.instructions[i / INSN_PAIR_SIZE];
		bool is_pc = current.registers.VI[TPC].UL == i;
		ImGuiSelectableFlags flags = instruction.is_executed ?
			ImGuiSelectableFlags_None :
			ImGuiSelectableFlags_Disabled;
		
		std::string disassembly = disassemble(&current.program[i], i);
		
		if(instruction.branch_from_times.size() > 0) {
			std::stringstream addresses;
			std::size_t fallthrough_times = app.instructions[i / 8 + 1].times_executed;
			for(auto &[addr, times] : instruction.branch_from_times) {
				addresses << std::hex << addr << " (" << std::dec << times << ") ";
				fallthrough_times -= times;
			} 
			ImGui::Text("  %s/ ft (%ld) ->", addresses.str().c_str(), fallthrough_times);
		}
		
		bool is_highlighted =
			app.disassembly_highlight.size() > 0 &&
			disassembly.find(app.disassembly_highlight) != std::string::npos;
		
		if(is_highlighted) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 0).Value);
		}
		bool clicked = ImGui::Selectable(disassembly.c_str(), is_pc, flags);
		if(is_highlighted) {
			ImGui::PopStyleColor();
		}
		
		if(instruction.branch_to_times.size() > 0) {
			std::stringstream addresses;
			std::size_t fallthrough_times = instruction.times_executed;
			for(auto &[addr, times] : instruction.branch_to_times) {
				addresses << std::hex << addr << " (" << std::dec << times << ") ";
				fallthrough_times -= times;
			} 
			ImGui::Text("  -> %s/ ft (%ld)", addresses.str().c_str(), fallthrough_times);
		}
		
		if(is_pc && app.disassembly_scroll_to) {
			ImGui::SetScrollHere(0.5);
			app.disassembly_scroll_to = false;
		}
		
		if(!is_pc && clicked) {
			bool pc_changed = false;
			if(current.registers.VI[TPC].UL > i) {
				pc_changed = walk_until_pc_equal(app, i, -1);
				if(!pc_changed) {
					pc_changed = walk_until_pc_equal(app, i, 1);
				}
			} else {
				pc_changed = walk_until_pc_equal(app, i, 1);
				if(!pc_changed) {
					pc_changed = walk_until_pc_equal(app, i, -1);
				}
			}
			if(pc_changed) {
				app.disassembly_scroll_to = true;
			}
		}
		
		ImGui::NextColumn();
		if(!is_pc) {
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
		}
		ImVec2 comment_size(ImGui::GetWindowSize().x - 768.f, 14.f);
		std::string &comment = app.comments.at(i / INSN_PAIR_SIZE);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushItemWidth(-1);
		ImGuiInputTextFlags comment_flags = app.comments_loaded ?
			ImGuiInputTextFlags_None :
			ImGuiInputTextFlags_ReadOnly;
		if(ImGui::InputText("##comment", &comment, comment_flags)) {
			save_comment_file(app);
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		if(!is_pc) {
			ImGui::PopStyleColor();
		}
		ImGui::NextColumn();
		
		ImGui::PopID();
	}
	
	ImGui::Columns();
	ImGui::EndChild();
}

void gs_packet_window(AppState &app)
{
	ImGui::Columns(2);
	
	static std::string address_hex;
	ImGui::InputText("Address", &address_hex);
	
	Snapshot &snap = app.snapshots[app.current_snapshot];
	
	std::size_t address;
	if(address_hex.size() == 0) {
		u32 pc = snap.registers.VI[TPC].UL;
		u32 lower = *(u32*) &snap.program[pc];
		if(is_xgkick(lower)) {
			u32 is = bit_range(lower, 11, 15);
			address = snap.registers.VI[is].UL * 0x10;
		} else {
			return;
		}
	} else {
		address = from_hex(address_hex);
	}
	
	if(address < 0) address = 0;
	if(address > VU1_MEMSIZE) address = VU1_MEMSIZE;
	
	GsPacket packet = read_gs_packet(&snap.memory[address], VU1_MEMSIZE - address);
	
	ImGui::BeginChild("primlist");
	
	static std::size_t selected_primitive = 0;
	for(std::size_t i = 0; i < packet.primitives.size(); i++) {
		std::string label = std::to_string(i);
		if(ImGui::Selectable(label.c_str(), i == selected_primitive)) {
			selected_primitive = i;
		}
	}
	if(selected_primitive >= packet.primitives.size()) {
		selected_primitive = 0;
	}
	
	ImGui::EndChild();
	ImGui::NextColumn();
	
	if(packet.primitives.size() < 1) {
		return;
	}
	const GsPrimitive &prim = packet.primitives[selected_primitive];
	const GifTag &tag = prim.tag;
	
	ImGui::TextWrapped("NLOOP=%x, EOP=%x, PRE=%x, FLAG=%s, NREG=%lx\n",
		tag.nloop, tag.eop, tag.pre, gif_flag_name(tag.flag), tag.regs.size());
	
	ImGui::TextWrapped("PRIM: PRIM=%s, IIP=%s, TME=%d, FGE=%d, ABE=%d, AA1=%d, FST=%s, CTXT=%s, FIX=%d",
		gs_primitive_type_name(tag.prim.prim),
		tag.prim.iip == GSSHADE_FLAT ? "FLAT" : "GOURAUD",
		tag.prim.tme, tag.prim.fge, tag.prim.abe, tag.prim.aa1,
		tag.prim.fst == GSFST_STQ ? "STQ" : "UV",
		tag.prim.ctxt == GSCTXT_1 ? "FIRST" : "SECOND",
		tag.prim.fix);
	
	
	ImGui::TextWrapped("REGS:");
	ImGui::SameLine();
	for(std::size_t i = 0; i < tag.regs.size(); i++) {
		ImGui::TextWrapped("%s", gs_register_name(tag.regs[i]));
		ImGui::SameLine();
	}
	ImGui::NewLine();
	
	ImGui::BeginChild("data");
	for(const GsPackedData& item : prim.packed_data) {
		ImGui::Text("%x: %6s", item.source_address, gs_register_name(item.reg));
		ImGui::SameLine();
		switch(item.reg) {
			case GSREG_AD: {
				ImGui::Text("%s <- %lx\n", gs_register_name(item.ad.addr), item.ad.data);
				break;
			}
			default: {
				// Hex dump the raw data.
				for(std::size_t i = 0; i < 0x10; i += 4) {
					ImGui::Text("%02x%02x%02x%02x",
						item.buffer[i + 0],
						item.buffer[i + 1],
						item.buffer[i + 2],
						item.buffer[i + 3]);
					ImGui::SameLine();
				}
				ImGui::NewLine();
			}
		}
	}
	ImGui::EndChild();
}

bool walk_until_pc_equal(AppState &app, u32 target_pc, int step)
{
	std::size_t snapshot = app.current_snapshot;
	do {
		if(-step > (int) snapshot || snapshot + step >= app.snapshots.size()) {
			return false;
		}
		snapshot += step;
	} while(app.snapshots[snapshot].registers.VI[TPC].UL != target_pc);
	app.current_snapshot = snapshot;
	app.snapshots_scroll_to = true;
	return true;
}

enum VUTracePacketType {
	VUTRACE_PUSHSNAPSHOT = 'P', // Next packet directly follows.
	VUTRACE_SETREGISTERS = 'R', // VURegs struct follows (32-bit pointers).
	VUTRACE_SETMEMORY = 'M', // 16k memory follows.
	VUTRACE_SETINSTRUCTIONS = 'I', // 16k micromem follows.
	VUTRACE_LOADOP = 'L', // u32 address, u32 size follows.
	VUTRACE_STOREOP = 'S' // u32 address, u32 size follows.
};

void parse_trace(AppState &app, std::string trace_file_path)
{
	std::vector<Snapshot> snapshots;
	
	app.trace_file_path = trace_file_path;
	
	FILE *trace = fopen(trace_file_path.c_str(), "rb");
	if(trace == nullptr) {
		fprintf(stderr, "Error: Failed to read trace!\n");
		exit(1);
	}
	
	app.instructions.resize(VU1_PROGSIZE / INSN_PAIR_SIZE);
	
	auto check_eof = [](int n) {
		if(n != 1) {
			fprintf(stderr, "Error: Unexpected end of file.\n");
			exit(1);
		}
	};
	
	app.snapshots = {};
	
	Snapshot current;
	VUTracePacketType packet_type;
	while(fread(&packet_type, 1, 1, trace) == 1) {
		switch(packet_type) {
			case VUTRACE_PUSHSNAPSHOT: {
				app.snapshots.push_back(current);
				
				u32 pc = current.registers.VI[TPC].UL;
				Instruction &instruction = app.instructions[pc / INSN_PAIR_SIZE];
				instruction.is_executed = true;
				
				if(app.snapshots.size() >= 2) {
					Snapshot &last = app.snapshots.at(app.snapshots.size() - 2);
					u32 last_pc = last.registers.VI[TPC].UL;
					if(last_pc + INSN_PAIR_SIZE != pc) {
						// A branch has taken place.
						app.instructions[last_pc / INSN_PAIR_SIZE].branch_to_times[pc]++;
						instruction.branch_from_times[last_pc]++;
					}
				}
				instruction.times_executed++;
				
				current.read_addr = 0;
				current.read_size = 0;
				current.write_addr = 0;
				current.write_size = 0;
				break;
			}
			case VUTRACE_SETREGISTERS:
				check_eof(fread(&current.registers, sizeof(VURegs), 1, trace));
				break;
			case VUTRACE_SETMEMORY:
				check_eof(fread(current.memory, VU1_MEMSIZE, 1, trace));
				break;
			case VUTRACE_SETINSTRUCTIONS:
				check_eof(fread(current.program, VU1_PROGSIZE, 1, trace));
				break;
			case VUTRACE_LOADOP:
				check_eof(fread(&current.read_addr, sizeof(u32), 1, trace));
				check_eof(fread(&current.read_size, sizeof(u32), 1, trace));
				break;
			case VUTRACE_STOREOP:
				check_eof(fread(&current.write_addr, sizeof(u32), 1, trace));
				check_eof(fread(&current.write_size, sizeof(u32), 1, trace));
				break;
			default:
				fprintf(stderr, "Error: Invalid packet type 0x%x in trace file at 0x%lx!\n",
					packet_type, ftell(trace));
				exit(1);
		}
	}
	if(!feof(trace)) {
		fprintf(stderr, "Error: Failed to read trace!\n");
		exit(1);
	}
	
	fclose(trace);
}

void parse_comment_file(AppState &app, std::string comment_file_path) {
	app.comment_file_path = comment_file_path;
	std::ifstream comment_file(comment_file_path);
	if(comment_file) {
		std::string line;
		for(std::size_t i = 0; std::getline(comment_file, line) && i < VU1_PROGSIZE / INSN_PAIR_SIZE; i++) {
			app.comments[i] = line;
		}
		app.comments_loaded = true;
	}
}

void save_comment_file(AppState &app)
{
	std::ofstream comment_file(app.comment_file_path);
	for(std::size_t i = 0; i < app.comments.size(); i++) {
		comment_file << app.comments[i] << "\n";
	}
}

bool is_xgkick(u32 lower)
{
	return bit_range(lower, 0, 10) == 0b11011111100;
}

void init_gui(GLFWwindow **window)
{
	if(!glfwInit()) {
		fprintf(stderr, "Cannot load GLFW.");
		exit(1);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	*window = glfwCreateWindow(1280, 720, "vutrace", NULL, NULL);
	if(*window == nullptr) {
		fprintf(stderr, "Cannot create GLFW window.");
		exit(1);
	}

	glfwMaximizeWindow(*window);
	glfwMakeContextCurrent(*window);
	glfwSwapInterval(1); // vsync

	if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
		fprintf(stderr, "Cannot load GLAD.");
		exit(1);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = true;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(*window, true);
	ImGui_ImplOpenGL3_Init("#version 120");
}

void begin_docking() {
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	static bool p_open;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("dock_space", &p_open, window_flags);
	ImGui::PopStyleVar();
	
	ImGui::PopStyleVar(2);

	ImGuiID dockspace_id = ImGui::GetID("dock_space");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void create_dock_layout(GLFWwindow *window)
{
	ImGuiID dockspace_id = ImGui::GetID("dock_space");
	
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	
	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(width, height));
	
	ImGuiID top, bottom;
	ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.75f, &top, &bottom);
	
	ImGuiID registers, middle;
	ImGui::DockBuilderSplitNode(top, ImGuiDir_Left, 1.f / 3.f, &registers, &middle);
	
	ImGuiID snapshots, disassembly;
	ImGui::DockBuilderSplitNode(middle, ImGuiDir_Left, 0.2f, &snapshots, &disassembly);
	
	ImGuiID memory, gs_packet;
	ImGui::DockBuilderSplitNode(bottom, ImGuiDir_Left, 0.5f, &memory, &gs_packet);
	
	ImGui::DockBuilderDockWindow("Registers", registers);
	ImGui::DockBuilderDockWindow("Snapshots", snapshots);
	ImGui::DockBuilderDockWindow("Disassembly", disassembly);
	ImGui::DockBuilderDockWindow("Memory", memory);
	ImGui::DockBuilderDockWindow("GS Packet", gs_packet);
}

void alert(MessageBoxState &state, const char *title)
{
	if(state.is_open) {
		ImGui::SetNextWindowSize(ImVec2(400, 100));
		ImGui::Begin(title);
		ImGui::Text("%s", state.text.c_str());
		if(ImGui::Button("Close")) {
			state.text = "";
			state.is_open = false;
		}
		ImGui::End();
	}
}

bool prompt(MessageBoxState &state, const char *title)
{
	bool result = false;
	if(ImGui::Button(title)) {
		state.is_open = true;
		state.text = "";
	}
	if(state.is_open) {
		ImGui::SetNextWindowSize(ImVec2(400, 100));
		ImGui::Begin(title);
		ImGui::InputText("##input", &state.text);
		if(ImGui::Button("Okay")) {
			state.is_open = false;
			result = true;
		}
		ImGui::SameLine();
		if(ImGui::Button("Cancel")) {
			state.is_open = false;
		}
		ImGui::End();
	}
	return result;
}

std::vector<u8> decode_hex(const std::string &in)
{
	std::vector<u8> result;
	u8 current_byte;
	bool reading_second_nibble = false;
	for(char c : in) {
		u8 nibble = 0;
		if(c >= '0' && c <= '9') {
			nibble = c - '0';
		} else if(c >= 'A' && c <= 'Z') {
			nibble = c - 'A' + 0xa;
		} else if(c >= 'a' && c <= 'z') {
			nibble = c - 'a' + 0xa;
		} else {
			continue;
		}
		
		if(!reading_second_nibble) {
			current_byte = nibble << 4;
			reading_second_nibble = true;
		} else {
			current_byte |= nibble;
			result.push_back(current_byte);
			reading_second_nibble = false;
		}
		
	}
	return result;
}

std::string to_hex(size_t n)
{
	std::stringstream ss;
	ss << std::hex << n;
	return ss.str();
}

size_t from_hex(const std::string& in) {
	size_t result;
	std::stringstream ss;
	ss << std::hex << in;
	ss >> result;
	return result; 
}
