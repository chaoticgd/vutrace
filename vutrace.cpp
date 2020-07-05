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

static const u32 I_BIT = 1 << 31;
static const u32 E_BIT = 1 << 30;
static const u32 M_BIT = 1 << 29;
static const u32 D_BIT = 1 << 28;
static const u32 T_BIT = 1 << 27;

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
std::vector<Snapshot> parse_trace(AppState &app, std::string dir_path);
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
		fprintf(stderr, "You must specify a trace directory.\n");
		return 1;
	}
	
	GLFWwindow *window;
	int width, height;
	init_gui(&window);
	
	AppState app;
	app.snapshots = parse_trace(app, argv[1]);

	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

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
				std::string disassembly = disassemble_lower(lower, pc);
				return disassembly[0] == 'X' && disassembly[1] == 'G';
			};
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	
	ImVec2 size = ImGui::GetWindowSize();
	size.x -= 16;
	size.y -= 64;
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
			if(ImGui::Selectable(ss.str().c_str(), is_selected)) {
				app.current_snapshot = i;
				app.disassembly_scroll_to = true;
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
	
	auto draw_vec_reg = [](const char *name, VECTOR value) {
		ImGui::Text("%s = %.4f %.4f %.4f %.4f",
			name, value.F[0], value.F[1], value.F[2], value.F[3]);
	};
	
	auto draw_int_reg = [](const char *name, REG_VI value) {
		ImGui::Text("%s = 0x%x = %d", name, value.UL, value.UL);
	};
	
	ImGui::Columns(2);
		for(int i = 0; i < 32; i++) {
			std::string name = std::string("vf") + std::to_string(i);
			draw_vec_reg(name.c_str(), regs.VF[i]);
		}
	ImGui::NextColumn();
	ImGui::SetColumnWidth(1, 192);
	ImGui::SetColumnOffset(1, ImGui::GetWindowSize().x - 192);
		for(int i = 0; i < 16; i++) {
			std::string name = std::string("vi") + std::to_string(i);
			draw_int_reg(name.c_str(), regs.VI[i]);
		}
		draw_int_reg("Status", regs.VI[16]);
		draw_int_reg("MACflag", regs.VI[17]);
		draw_int_reg("ClipFlag", regs.VI[18]);
		draw_int_reg("c2c19", regs.VI[19]);
		draw_int_reg("R", regs.VI[20]);
		draw_int_reg("I", regs.VI[21]);
		draw_int_reg("Q", regs.VI[22]);
		draw_int_reg("c2c23", regs.VI[23]);
		draw_int_reg("c2c24", regs.VI[24]);
		draw_int_reg("c2c25", regs.VI[25]);
		draw_int_reg("TPC", regs.VI[26]);
		draw_int_reg("CMSAR0", regs.VI[27]);
		draw_int_reg("FBRST", regs.VI[28]);
		draw_int_reg("VPU-STAT", regs.VI[29]);
		draw_int_reg("c2c30", regs.VI[30]);
		draw_int_reg("CMSAR1", regs.VI[31]);
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
			dl->AddText(ImVec2(8, pos_y), row_header_col, row_header.str().c_str());
			
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
	
	for(std::size_t i = 0; i < VU1_PROGSIZE; i += 8) {
		Instruction instruction = app.instructions[i / 8];
		bool is_pc = current.registers.VI[TPC].UL == i;
		ImGuiSelectableFlags flags = instruction.is_executed ?
			ImGuiSelectableFlags_None :
			ImGuiSelectableFlags_Disabled;
		
		u32 upper = *(u32*) &current.program[i + 4];
		u32 lower = *(u32*) &current.program[i];
		
		std::stringstream ss;
		ss << std::hex << std::setw(4) << std::setfill('0') << i + 4 << ": (";
		ss << std::hex << std::setw(8) << std::setfill('0') << upper << ") ";
		ss << disassemble_upper(upper, i + 4);
		if(upper & I_BIT) ss << " [I]";
		if(upper & E_BIT) ss << " [E]";
		if(upper & M_BIT) ss << " [M]";
		if(upper & D_BIT) ss << " [D]";
		if(upper & T_BIT) ss << " [T]";
		while(ss.str().size() < 64) ss << " ";
		ss << std::hex << std::setw(4) << std::setfill('0') << i << ": (";
		ss << std::hex << std::setw(8) << std::setfill('0') << lower << ") ";
		if(upper & I_BIT) {
			ss << *(float*) &lower;
		} else {
			ss << disassemble_lower(lower, i);
		}
		
		if(instruction.branch_from_times.size() > 0) {
			std::stringstream addresses;
			std::size_t fallthrough_times = app.instructions[i / 8 + 1].times_executed;
			for(auto &[addr, times] : instruction.branch_from_times) {
				addresses << std::hex << addr << " (" << std::dec << times << ") ";
				fallthrough_times -= times;
			} 
			ImGui::Text("  %s/ ft (%ld) ->", addresses.str().c_str(), fallthrough_times);
		}
		
		bool clicked = ImGui::Selectable(ss.str().c_str(), is_pc, flags);
		
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
			int walk;
			if(current.registers.VI[TPC].UL > i) {
				walk = -1;
			} else {
				walk = 1;
			}
			int j = app.current_snapshot;
			while(app.snapshots[j].registers.VI[TPC].UL != i) {
				j += walk;
				if(j < 0 || j >= app.snapshots.size()) {
					j = -1;
					break;
				}
			}
			if(j == -1) {
				j = app.current_snapshot;
				while(app.snapshots[j].registers.VI[TPC].UL != i) {
					j -= walk;
					if(j < 0 || j >= app.snapshots.size()) {
						j = -1;
						break;
					}
				}
			}
			if(j != -1) {
				app.current_snapshot = j;
				app.snapshots_scroll_to = true;
				app.disassembly_scroll_to = true;
			}
		}
	}
}

void gs_packet_window(AppState &app)
{
	ImGui::Columns(2);
	
	static std::string address_hex;
	ImGui::InputText("Address", &address_hex);
	
	if(address_hex.size() == 0) {
		return;
	}
	
	std::size_t address = from_hex(address_hex);
	if(address < 0) address = 0;
	if(address > VU1_MEMSIZE) address = VU1_MEMSIZE;
	
	Snapshot &snap = app.snapshots[app.current_snapshot];
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
	for(const GsPackedData& data : prim.packed_data) {
		ImGui::Text("%x: %6s", data.source_address, gs_register_name(data.reg));
		ImGui::SameLine();
		for(std::size_t i = 0; i < 0x10; i += 4) {
			ImGui::Text("%02x%02x%02x%02x",
				data.buffer[i + 0],
				data.buffer[i + 1],
				data.buffer[i + 2],
				data.buffer[i + 3]);
			ImGui::SameLine();
		}
		ImGui::NewLine();
	}
	ImGui::EndChild();
}

enum VUTracePacketType {
	VUTRACE_PUSHSNAPSHOT = 'P', // Next packet directly follows.
	VUTRACE_SETREGISTERS = 'R', // VURegs struct follows (32-bit pointers).
	VUTRACE_SETMEMORY = 'M', // 16k memory follows.
	VUTRACE_SETINSTRUCTIONS = 'I', // 16k micromem follows.
	VUTRACE_LOADOP = 'L', // u32 address, u32 size follows.
	VUTRACE_STOREOP = 'S' // u32 address, u32 size follows.
};

std::vector<Snapshot> parse_trace(AppState &app, std::string dir_path)
{
	std::vector<Snapshot> snapshots;
	
	std::string vu1_file = dir_path + "/vu1.bin";
	
	FILE *trace = fopen(vu1_file.c_str(), "rb");
	if(trace == nullptr) {
		fprintf(stderr, "Error: Failed to read trace!\n");
		exit(1);
	}
	
	static const int reg_pos = 0x10;
	static const int mem_pos = reg_pos + sizeof(VURegs) + 0x10;
	static const int prog_pos = mem_pos + VU1_MEMSIZE + 0x10;
	
	app.instructions.resize(VU1_PROGSIZE / 8);
	
	auto check_eof = [](int n) {
		if(n != 1) {
			fprintf(stderr, "Error: Unexpected end of file.\n");
			exit(1);
		}
	};
	
	Snapshot current;
	VUTracePacketType packet_type;
	while(fread(&packet_type, 1, 1, trace) == 1) {
		switch(packet_type) {
			case VUTRACE_PUSHSNAPSHOT: {
				snapshots.push_back(current);
				
				u32 pc = current.registers.VI[TPC].UL;
				Instruction &instruction = app.instructions[pc / 8];
				instruction.is_executed = true;
				
				Snapshot &last = *(snapshots.end() - 2);
				u32 last_pc = last.registers.VI[TPC].UL;
				if(last_pc + 8 != pc) {
					// A branch has taken place.
					app.instructions[last_pc / 8].branch_to_times[pc]++;
					instruction.branch_from_times[last_pc]++;
				}
				instruction.times_executed++;
				
				current = {};
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
	
	return snapshots;
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
	ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.5f, &top, &bottom);
	
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
