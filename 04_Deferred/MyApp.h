#pragma once

// C++ includes
#include <memory>

// GLEW
#include <GL/glew.h>

// SDL
#include <SDL.h>
#include <SDL_opengl.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform2.hpp>

#include "ProgramObject.h"
#include "BufferObject.h"
#include "VertexArrayObject.h"
#include "TextureObject.h"

#include "Mesh_OGL3.h"
#include "gCamera.h"

class CMyApp
{
public:
	CMyApp(void);
	~CMyApp(void);

	bool Init();
	void Clean();

	void Update();
	void Render();

	void KeyboardDown(SDL_KeyboardEvent&);
	void KeyboardUp(SDL_KeyboardEvent&);
	void MouseMove(SDL_MouseMotionEvent&);
	void MouseDown(SDL_MouseButtonEvent&);
	void MouseUp(SDL_MouseButtonEvent&);
	void MouseWheel(SDL_MouseWheelEvent&);
	void Resize(int, int);
protected:
	gCamera				m_camera;

	// variables for shaders
	ProgramObject		m_program;				// basic program for shaders
	ProgramObject		m_deferredPointlight;	// A deffered shader program to draw point lightsources

	// assets
	Texture2D			m_scene_texture;
	std::unique_ptr<Mesh>	m_mesh;

	// stuffs for the FBO
	bool m_frameBufferCreated{ false };
	GLuint m_frameBuffer;
	GLuint m_diffuseBuffer;
	GLuint m_normalBuffer;
	GLuint m_depthBuffer;

	// FBO creating function
	void CreateFrameBuffer(int width, int height);
	void DrawNightclubScene(const glm::mat4& viewProj, ProgramObject& program);
};

