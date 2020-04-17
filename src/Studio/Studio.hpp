#ifndef STUDIO_HPP
#define STUDIO_HPP

#define NUM_HRTF_VALS 3
#define NUM_SOUND_SOURCES 1

#include <string>
#include <vector>
#include <glm/gtc/quaternion.hpp>

#ifdef __APPLE__
#include "rapidmix.h"
#elif _WIN32
#include "RapidLib/regression.h"
#endif

#include "SoundObject.hpp"
#include "CsoundSession.hpp"

class Studio {

public:
	
	//struct to hold data for raymarch shader
	struct RaymarchData{
		float tanFovYOver2;
		float aspect;
	};

	// struct to hold PBO info
	struct PBOInfo
	{
		unsigned char* pboPtr;
		uint32_t pboSize;		
	};

	//bools to control machine learning
	struct MachineLearning{
		bool bRecord;
		bool bRandomParams;
		bool bTrainModel;
		bool bRunModel;
		bool bSaveTrainingData;
		bool bHaltModel;
		bool bLoadTrainingData;
		bool bSaveModel;
		bool bLoadModel;
	};

	bool setup(std::string csd, GLuint shaderProg);
	void update(glm::mat4 viewMat, glm::vec3 camPos, MachineLearning& machineLearning, glm::vec3 controllerWorldPos_0, glm::vec3 controllerWorldPos_1, glm::quat controllerQuat_0, glm::quat controllerQuat_1, PBOInfo& pboInfo, glm::vec3 translateVec);
	void draw(glm::mat4 projMat, glm::mat4 viewMat, glm::mat4 eyeMat, RaymarchData& raymarchData, GLuint mengerProg);
	void exit();

private:

	glm::vec4 cameraPos;
	
	//matrices 
	glm::mat4 modelMatrix;
	glm::vec3 m_vec3Translation;

	//Csound
	CsoundSession *session;
	MYFLT* azimuthVals[NUM_SOUND_SOURCES];
	MYFLT* elevationVals[NUM_SOUND_SOURCES];
	MYFLT* distanceVals[NUM_SOUND_SOURCES];
	MYFLT* m_pRmsOut;
	MYFLT* m_cspSineControlVal;

	//raymarching quad
	unsigned int m_uiNumSceneVerts;
	unsigned int m_uiNumSceneIndices;
	unsigned int m_uiNumSceneTexCoords;

	GLuint m_uiglSceneVAO;
	GLuint m_uiglSceneVBO;
	GLuint m_uiglIndexBuffer;
	
	GLint m_gliMVEPMatrixLocation;
	GLint m_gliInverseMVEPLocation;
	GLint m_gliSineControlValLoc;
	glm::mat4 modelViewEyeProjectionMat;
	glm::mat4 inverseMVEPMat;
	
	//control variables
	float sineControlVal;

	bool m_bFirstLoop; 

	struct SoundSourceData
	{
		glm::vec4 position;
		glm::vec4 posCamSpace;
		float distCamSpace;
		float azimuth;
		float elevation;
	};

	std::vector<SoundSourceData> m_vSoundSources;
};
#endif
