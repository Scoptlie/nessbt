#include "Interface.h"

#include <cstdlib>

#include <GL/gl3w.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_sdl2.h>
#include <SDL2/SDL.h>

#include "ppu.h"

namespace Interface {
	bool quitRequest;
	
	SDL_Window *window;
	SDL_GLContext glContext;
	
	void update() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quitRequest = true;
			}
			
			ImGui_ImplSDL2_ProcessEvent(&event);
		}
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame(); {
			ImGui::Begin("Screen"); {
				ImGui::Image(ImTextureID(USize(Ppu::frame)), ImVec2(256.0f * 3.0f, 240.0f * 3.0f));
			}
			ImGui::End();
		}
		ImGui::Render();
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		SDL_GL_SwapWindow(window);
	}
	
	void init() {
		SDL_Init(SDL_INIT_VIDEO);
		
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
		
		window = SDL_CreateWindow(
			"",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			1200, 900,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		);
		
		glContext = SDL_GL_CreateContext(window);
		
		gl3wInit();
		
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |=
			ImGuiConfigFlags_NavEnableKeyboard |
			ImGuiConfigFlags_DockingEnable;
		
		ImGui_ImplSDL2_InitForOpenGL(window, glContext);
		ImGui_ImplOpenGL3_Init();
	}
}
