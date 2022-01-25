#include "MyApp.h"

#include <math.h>
#include <vector>

#include <array>
#include <list>
#include <tuple>

#include "imgui\imgui.h"
#include "ObjParser_OGL3.h"

CMyApp::CMyApp(void){}

CMyApp::~CMyApp(void){}

bool CMyApp::Init()
{
	glClearColor(0.2, 0.4, 0.7, 1);	// Clear color is bluish
	glEnable(GL_CULL_FACE);			// Drop faces looking backwards
	glEnable(GL_DEPTH_TEST);		// Enable depth test

	m_program.Init({			// Shader for drawing geometries
		{ GL_VERTEX_SHADER,   "shaders/myVert.vert" },
		{ GL_FRAGMENT_SHADER, "shaders/myFrag.frag" }
	}/*,{						// This part is now shader defined!!
		{ 0, "vs_in_pos"	},	// VAO index 0 will be vs_in_pos
		{ 1, "vs_in_normal" },	// VAO index 1 will be vs_in_normal
		{ 2, "vs_out_tex0"	},	// VAO index 2 will be vs_in_tex0
	}*/);

	m_deferredPointlight.Init({ // A deferred shader for point lights
		{ GL_VERTEX_SHADER,		"shaders/deferredPoint.vert" },
		{ GL_FRAGMENT_SHADER,	"shaders/deferredPoint.frag" }
	});

	// Loading assets for the nightclub scene
	m_scene_texture.FromFile("assets/scene colors.bmp");
	m_mesh = ObjParser::parse("assets/Night club scene all textured unified.obj");

	// Camera
	// Blender to OGLBase: (x,y,z) -> (x,z,-y)
	m_camera.SetView(glm::vec3(0.0f, 6.0f, 0.0f), glm::vec3(0.0f, 6.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	m_camera.SetProj(45.0f, 640.0f / 480.0f, 0.01f, 1000.0f);

	// FBO - initial
	CreateFrameBuffer(640, 480);

	return true;
}

void CMyApp::Clean()
{
	if (m_frameBufferCreated)
	{
		glDeleteTextures(1, &m_diffuseBuffer);
		glDeleteTextures(1, &m_normalBuffer);
		glDeleteTextures(1, &m_depthBuffer);
		glDeleteFramebuffers(1, &m_frameBuffer);
	}
}

void CMyApp::Update()
{
	static Uint32 last_time = SDL_GetTicks();
	float delta_time = (SDL_GetTicks() - last_time) / 1000.0f;

	m_camera.Update(delta_time);

	last_time = SDL_GetTicks();
}

void CMyApp::DrawNightclubScene(const glm::mat4& viewProj, ProgramObject& program)
{
	program.Use();

	program.SetTexture("texImage", 0, m_scene_texture);
	glm::mat4 nightClubWorld = glm::mat4(1);
	program.SetUniform("MVP", viewProj * nightClubWorld);
	program.SetUniform("world", nightClubWorld);
	program.SetUniform("worldIT", glm::transpose(glm::inverse(nightClubWorld)));
	program.SetUniform("Kd", glm::vec4(1, 0.3, 0.3, 1));
	m_mesh->draw();

	program.Unuse();
}

void CMyApp::Render()
{
	// 1. Render to the framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	DrawNightclubScene(m_camera.GetViewProj(), m_program);

	// 2. Draw Lights by additions
	
	// 2.1. Setting up the blending
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0, 0, 0, 1);		// Clear to black
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST);		// Depth test is not performed -- all fragments should add color
	glDepthMask(GL_FALSE);			// Depth values are not need to be written
	glEnable(GL_BLEND);				// Instead of overwriting pixels in the FBO we
	glBlendEquation(GL_FUNC_ADD);	// perform addition for each pixel and thus
	glBlendFunc(GL_ONE, GL_ONE);	// summing the contribution of each light source

	// 2.2.1. The lamps on the wall:
	float wall_lamp_height = 6.71f;
	m_deferredPointlight.Use();
	m_deferredPointlight.SetTexture("diffuseTexture", 0, m_diffuseBuffer);
	m_deferredPointlight.SetTexture("normalTexture", 1, m_normalBuffer);
	m_deferredPointlight.SetTexture("depthTexture", 2, m_depthBuffer);
	m_deferredPointlight.SetUniform("projMatrixInv", glm::inverse(m_camera.GetProj()));
	m_deferredPointlight.SetUniform("viewMatrixInv", glm::inverse(m_camera.GetViewMatrix()));
	m_deferredPointlight.SetUniform("in_eye_pos", m_camera.GetEye());
	m_deferredPointlight.SetUniform("Ld", glm::vec4(0.3f, 0.0f, 0.3f, 1));
	m_deferredPointlight.SetUniform("brightness", 100.0f);

	m_deferredPointlight.SetUniform("lightPos", glm::vec3(26.25f, wall_lamp_height, -6.69f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(26.25f, wall_lamp_height, 1.16f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(26.25f, wall_lamp_height, 8.99f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(19.92f, wall_lamp_height, 24.29f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(11.98f, wall_lamp_height, 24.29f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(-12.07f, wall_lamp_height, 24.29f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(-19.99f, wall_lamp_height, 24.29f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(-27.92f, wall_lamp_height, 24.29f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	// 2.2.2. Also light up the bar shelves:
	m_deferredPointlight.SetUniform("Ld", glm::vec4(0.3f, 0.3f, 0.3f, 1));

	m_deferredPointlight.SetUniform("lightPos", glm::vec3(-31.6f, 5.61f, 5.95f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(-31.6f, 5.61f, 1.0f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(-31.6f, 5.61f, -3.54f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(-31.6f, 3.42f, 5.95f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(-31.6f, 3.42f, 1.0f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	m_deferredPointlight.SetUniform("lightPos", glm::vec3(-31.6f, 3.42f, -3.54f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// 2.2.5. Backlighting for the stage:
	m_deferredPointlight.SetUniform("Ld", glm::vec4(0.26f, 0.96f, 0.8f, 1));

	m_deferredPointlight.SetUniform("lightPos", glm::vec3(0.01f, 5.3f, -22.0f));
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// 2.2.4. The dance floor tiles:
	m_deferredPointlight.SetUniform("brightness", 75.0f);
	int vertical_index = 0;
	int horizontal_index = 0;
	for (float dance_floor_panel_x = -14.5f; dance_floor_panel_x <= 14.5f; dance_floor_panel_x += 3.22f, vertical_index++) {
		horizontal_index = 0;
		for (float dance_floor_panel_z = -14.5f; dance_floor_panel_z <= 14.5f; dance_floor_panel_z += 3.22f, horizontal_index++) {
			m_deferredPointlight.SetUniform("lightPos", glm::vec3(dance_floor_panel_x, 1.0f, dance_floor_panel_z));
			if ((vertical_index + horizontal_index) % 2 == 0) {
				m_deferredPointlight.SetUniform("Ld", glm::vec4(0.1f, 0.0f, 0.1f, 1));
			}
			else {
				m_deferredPointlight.SetUniform("Ld", glm::vec4(0.0f, 0.0f, 0.1f, 1));
			}
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}
	}
	m_deferredPointlight.Unuse();

	// 2.3. Undo the blending options
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	// 3. User Interface
	//ImGui::ShowTestWindow(); // Demo of all ImGui commands. See its implementation for details.
		// It's worth browsing imgui.h, as well as reading the FAQ at the beginning of imgui.cpp.
		// There is no regular documentation, but the things mentioned above should be sufficient.

	/*
	ImGui::SetNextWindowPos(ImVec2(300, 400), ImGuiSetCond_FirstUseEver);
	if(ImGui::Begin("Test window")) // Note that ImGui returns false when window is collapsed so we can early-out
	{
		ImGui::Image((ImTextureID)m_diffuseBuffer, ImVec2(256, 256), ImVec2(0,1),  ImVec2(1,0)); ImGui::SameLine();
		ImGui::Image((ImTextureID)m_normalBuffer , ImVec2(256, 256), ImVec2(0,1),  ImVec2(1,0)); ImGui::SameLine();
		ImGui::Image((ImTextureID)m_depthBuffer  , ImVec2(256, 256), ImVec2(0,1),  ImVec2(1,0));
	}
	ImGui::End(); // In either case, ImGui::End() needs to be called for ImGui::Begin().
		// Note that other commands may work differently and may not need an End* if Begin* returned false.
	*/
}

void CMyApp::KeyboardDown(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardDown(key);
}

void CMyApp::KeyboardUp(SDL_KeyboardEvent& key)
{
	m_camera.KeyboardUp(key);
}

void CMyApp::MouseMove(SDL_MouseMotionEvent& mouse)
{
	m_camera.MouseMove(mouse);
}

void CMyApp::MouseDown(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseUp(SDL_MouseButtonEvent& mouse)
{
}

void CMyApp::MouseWheel(SDL_MouseWheelEvent& wheel)
{
}

// _w and _h are the width and height of the window's size
void CMyApp::Resize(int _w, int _h)
{
	glViewport(0, 0, _w, _h );
	m_camera.Resize(_w, _h);
	CreateFrameBuffer(_w, _h);
}

inline void setTexture2DParameters(GLenum magfilter = GL_LINEAR, GLenum minfilter = GL_LINEAR, GLenum wrap_s = GL_CLAMP_TO_EDGE, GLenum wrap_t = GL_CLAMP_TO_EDGE)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
}

void CMyApp::CreateFrameBuffer(int width, int height)
{
	// Clear if the function is not being called for the first time
	if (m_frameBufferCreated)
	{
		glDeleteTextures(1, &m_diffuseBuffer);
		glDeleteTextures(1, &m_normalBuffer);
		glDeleteTextures(1, &m_depthBuffer);
		glDeleteFramebuffers(1, &m_frameBuffer);
	}

	glGenFramebuffers(1, &m_frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuffer);

	// 1.  Diffuse colors
	glGenTextures(1, &m_diffuseBuffer);
	glBindTexture(GL_TEXTURE_2D, m_diffuseBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_diffuseBuffer, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating color attachment 0" << std::endl;
		exit(1);
	}

	// 2.  Normal vectors
	glGenTextures(1, &m_normalBuffer);
	glBindTexture(GL_TEXTURE_2D, m_normalBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_FLOAT, nullptr);	// Has to be GL_RGBA8 because of my GPU.
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normalBuffer, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating color attachment 1" << std::endl;
		exit(1);
	}

	// 3. Depth texture
	glGenTextures(1, &m_depthBuffer);
	glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);
	setTexture2DParameters(GL_NEAREST, GL_NEAREST);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthBuffer, 0);
	if (glGetError() != GL_NO_ERROR) {
		std::cout << "Error creating depth texture." << std::endl;
		exit(1);
	}

	// Specifying which color outputs are active
	GLenum drawBuffers[2] = {GL_COLOR_ATTACHMENT0,
							 GL_COLOR_ATTACHMENT1};
	glDrawBuffers(2, drawBuffers);

	// -- Completeness check
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Incomplete framebuffer (";
		switch (status) {
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";		 break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED:					std::cout << "GL_FRAMEBUFFER_UNSUPPORTED";					 break;
		}
		std::cout << ")" << std::endl;
		exit(1);
	}

	// -- Unbind framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	m_frameBufferCreated = true;
}