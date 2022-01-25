// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// ImGui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_gl3.h>

// standard
#include <iostream>
#include <sstream>

#include "MyApp.h"

void exitProgram()
{
	SDL_Quit();

	std::cout << "Press a button to exit..." << std::endl;
	std::cin.get();
}

int main(int argc, char* args[])
{
	// By this setting, the system will call exitProgram before this process terminates
	// Question: What would happen without this?
	atexit(exitProgram);

	//
	// Step 1: initialize SDL
	//

	// Turn on the graphical subsystem
	// If there is an error, notify and exit
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		// Print the error and terminate
		std::cout << "[SDL start]Error during the initialization of SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	//
	// Step 2: set OpenGL requirements, create our window, start OpenGL
	//

	// 2a: Start-up configuration of OpenGL, this has to be done before the creation of any window
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	// Set the color depth (how many bits do we want to store red, green, blue and alpha(transparency) properties per pixel)
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	// Double buffering
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	// Size of depth buffer in bits
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Antialiasing - if needed
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,  1);
	//SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,  2);

	// Create our window
	SDL_Window *win = 0;
	win = SDL_CreateWindow("Hello SDL&OpenGL!",		// az ablak fejléce
		100,						// az ablak bal-felsõ sarkának kezdeti X koordinátája
		100,						// az ablak bal-felsõ sarkának kezdeti Y koordinátája
		640,						// ablak szélessége
		480,						// és magassága
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);			// megjelenítési tulajdonságok


// If the window creation failed, print the error and exit
	if (win == 0)
	{
		std::cout << "[Window creation]Error during the initialization of SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	//
	// Step 3: Create the OpenGL context: we will draw using this
	//

	SDL_GLContext	context = SDL_GL_CreateContext(win);
	if (context == 0)
	{
		std::cout << "[OGL context creation]Error during the initialization of SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	// Display: wait for vertical sync
	SDL_GL_SetSwapInterval(1);

	// Start GLEW
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		std::cout << "[GLEW] Error during the initialization!" << std::endl;
		return 1;
	}

	// Query the OpenGL version
	int glVersion[2] = { -1, -1 };
	glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
	glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);
	std::cout << "Running OpenGL " << glVersion[0] << "." << glVersion[1] << std::endl;

	if (glVersion[0] == -1 && glVersion[1] == -1)
	{
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(win);

		std::cout << "[OGL context creation] Error during the creation of the OpenGL context! One of the attributes at SDL_GL_SetAttribute(...) might be wrong." << std::endl;

		return 1;
	}

	std::stringstream window_title;
	window_title << "OpenGL " << glVersion[0] << "." << glVersion[1];
	SDL_SetWindowTitle(win, window_title.str().c_str());

	//Imgui init
	ImGui_ImplSdlGL3_Init(win);

	//
	// Step 4: Start the event loop
	// 
	{
		// Should the program end?
		bool quit = false;
		// Event to be processed
		SDL_Event ev;

		// Instance of the application
		CMyApp app;
		if (!app.Init())
		{
			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(win);
			std::cout << "[app.Init] Error during the initialization of the application!" << std::endl;
			return 1;
		}

		while (!quit)
		{
			// While there is an event to process, process all of them
			while (SDL_PollEvent(&ev))
			{
				ImGui_ImplSdlGL3_ProcessEvent(&ev);
				bool is_mouse_captured = ImGui::GetIO().WantCaptureMouse; // Do we need mouse for imgui?
				bool is_keyboard_captured = ImGui::GetIO().WantCaptureKeyboard;	// Do we need keyboard for imgui?
				switch (ev.type)
				{
				case SDL_QUIT:
					quit = true;
					break;
				case SDL_KEYDOWN:
					if (ev.key.keysym.sym == SDLK_ESCAPE)
						quit = true;
					if (!is_keyboard_captured)
						app.KeyboardDown(ev.key);
					break;
				case SDL_KEYUP:
					if (!is_keyboard_captured)
						app.KeyboardUp(ev.key);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (!is_mouse_captured)
						app.MouseDown(ev.button);
					break;
				case SDL_MOUSEBUTTONUP:
					if (!is_mouse_captured)
						app.MouseUp(ev.button);
					break;
				case SDL_MOUSEWHEEL:
					if (!is_mouse_captured)
						app.MouseWheel(ev.wheel);
					break;
				case SDL_MOUSEMOTION:
					if (!is_mouse_captured)
						app.MouseMove(ev.motion);
					break;
				case SDL_WINDOWEVENT:
					if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
					{
						app.Resize(ev.window.data1, ev.window.data2);
					}
					break;
				}

			}
			ImGui_ImplSdlGL3_NewFrame(win); //After this we can call imgui commands until ImGui::Render()

			app.Update();
			app.Render();
			ImGui::Render();

			SDL_GL_SwapWindow(win);
		}

		// The object should clean after itself
		app.Clean();
	}	// the destructor of the app will run while our context is alive => destructors of classes including the GPU resources will run here too

	//
	// Step 4: exit
	// 
	ImGui_ImplSdlGL3_Shutdown();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);

	return 0;
}