#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include <memory>
#include <string>
#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>
//#include <ctime>

#include "Studio.hpp"
#include "VR_Manager.hpp"

#ifdef __APPLE__ 
#include "GLFW/glfw3.h"
#elif _WIN32 
#include "glfw3.h"
#endif


class Graphics{

public:

	Graphics(std::unique_ptr<ExecutionFlags>& flagPtr);
	bool BInitGL(std::string avFileName, bool fullscreen = false);
	bool BCreateDefaultShaders();
	GLuint BCreateSceneShaders(std::string shaderName);
	GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader );
	void CreatePBO();
	bool BSetupStereoRenderTargets(std::unique_ptr<VR_Manager>& vrm);
	void CleanUpGL(std::unique_ptr<VR_Manager>& vrm);
	bool BSetupCompanionWindow();
	//void DevMouseCallback(GLFWwindow* window, double xpos, double ypos);
	void GetControllerEvents(std::unique_ptr<VR_Manager>& vrm);
	void DevProcessInput(GLFWwindow *window);
	void UpdateSceneData(std::unique_ptr<VR_Manager>& vrm);
	bool BRenderFrame(std::unique_ptr<VR_Manager>& vrm);
	void RenderControllerAxes(std::unique_ptr<VR_Manager>& vrm);
	void RenderStereoTargets(std::unique_ptr<VR_Manager>& vrm);
	void RenderScene(vr::Hmd_Eye nEye, std::unique_ptr<VR_Manager>& vrm);
	void RenderCompanionWindow();
	void WriteToPNG(GLubyte* &data);
	bool TempEsc();
	void IncreaseRotationValue(std::unique_ptr<int>& pVal);

private:

	void WriteDataToPBO();
	void TransferDataToCPU();
	bool CheckGLError(std::string location);

	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nRenderTextureId;
		GLuint m_nRenderFramebufferId;
		GLuint m_nResolveTextureId;
		GLuint m_nResolveFramebufferId;
		GLuint m_gluiDataTexture2DMulti;
	};
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	GLuint m_gluiStorageFBO;
	GLuint m_gluiDataTexture2D;

	bool BCreateStorageFBO();
	bool BCreateFrameBuffer(FramebufferDesc& framebufferDesc, bool leftEye);
	void BlitDataTexture();

	glm::mat4 m_mat4CurrentViewMatrix;

	glm::vec4 m_vFarPlaneDimensions;

	GLFWwindow* m_pGLContext; // TODO: convert this to unique_ptr<>	

	uint32_t m_nCompanionWindowWidth;
	int32_t m_nCompanionWindowHeight;
	uint32_t windowWidth;
	uint32_t windowHeight;
	GLuint m_glMainShaderProgramID;
	GLuint m_gluiTetraShaderProgramID;
	
	struct VertexDataWindow
	{
		glm::vec2 position;
		glm::vec2 texCoord;

		VertexDataWindow( const glm::vec2& pos, const glm::vec2 tex ) :  position(pos), texCoord(tex) {	}
	};
	
	GLuint m_unCompanionWindowVAO;
	GLuint m_glCompanionWindowIDVertBuffer;
	GLuint m_glCompanionWindowIDIndexBuffer;
	unsigned int m_uiCompanionWindowIndexSize;

	unsigned int m_uiControllerVertCount;

	GLuint m_unControllerVAO;
	GLuint m_glControllerVertBuffer;
	//GLint m_nSceneMatrixLocation;
	glm::mat4 m_mat4HMDPose;
	GLuint m_unSceneVAO;
	GLuint m_glSceneVBO;
	GLuint m_gluiTetraVAO;
	GLuint m_gluiTetraVBO;
	GLuint m_gluiTetraIndexBuffer;
	unsigned int m_uiNumSceneVerts;
	unsigned int m_uiNumTetraVerts;
	unsigned int m_uiNumTetraIndices;
	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;
	
	GLuint m_unCompanionWindowProgramID;
	GLuint m_unControllerTransformProgramID;
	GLuint m_unRenderModelProgramID;

	GLint m_nControllerMatrixLocation;
	GLint m_nRenderModelMatrixLocation;

	bool m_bVblank;
	bool m_bGLFinishHack;

	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount_Last;
	std::string m_strPoseClasses;                            // what classes we saw poses for this frame

	bool m_bDebugOpenGL;
	bool m_bDebugPrintMessages;
	bool m_bDevMode;

	//GLint resolution; 
	GLint m_gliViewEyeProjLocation;
	GLint m_nViewMatrixLocation;
	GLint m_nViewEyeMatrixLocation;
	GLint m_nRenderWidthLocation;
	GLint m_nRenderHeightLocation;
	//GLint m_nZNearLocation;
	//GLint m_nZFarLocation;
	GLint m_nAspectLocation;
	GLint m_nTanFovLocation;
	GLint m_nProjectionMatrixLocation;
	GLint m_nEyeMatLocation;
	GLint m_nRotation3DLocation;
	GLint m_gliTimerLocation;

	unsigned int m_uiNumSceneIndices;
	GLuint m_glIndexBuffer;

	std::unique_ptr<int> m_pRotationVal;

	unsigned int m_unImageCount;	
	bool m_bRecordScreen;

	//time_t m_tStartTime;
	//unsigned int m_uiFrameNumber;

	//shader handles
	GLuint skyboxShaderProg;
	GLuint soundObjShaderProg;
	GLuint groundPlaneShaderProg;
	GLuint studioShaderProg;
	GLuint quadShaderProg;

	Studio studio;
	
	//quad
	//glm::mat4 quadModelMatrix;
	//GLuint quadVAO;
	//GLuint quadIndexBuffer;
	//unsigned int quadTexID;

	//GLint quad_projMatLoc;
	//GLint quad_viewMatLoc;
	//GLint quad_modelMatLoc;

	//dev mode
	glm::mat4 m_matDevProjMatrix;
	glm::mat4 m_matDevProjMatrix_InfiniteFarPlane;
	glm::mat4 m_matDevViewMatrix;
	glm::vec3 m_vec3DevCamPos;
	glm::vec3 m_vec3DevCamUp;

	float m_fDeltaTime;
	float m_fLastFrame;	

	//menger sponge
	GLuint mengerShaderProg;

	
	float m_fFov;
	
	//rapidmix
	Studio::MachineLearning machineLearning;

	//movement controls
	glm::vec2 m_vVRPos;

	//pointer to pointer of 2D dynamic array for storing mandel values from openGL dataTexture
	//typedef std::unique_ptr<float[]> m_fpDataArrayRow;
	//std::unique_ptr<m_fpDataArrayRow[]> m_fpDataArrayCol;

	// handle for PBO
	GLuint pbo[2];

	// vector to store floats from pbo
	std::vector<float> shaderData;

	//set draw buffer for storage fbo
	GLenum buf[1];
	GLenum buffers[2];
	GLenum rightBuffer[1];
	
	//boolean flag to indicate PBO has data
	bool m_bPBOFirstFrame;
	bool m_bWriteInProgress;
	
	GLsync sync;
	//unsigned char* m_pDataSize;
	bool m_bPboIndex;

	GLuint m_gluiDummyTexture;

	glm::vec3 m_vec3ControllerWorldPos[2];
	glm::quat m_quatController[2];

	Studio::PBOInfo m_structPboInfo;	

	glm::vec4 m_vec4TranslationVal;
	glm::vec3 m_vec3TranslationVal;
	float m_fMaxDist;
	glm::vec3 m_vec3InitCamPos;
};


#endif
