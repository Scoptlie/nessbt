#include "interface.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#include <GL/gl3w.h>
#include <SDL2/SDL.h>

#include "runtime/ppu.h"

namespace Interface {
	bool quitRequest;
	
	SDL_Window *window;
	SDL_GLContext glContext;
	
	U32 program;
	I32 programUTex;
	U32 vertArray, vertBuf;
	
	U32 frame;
	
	void update() {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				exit(0);
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		int winW, winH;
		SDL_GetWindowSize(window, &winW, &winH);
		
		auto scale = std::min(winW / 256.0, winH / 240.0);
		auto vpW = int(256.0 * scale);
		auto vpH = int(240.0 * scale);
		auto vpX = (winW - vpW) / 2;
		auto vpY = (winH - vpH) / 2;
		glViewport(vpX, vpY, vpW, vpH);
		
		glBindTexture(GL_TEXTURE_2D, frame);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RGB, GL_UNSIGNED_BYTE, Ppu::colourBuf);
		
		glUseProgram(program);
		glUniform1i(programUTex, 0);
		
		glBindVertexArray(vertArray);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		SDL_GL_SwapWindow(window);
	}
	
	void init() {
		SDL_Init(SDL_INIT_VIDEO);
		
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
		
		window = SDL_CreateWindow(
			"",
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			256*3, 240*3,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		);
		
		glContext = SDL_GL_CreateContext(window);
		
		SDL_GL_SetSwapInterval(0);
		
		gl3wInit();
		
		auto createShader = [](int type, char const *source) {
			auto r = glCreateShader(type);
			glShaderSource(r, 1, &source, nullptr);
			glCompileShader(r);
			
			int compiled;
			glGetShaderiv(r, GL_COMPILE_STATUS, &compiled);
			if (!compiled) {
				int logLen;
				glGetShaderiv(r, GL_INFO_LOG_LENGTH, &logLen);
				
				auto log = new char[logLen + 1];
				glGetShaderInfoLog(r, logLen, &logLen, log);
				log[logLen] = '\0';
				
				puts(log);
				
				__debugbreak();
				exit(1);
			}
			
			return r;
		};
		
		auto vertShader = createShader(GL_VERTEX_SHADER,
			"#version 120\n"
			"attribute vec2 vUv;"
			"varying vec2 fUv;"
			"void main(){"
				"gl_Position=vec4((vUv*2.0-vec2(1.0,1.0))*vec2(1.0,-1.0),0.0,1.0);"
				"fUv=vUv;"
			"}"
		);
		
		auto fragShader = createShader(GL_FRAGMENT_SHADER,
			"#version 120\n"
			"varying vec2 fUv;"
			"uniform sampler2D uTex;"
			"void main(){"
				"gl_FragColor=texture2D(uTex,fUv);"
			"}"
		);
		
		program = glCreateProgram();
		glAttachShader(program, vertShader);
		glAttachShader(program, fragShader);
		glLinkProgram(program);
		
		int linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked) {
			int logLen;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
			
			auto log = new char[logLen + 1];
			glGetProgramInfoLog(program, logLen, &logLen, log);
			log[logLen] = '\0';
			
			puts(log);
			
			__debugbreak();
			exit(1);
		}
		
		programUTex = glGetUniformLocation(program, "uTex");
		
		glGenVertexArrays(1, &vertArray);
		glBindVertexArray(vertArray);
		
		float vertBufData[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f,
			1.0f, 0.0f,
			0.0f, 1.0f
		};
		
		glGenBuffers(1, &vertBuf);
		glBindBuffer(GL_ARRAY_BUFFER, vertBuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertBufData), vertBufData, GL_STATIC_DRAW);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		glGenTextures(1, &frame);
		glBindTexture(GL_TEXTURE_2D, frame);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 240, 0, GL_RGB, GL_UNSIGNED_BYTE, Ppu::colourBuf);
	}
}
