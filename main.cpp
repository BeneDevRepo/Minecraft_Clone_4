#include "Window/OpenGLWindow.h"

#include "glm/glm.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"

#include "OpenGL/opengl_include.h"
#include "OpenGL/Camera.h"
#include "OpenGL/ShaderProgram.h"
#include "OpenGL/StaticMesh.h"
#include "OpenGL/Texture.h"
#include "OpenGL/Cubemap.h"
#include "OpenGL/DebugRenderer.h"

// #include "Threading/mingw.thread.h"
// #include "Threading/mingw.mutex.h"

#include <iostream>
#include <cstdint>
#include <vector>

#include <string>


#include "World.h"
#include "Player.h"





// - Monitor Physik Engine
// - Audio (z.B. Console keyboard)

// - - Minecraft renderer / chunk system

// - Text
// - ECS

// - OpenGL Raytracer
// - Atmospheric Scattering
// - GUI
// - Clouds
// - Particle System (Compute Shader + Instanced Renderer)
// - Deferred Renderer
// - HDR + PBR
// - Image Based Lighting
// - Delete all Copyconstructors and CopyAssignmentOperators
// - use glVertexAttribDivisor to send vertex attributes per Primitive instead of per Vertex
// - use Shader Storage Buffer Objects for Lights etc.
// - use equirectangular HDRs for skymap + IBL
// - Use same VAO for all draw calls (or atl. reduce number of VAOs)

// - Better Block Picking:
// https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.42.3443&rep=rep1&type=pdf

// General Rendering:
/*
https://on-demand.gputechconf.com/gtc/2013/presentations/S3032-Advanced-Scenegraph-Rendering-Pipeline.pdf
https://on-demand.gputechconf.com/gtc/2014/presentations/S4379-opengl-44-scene-rendering-techniques.pdf
https://realtimecollisiondetection.net/blog/?p=91
https://i.stack.imgur.com/JgrSc.jpg
https://learnopengl.com/Advanced-Lighting/Deferred-Shading
https://learnopengl.com/PBR/IBL/Specular-IBL
quixels megascans (real scans)
*/

/*
 * ################   URGENT:
 * - Implement floating Origin
 * - recreate Meshes next to freshly created chunks
 * - Implement greedy Meshing
 * - Implement Block Types
 * - Implement text rendering
 * - Implement profiling
 * - implement multithreaded / deferred chunk building
 * 
 * - Maybe try deferred rendering
 */


DebugRenderer *DEBUG_RENRERER;

#include "Audio/AudioOutput.h"


int main() {
	// {
	// 	AudioOutput audioOutput;

	// 	audioOutput.setMasterVolume(.5);
	// 	audioOutput.start();

	// 	constexpr uint8_t CHANNELS = 2;
	// 	// constexpr uint32_t MAX_FRAMES_IN_DRIVER = 480;
	// 	// UINT32 numFramesInDriver = audioOutput.numFramesPadding();
	// 	// UINT32 numFramesToWrite = std::max<int32_t>(MAX_FRAMES_IN_DRIVER - numFramesInDriver, 0);
	// 	UINT32 numFramesToWrite = 48000;
	// 	float* outputBuffer = (float*)audioOutput.getBuffer(numFramesToWrite);

	// 	for(uint64_t frame = 0; frame < numFramesToWrite; frame++) {
	// 		outputBuffer[frame * CHANNELS + 0] = sin(frame * 440.f / 48000) * .2;
	// 		outputBuffer[frame * CHANNELS + 1] = sin(frame * 440.f / 48000) * .2;
	// 	}

	// 	audioOutput.releaseBuffer(numFramesToWrite);

	// 	Sleep(1000);

	// 	audioOutput.stop();

	// 	exit(1);
	// }



	OpenGLWindow win(800, 800);
	win.win.setCaptureMouse(true);


	World world; // Za Warudo!


	Player player(.5, 64., .5, 90.);
	// Player player(1000*1000, 64., .5, 90.);


	ShaderProgram shadowMapShaderProgram("../res/shaders/ShadowMap.vert.glsl", "../res/shaders/ShadowMap.frag.glsl");

	ShaderProgram shaderProgram("../res/shaders/main.vert.glsl", "../res/shaders/main.frag.glsl"); // Light + materials
	// ShaderProgram shaderProgram("../res/shaders/Mirror.vert.glsl", "../res/shaders/Mirror.frag.glsl"); // Mirror / Glass

	ShaderProgram debugShaderProgram("../res/shaders/Debug.vert.glsl", "../res/shaders/Debug.frag.glsl");

	// ShaderProgram lightsourceShaderProgram("../res/shaders/lightsource.vert.glsl", "../res/shaders/lightsource.frag.glsl");
	ShaderProgram skyboxShaderProgram("../res/shaders/SkyBox.vert.glsl", "../res/shaders/SkyBox.frag.glsl");


	ShaderProgram bloomCutoffShaderProgram("../res/shaders/BloomCutoff.compute.glsl");
	ShaderProgram bloomBlurShaderProgram("../res/shaders/BloomBlur.compute.glsl");
	ShaderProgram bloomAddShaderProgram("../res/shaders/BloomAdd.compute.glsl");

	ShaderProgram framebufferShaderProgram("../res/shaders/FrameBuffer.vert.glsl", "../res/shaders/FrameBuffer.frag.glsl");




	int work_grp_cnt[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
	printf("max global (total) work group counts x:%i y:%i z:%i\n", work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

	int work_grp_size[3];
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n", work_grp_size[0], work_grp_size[1], work_grp_size[2]);

	int work_grp_inv;
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("max local work group invocations %i\n", work_grp_inv);


	// Texture diffuse("../res/dirt.jpg");
	// Texture diffuse("../res/tex1.png");
	// Texture diffuse("../res/box_diffuse.png");

	// Texture normal("../res/brick_normal.png");
	// Texture normal("../res/pillow_normal.png");
	// Texture normal("../res/bumpy_normal.jpg");
	// Texture normal("../res/face_normal.jpg");

	Texture diffuse("../res/dirt_1024.png");
	Texture normal("../res/dirt_1024_normal.png");
	// Texture diffuse("../res/quartz_1024.png");
	// Texture normal("../res/quartz_1024_normal.png");
	// Texture diffuse("../res/melon_1024.png");
	// Texture normal("../res/melon_1024_normal.png");

	// Texture specular("../res/box_specular.png");

	Cubemap skyTexture("../res/cubemap----"); // files hard coded


	// StaticMesh bunny = StaticMesh::loadSTL("../res/Bunny.stl");
	StaticMesh bunny = StaticMesh::cube();
	// StaticMesh lightsource = StaticMesh::cube();
	StaticMesh viewportRect = StaticMesh::viewportRect();
	StaticMesh skyBox = StaticMesh::cube(glm::vec3(2.f));

	DebugRenderer debugRenderer;

	DEBUG_RENRERER = &debugRenderer;

	// glEnable(GL_MULTISAMPLE);

	glEnable(GL_CULL_FACE); // Backface Culling
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// glDepthMask(GL_FALSE); // Disable writing to depth Buffer
	// glDepthFunc(GL_ALWAYS); // Depth comparison Function

	// glEnable(GL_BLEND);
	// glBlendFunc(GL_ONE, GL_ZERO);

	const auto createTexture = [](GLuint &tex, const GLsizei width, const GLsizei height, const GLenum format) {
		glCreateTextures(GL_TEXTURE_2D, 1, &tex);
		glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// glTextureStorage2D(tex, 1, GL_RGB8, width, height); // color clamped between [0.0, 1.0], 8 bit per channel
		// glTextureStorage2D(tex, 1, GL_RGBA16F, width, height); // floating point color, not clamped, 16 bit per channel (64 bit total)
		// glTextureStorage2D(tex, 1, GL_R11F_G11F_B10F, width, height); // floating point color, not clamped, less bits per channel (32 bit total)
		glTextureStorage2D(tex, 1, format, width, height);
	};


	// <Main Framebuffer>
	GLuint FBO;
	glCreateFramebuffers(1, &FBO);

	GLuint framebufferTex;
	createTexture(framebufferTex, win.win.width, win.win.height, GL_R11F_G11F_B10F);
	glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, framebufferTex, 0);

	GLuint RenderBuffer;
	glCreateRenderbuffers(1, &RenderBuffer);
	glNamedRenderbufferStorage(RenderBuffer, GL_DEPTH_COMPONENT24, win.win.width, win.win.height);
	glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderBuffer); //Attach depth buffer to FBO

	GLenum fboStatus = glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer error: " << fboStatus << "\n";
		exit(1);
	}

	uint32_t fbWidth = win.win.width, fbHeight = win.win.height;
	// </Main Framebuffer>


	// <Shadowmap Framebuffer>
	// const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
	// const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
	// const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	const unsigned int SHADOW_WIDTH = 512, SHADOW_HEIGHT = 512;
	GLuint shadowFBO;
	glCreateFramebuffers(1, &shadowFBO);

	GLuint shadowTexture;
	glCreateTextures(GL_TEXTURE_2D, 1, &shadowTexture);

	glTextureParameteri(shadowTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(shadowTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureParameteri(shadowTexture, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE); // activate camparison mode for shadow map
	glTextureParameteri(shadowTexture, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL); // camparison mode for shadow map

	glTextureParameteri(shadowTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(shadowTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTextureStorage2D(shadowTexture, 1, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT);

	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTextureParameterfv(shadowTexture, GL_TEXTURE_BORDER_COLOR, borderColor);

	glNamedFramebufferTexture(shadowFBO, GL_DEPTH_ATTACHMENT, shadowTexture, 0);
	glNamedFramebufferDrawBuffer(shadowFBO, GL_NONE); // Otherwise Framebuffers without Color attachments are incomplete
	glNamedFramebufferReadBuffer(shadowFBO, GL_NONE); // Otherwise Framebuffers without Color attachments are incomplete

	{
		GLenum fboStatus = glCheckNamedFramebufferStatus(shadowFBO, GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "Framebuffer error: " << fboStatus << "\n";
			exit(1);
		}
	}
	// </Shadowmap Framebuffer>


	GLuint bloomTex1;
	glCreateTextures(GL_TEXTURE_2D, 1, &bloomTex1);
	glTextureParameteri(bloomTex1, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(bloomTex1, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(bloomTex1, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(bloomTex1, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTextureStorage2D(bloomTex1, 1, GL_R11F_G11F_B10F, win.win.width, win.win.height); // floating point color, not clamped, less bits per channel (32 bit total)
	GLuint bloomTex2;
	glCreateTextures(GL_TEXTURE_2D, 1, &bloomTex2);
	glTextureParameteri(bloomTex2, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(bloomTex2, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(bloomTex2, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTextureParameteri(bloomTex2, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTextureStorage2D(bloomTex2, 1, GL_R11F_G11F_B10F, win.win.width, win.win.height); // floating point color, not clamped, less bits per channel (32 bit total)


	// global UniformBufferObject (for proj + view Matrix):
	// <UBO>
	unsigned int UBO; 
	glCreateBuffers(1, &UBO);
	glNamedBufferData(UBO, 2 * (4 * 4) * sizeof(float), nullptr, GL_STATIC_DRAW); // allocate 152 bytes of memory

	// unsigned int ubIndex = glGetUniformBlockIndex(shaderProgram, "Lights");
	// glUniformBlockBinding(shaderProgram, ubIndex, 2);

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO); // target, index, buffer // option 1
	// glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboExampleBlock, 0, 152); // target, index, buffer, offset, size // option 2
	// </UBO>


	// Performance Counter Frequency:
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER lastTime;
	for(;;) {
		win.pollMsg();
		if(win.shouldClose())
			break;

		debugRenderer.clear();

		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);
		uint64_t timeSinceLastFrame = currentTime.QuadPart - lastTime.QuadPart;
		lastTime = currentTime;

		static int frame = 0; frame++;
		if(frame%60 == 0) {
			const int FPS = frequency.QuadPart / timeSinceLastFrame;
			const float frameTime = timeSinceLastFrame / (frequency.QuadPart / 1000 / 100) / 100.f; // first calculate 1/10th millisecond steps as integer, then shift comma left by one
			std::string num_text = std::to_string(frameTime);
			SetWindowTextA(win.win.wnd, ("FPS: " + std::to_string(FPS) + "  Frametime: " + num_text.substr(0, num_text.find(".")+2+1) + "ms").c_str());
		}

		const float dt = std::min(timeSinceLastFrame * 1. / frequency.QuadPart, .1); // frametime in seconds


		player.handleMouseInput(dt, win.win.mouseX, win.win.mouseY);

		player.update(dt, world);

		world.loadChunksAround({
				(int64_t)std::floor(player.getFootPos().x / 16),
				(int64_t)std::floor(player.getFootPos().y / 16),
				(int64_t)std::floor(player.getFootPos().z / 16),
			});

		bool sizeChanged = fbWidth != win.win.width || fbHeight != win.win.height;
		if(sizeChanged) {
			fbWidth = win.win.width;
			fbHeight = win.win.height;
		}

		// Framebuffer Resizing:
		if(sizeChanged) {
			glDeleteRenderbuffers(1, &RenderBuffer);
				glDeleteTextures(1, &framebufferTex);

				// createFrameBuffer(FBO, framebufferTex, RenderBuffer, win.win.width, win.win.height);

				createTexture(framebufferTex, win.win.width, win.win.height, GL_R11F_G11F_B10F);
				glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, framebufferTex, 0);

			glCreateRenderbuffers(1, &RenderBuffer);
			glNamedRenderbufferStorage(RenderBuffer, GL_DEPTH_COMPONENT24, win.win.width, win.win.height);
			glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderBuffer); //Attach depth buffer to FBO

			GLenum fboStatus = glCheckNamedFramebufferStatus(FBO, GL_FRAMEBUFFER);
			if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
				std::cout << "Framebuffer recreation error: " << fboStatus << "\n";
				exit(1);
			}

			// glCreateTextures(GL_TEXTURE_2D, 1, &framebufferTex);
			// glTextureParameteri(framebufferTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			// glTextureParameteri(framebufferTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			// glTextureParameteri(framebufferTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			// glTextureParameteri(framebufferTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// // glTextureStorage2D(framebufferTex, 1, GL_RGB8, win.win.width, win.win.height);
			// glTextureStorage2D(framebufferTex, 1, GL_R11F_G11F_B10F, win.win.width, win.win.height);
			// glNamedFramebufferTexture(FBO, GL_COLOR_ATTACHMENT0, framebufferTex, 0);

			// glCreateRenderbuffers(1, &RenderBuffer);
			// glNamedRenderbufferStorage(RenderBuffer, GL_DEPTH_COMPONENT24, win.win.width, win.win.height);
			// glNamedFramebufferRenderbuffer(FBO, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RenderBuffer); //Attach depth buffer to FBO
		}

		// Resize Bloom Texture:
		if(sizeChanged) {
			glDeleteTextures(1, &bloomTex1);
			glCreateTextures(GL_TEXTURE_2D, 1, &bloomTex1);
			glTextureParameteri(bloomTex1, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(bloomTex1, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(bloomTex1, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(bloomTex1, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTextureStorage2D(bloomTex1, 1, GL_R11F_G11F_B10F, win.win.width, win.win.height);
			glDeleteTextures(1, &bloomTex2);
			glCreateTextures(GL_TEXTURE_2D, 1, &bloomTex2);
			glTextureParameteri(bloomTex2, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(bloomTex2, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(bloomTex2, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(bloomTex2, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTextureStorage2D(bloomTex2, 1, GL_R11F_G11F_B10F, win.win.width, win.win.height);
		}


		const glm::mat4 projection = glm::perspective(glm::radians(player.getFOV()), win.win.width * 1.f / win.win.height, 0.1f, 1000.f);
		const glm::mat4 view = player.getViewMatrix();


		// Global Projection + View UBO:
		glNamedBufferSubData(UBO, 0, (4 * 4) * sizeof(float), &projection[0][0]);
		glNamedBufferSubData(UBO, (4 * 4) * sizeof(float), (4 * 4) * sizeof(float), &view[0][0]);

		// Main model:
		{
			const glm::vec3 viewPos = player.getViewPos();
			glProgramUniform3fv(shaderProgram, shaderProgram.getUniformLocation("viewPos"), 1, &viewPos[0]);

			// bunny.model = glm::rotate(bunny.model, glm::radians(30.f) * dt, glm::vec3(0.f, 1.f, 0.f));
			glProgramUniformMatrix4fv(shaderProgram, shaderProgram.getUniformLocation("model"), 1, GL_FALSE, &bunny.model[0][0]);
		}

		const glm::vec3 sunDir(.5f, -1.f, .2f);

		{

			// const glm::vec3 lightPos(1.3f, .4f, 1.5f);
			// const glm::vec3 lightPos(1.3f, 180.4f, 1.5f);
			const glm::vec3 lightPos = player.getRightHandPos();
			// Pointlight:
			// glm::vec3 lightPos = player.getRightHandPos();
			glProgramUniform3fv(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].position"), 1, &lightPos[0]);

			glProgramUniform1f(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].constant"), 1.f);
			glProgramUniform1f(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].linear"), .09f);
			glProgramUniform1f(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].quadratic"), .032f);

			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].ambient"), .2f, .2f, .2f);
			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].diffuse"), .5f, .5f, .5f);
			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].specular"), .3f, .2f, .1f);
			glm::vec3 pointCol = glm::vec3{ 0.98f, 0.65f, 0.03f } * .2f;
			glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].ambient"), .0f, .0f, .0f);
			glProgramUniform3fv(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].diffuse"), 1, &pointCol[0]);
			glProgramUniform3fv(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].specular"), 1, &pointCol[0]);
			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].diffuse"), 5.f, 5.f, 5.f); // extremely bright
			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("pointLights[0].specular"), 12.f, 15.f, 13.f); // extremely bright

			// Dirlight:
			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("dirLight.direction"), 1.f, -1.f, 1.f);
			glProgramUniform3fv(shaderProgram, shaderProgram.getUniformLocation("dirLight.direction"), 1, &sunDir[0]);
			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("dirLight.direction"), 0.f, -1.f, 0.f);

			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("dirLight.ambient"), .2f, .2f, .2f);
			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("dirLight.diffuse"), .2f, .2f, .2f);
			// glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("dirLight.specular"), .3f, .2f, .1f);
			glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("dirLight.ambient"), .1f, .1f, .1f);
			glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("dirLight.diffuse"), .4f, .4f, .4f);
			glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("dirLight.specular"), .4f, .4f, .4f);

			// Material:
			glProgramUniform1f(shaderProgram, shaderProgram.getUniformLocation("material.shininess"), 128.f);
			glProgramUniform3f(shaderProgram, shaderProgram.getUniformLocation("material.specularColor"), 1.f, 1.f, 1.f);
		}


		// --------------------------   Render Shadow map:
		glm::mat4 lightSpaceMatrix;
		if(false)
		{
			const float near_plane = 1.0f, far_plane = 100.5f;
			glm::vec3 sunPos = player.getViewPos() - sunDir * 40.f;

			// const float pixelSize = 80.f / SHADOW_WIDTH;
			// sunPos.x = pixelSize * (int)(sunPos.x / pixelSize);
			// sunPos.z = pixelSize * (int)(sunPos.z / pixelSize);

			glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane); // left, right, bottom, top, near, far

			glm::mat4 lightView = glm::lookAt(  sunPos, // eye
												sunPos + sunDir, // center
												glm::vec3( 0.0f, 1.0f,  0.0f)); // up

			lightSpaceMatrix = lightProjection * lightView;


			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
				glClear(GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				// glCullFace(GL_FRONT);
				glProgramUniformMatrix4fv(shadowMapShaderProgram, glGetUniformLocation(shadowMapShaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);
				shadowMapShaderProgram.bind();
					world.draw(shadowMapShaderProgram);

					glm::mat4 playerModel(1.f);
					playerModel = glm::translate(playerModel, player.getFootPos() + glm::vec3(0.f, 1.65f / 2 + 0.01f, 0.f));
					playerModel = glm::rotate(playerModel, glm::radians(-player.getYaw()), glm::vec3(0.f, 1.f, 0.f));
					glProgramUniformMatrix4fv(shadowMapShaderProgram, shadowMapShaderProgram.getUniformLocation("model"), 1, GL_FALSE, &playerModel[0][0]);
					player.draw();
				// glCullFace(GL_BACK);
				glDisable(GL_DEPTH_TEST);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, win.win.width, win.win.height);
		}


		// set lightSpaceMatrix of main shader Program for reverse transformation
		glProgramUniformMatrix4fv(shaderProgram, glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

		// forward TextureMap to Main ShaderProgram
		glBindTextureUnit(5, shadowTexture);
		// glBindTextureUnit(shadowTexture, 5);
		glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "shadowMap"), 5);




		// Render Scene:
		glEnable(GL_DEPTH_TEST); // Depth test main render
		glBindFramebuffer(GL_FRAMEBUFFER, FBO); // Render to FrameBuffer

		// Clear Screen:
			glClearColor(19.0f / 255.0f, 34.0f / 255.0f, 44.0f / 255.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Objects:
			// skyTexture.bind(0);
			// glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "skybox"), 0); // Reflective / Transmissive Shader

			shaderProgram.bind();
				diffuse.bind(1);
				glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "material.diffuseMap"), 1); // Solid Shader
				// specular.bind(2);
				glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "material.specularMap"), 0); // Solid Shader
				normal.bind(3);
				glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "normalMap"), 3); // Solid Shader
				// bunny.bind();
				// bunny.draw();
				// glm::mat4 worldModel(1.f);
				// glProgramUniformMatrix4fv(shaderProgram, shaderProgram.getUniformLocation("model"), 1, GL_FALSE, &worldModel[0][0]);
				world.draw(shaderProgram);

				if(player.bodyVisible()) {
					glm::mat4 playerModel(1.f);
					playerModel = glm::translate(playerModel, player.getFootPos() + glm::vec3(0.f, 1.65f / 2, 0.f));
					playerModel = glm::rotate(playerModel, glm::radians(-player.getYaw()), glm::vec3(0.f, 1.f, 0.f));
					glProgramUniformMatrix4fv(shaderProgram, shaderProgram.getUniformLocation("model"), 1, GL_FALSE, &playerModel[0][0]);
					player.draw();

					// DEBUG_RENRERER->box(glm::vec3(pos.x-.3f, pos.y, pos.z-.3f), glm::vec3(pos.x+.3f, pos.y+1.8f, pos.z+.3f)); // Hitbox
					DEBUG_RENRERER->box(player.getAABB().min, player.getAABB().max); // Hitbox
				}

			// Crosshair:
			DEBUG_RENRERER->box(player.getViewPos() + player.getViewDir() * .2f - glm::vec3(.0001f, .0001f, .0001f),
								player.getViewPos() + player.getViewDir() * .2f + glm::vec3(.0001f, .0001f, .0001f));

			// lightsourceShaderProgram.bind();
			// 	lightsource.bind();
			// 	lightsource.draw();

		// Debug:
			glDepthFunc(GL_LEQUAL);// change depth function so depth test passes when values are equal to depth buffer's content
			debugShaderProgram.bind();
				debugRenderer.bind();
				debugRenderer.draw();
			glDepthFunc(GL_LESS); // set depth function back to default


		// Skybox:
			glDepthFunc(GL_LEQUAL);// change depth function so depth test passes when values are equal to depth buffer's content
			glCullFace(GL_FRONT);
			skyTexture.bind(1);
			glProgramUniform1i(skyboxShaderProgram, glGetUniformLocation(skyboxShaderProgram, "skybox"), 1);
			skyboxShaderProgram.bind();
				skyBox.bind();
				skyBox.draw();
			glCullFace(GL_BACK);
			glDepthFunc(GL_LESS); // set depth function back to default

		glBindFramebuffer(GL_FRAMEBUFFER, 0); // Stop rendering to FrameBuffer
		glDisable(GL_DEPTH_TEST); // Don't depth test after main render



		// Bloom:
		// if(GetAsyncKeyState('B') & 0x8000) {
		LARGE_INTEGER beforeBloom;
		QueryPerformanceCounter(&beforeBloom);
			// glBindTextureUnit(0, framebufferTex);
			constexpr int fac = 64;
			const GLuint numDispatchesX = win.win.width * win.win.height / fac;// + 1;
			const GLuint numDispatchesY = 1;
			bloomCutoffShaderProgram.bind();
				glProgramUniform2i(bloomCutoffShaderProgram, glGetUniformLocation(bloomCutoffShaderProgram, "dimensions"), win.win.width, win.win.height); // horizontal
				glProgramUniform1f(bloomCutoffShaderProgram, glGetUniformLocation(bloomCutoffShaderProgram, "cutoff"), .5);
				glProgramUniform1f(bloomCutoffShaderProgram, glGetUniformLocation(bloomCutoffShaderProgram, "cutoffWidth"), .05);
				glBindImageTexture(0, framebufferTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R11F_G11F_B10F);
				glBindImageTexture(1, bloomTex1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
				glDispatchCompute(numDispatchesX, numDispatchesY, 1);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			bloomBlurShaderProgram.bind();
				for(int i = 0; i < 4; i++) {
					glProgramUniform2i(bloomBlurShaderProgram, glGetUniformLocation(bloomBlurShaderProgram, "dimensions"), win.win.width, win.win.height); // horizontal
					glProgramUniform1i(bloomBlurShaderProgram, glGetUniformLocation(bloomBlurShaderProgram, "horizontal"), 1); // horizontal
					glBindImageTexture(0, bloomTex1, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R11F_G11F_B10F);
					glBindImageTexture(1, bloomTex2, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
					glDispatchCompute(numDispatchesX, numDispatchesY, 1);
					glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

					glProgramUniform1i(bloomBlurShaderProgram, glGetUniformLocation(bloomBlurShaderProgram, "horizontal"), 0); // vertical
					glBindImageTexture(0, bloomTex2, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R11F_G11F_B10F);
					glBindImageTexture(1, bloomTex1, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R11F_G11F_B10F);
					glDispatchCompute(numDispatchesX, numDispatchesY, 1);
					glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
				}

			bloomAddShaderProgram.bind();
				glProgramUniform2i(bloomAddShaderProgram, glGetUniformLocation(bloomAddShaderProgram, "dimensions"), win.win.width, win.win.height); // horizontal
				glBindImageTexture(0, framebufferTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R11F_G11F_B10F);
				glBindImageTexture(1, bloomTex2, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R11F_G11F_B10F);
				glDispatchCompute(numDispatchesX, numDispatchesY, 1);
				glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		LARGE_INTEGER afterBloom;
		QueryPerformanceCounter(&afterBloom);
		if(frame % 2000 == 0)
			printf("Bloom time: %.4f ms\n", (afterBloom.QuadPart - beforeBloom.QuadPart) * 1. / (frequency.QuadPart / 1000));
		// }


		// Tonemapping + Gamma correction:
			// framebuffer texture
			glBindTextureUnit(1, framebufferTex);
			glProgramUniform1i(framebufferShaderProgram, glGetUniformLocation(framebufferShaderProgram, "screen"), 1);

			// Framebuffer post processing:
			framebufferShaderProgram.bind();
				viewportRect.bind();
				viewportRect.draw();

		win.swapBuffers();
	}

	return 0;
}