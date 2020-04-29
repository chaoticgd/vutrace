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

#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>

#include "pcsx2defs.h"
#include "pcsx2disassemble.h"

struct Snapshot
{
	VURegs registers;
	u8 memory[VU1_MEMSIZE];
	u8 program[VU1_PROGSIZE];
	int disassembly;
	int framebuffer;
};

struct AppState
{
	std::size_t current_snapshot = 0;
	std::vector<Snapshot> snapshots;
	std::vector<std::vector<std::string>> disassemblies;
	std::vector<GLuint> framebuffer_textures;
};

void update_gui(AppState &app);
void snapshots_window(AppState &app);
void registers_window(AppState &app);
void memory_window(AppState &app);
void disassembly_window(AppState &app);
void framebuffer_window(AppState &app);
std::vector<Snapshot> parse_trace(AppState &app, std::string dir_path);
void init_gui(GLFWwindow **window);
void begin_docking();
void create_dock_layout(GLFWwindow *window);

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
	if(ImGui::Begin("Framebuffer")) framebuffer_window(app); ImGui::End();
}

void snapshots_window(AppState &app)
{
	ImVec2 size = ImGui::GetWindowSize();
	size.x -= 16;
	size.y -= 64;
	ImGui::PushItemWidth(-1);
	if(ImGui::ListBoxHeader("##snapshots", size)) {
		for(std::size_t i = 0; i < app.snapshots.size(); i++) {
			Snapshot& snap = app.snapshots[i];
			std::string str = std::to_string(i);
			if(ImGui::Selectable(str.c_str(), i == app.current_snapshot)) {
				app.current_snapshot = i;
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
		ImGui::Text("%s = %d", name, value.UL);
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
	
	static const int ROW_SIZE = 32;
	
	if(ImGui::BeginChild("##rows", ImVec2(0, (VU1_MEMSIZE / ROW_SIZE) * 18))) {
		ImDrawList *dl = ImGui::GetWindowDrawList();
		
		for(int i = 0; i < VU1_MEMSIZE / ROW_SIZE; i++) {
			u8 *data = current.memory + i * ROW_SIZE;
			u8 *last_data = last->memory + i * ROW_SIZE;
			for(int j = 0; j < ROW_SIZE; j++) {
				int val = data[j];
				std::stringstream hex;
				if(val < 0x10) hex << "0";
				hex << std::hex << val;
				
				ImVec2 hex_pos {
					ImGui::GetItemRectMin().x + (j * 5) / 4 * 18.f,
					ImGui::GetItemRectMin().y + i * 18.f
				};
				ImColor hex_col = ImColor(0.8f, 0.8f, 0.8f);
				if(val != last_data[j]) {
					hex_col = ImColor(1.f, 0.5f, 0.5f);
				}
				dl->AddText(hex_pos, hex_col, hex.str().c_str());
			}
			
			for(int j = 0; j < ROW_SIZE; j += 4) {	
				std::stringstream fp;
				fp << *(float*) &data[j];
				ImVec2 fp_pos {
					ImGui::GetItemRectMin().x + ((ROW_SIZE + j + 1) * 5) / 4 * 18.f,
					ImGui::GetItemRectMin().y + i * 18.f
				};
				ImColor fp_col = ImColor(0.8f, 0.8f, 0.8f);
				if(*(float*) &data[j] != *(float*) &last_data[j]) {
					fp_col = ImColor(1.f, 0.5f, 0.5f);
				}
				dl->AddText(fp_pos, fp_col, fp.str().c_str());
			}
		}
		ImGui::EndChild();
	}
}

void disassembly_window(AppState &app)
{
	Snapshot &current = app.snapshots[app.current_snapshot];
	for(std::size_t i = 0; i < VU1_PROGSIZE; i += 8) {
		std::stringstream upper;
		upper << std::hex << std::setw(8) << std::setfill('0') << i + 4 << ": ";
		upper << disassemble_upper(*(u32*) &current.program[i + 4], i + 4);
		ImGui::Text("%s", upper.str().c_str());
		
		std::stringstream lower;
		lower << std::hex << std::setw(8) << std::setfill('0') << i << ": ";
		lower << disassemble_lower(*(u32*) &current.program[i], i);
		ImGui::Text("%s", lower.str().c_str());
	}
}

void framebuffer_window(AppState &app)
{
	Snapshot &current = app.snapshots[app.current_snapshot];
	GLuint framebuffer_texture = app.framebuffer_textures[current.framebuffer];
	
	ImGui::Image((void*)(intptr_t) framebuffer_texture, ImVec2(512, 512));
}

std::vector<Snapshot> parse_trace(AppState &app, std::string dir_path)
{
	std::vector<Snapshot> snapshots;
	
	std::string vu1_file = dir_path + "/vu1.bin";
	
	FILE *trace = fopen(vu1_file.c_str(), "rb");
	if(trace == nullptr) {
		fprintf(stderr, "Error: Failed to read trace!\n");
		exit(1);
	}
	
	// My VURegs varies in size to their one.
	// Instead of fixing it, here's this horrible thing.
	static const int hack_size = sizeof(VURegs) - 0x10;
	
	static const int reg_pos = 0x10;
	static const int mem_pos = reg_pos + hack_size + 0x10;
	static const int prog_pos = mem_pos + VU1_MEMSIZE + 0x10;
		
	char buffer[prog_pos + VU1_PROGSIZE];
	while(fread(buffer, sizeof(buffer), 1, trace) == 1) {
		bool should_continue = false;
		while(memcmp(buffer, "FRAMEBUFFER=====", 0x10) == 0) {
			uint32_t* u32buffer = (uint32_t*) &buffer;
			int size = u32buffer[4];
			int width = u32buffer[5];
			int height = u32buffer[6];
			
			fseek(trace, -sizeof(buffer) + 0x10 + 0x4*3, SEEK_CUR);
			
			uint32_t* pixels = (uint32_t*) malloc(size - 0x8);
			fread(pixels, size - 0x8, 1, trace);
			
			GLuint texture_id;
			glGenTextures(1, &texture_id);
			glBindTexture(GL_TEXTURE_2D, texture_id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, pixels);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			app.framebuffer_textures.push_back(texture_id);
			free(pixels);
			
			should_continue = true;
			break;
		}
		
		if(should_continue) {
			continue;
		}
		
		if(memcmp(buffer, "REGISTERS=======", 0x10) != 0) {
			fprintf(stderr, "Error: REGISTERS signature missing at %lx!", ftell(trace));
			exit(1);
		}
		
		Snapshot current;
		memcpy(&current.registers, buffer + reg_pos, hack_size);
		//memset(&current.registers + hack_size, 0, 0x10);
		memcpy(current.memory, buffer + mem_pos, VU1_MEMSIZE);
		memcpy(current.program, buffer + prog_pos, VU1_PROGSIZE);
		current.disassembly = app.disassemblies.size() - 1;
		current.framebuffer = app.framebuffer_textures.size() - 1;
		snapshots.push_back(current);
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

void create_dock_layout(GLFWwindow *window) {
	ImGuiID dockspace_id = ImGui::GetID("dock_space");
	
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	
	ImGui::DockBuilderRemoveNode(dockspace_id);
	ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(width, height));
	
	ImGuiID top, memory;
	ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Up, 0.5f, &top, &memory);
	
	ImGuiID registers, middle;
	ImGui::DockBuilderSplitNode(top, ImGuiDir_Left, 1.f / 3.f, &registers, &middle);
	
	ImGuiID snapshots, right;
	ImGui::DockBuilderSplitNode(middle, ImGuiDir_Left, 0.2f, &snapshots, &right);
	
	ImGuiID disassembly, framebuffer;
	ImGui::DockBuilderSplitNode(right, ImGuiDir_Left, 0.4f, &disassembly, &framebuffer);
	
	ImGui::DockBuilderDockWindow("Registers", registers);
	ImGui::DockBuilderDockWindow("Snapshots", snapshots);
	ImGui::DockBuilderDockWindow("Disassembly", disassembly);
	ImGui::DockBuilderDockWindow("Framebuffer", framebuffer);
	ImGui::DockBuilderDockWindow("Memory", memory);
}
