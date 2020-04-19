#ifndef STUDIO_HPP
#define STUDIO_HPP

#define NUM_HRTF_VALS 3
#define NUM_SOUND_SOURCES 1

#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/gtc/quaternion.hpp>

#ifdef __APPLE__
#include "rapidmix.h"
#elif _WIN32
#include "RapidLib/regression.h"
#endif

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

	struct SoundSourceData
	{
		glm::vec4 position;
		glm::vec4 posCamSpace;
		float distCamSpace;
		float azimuth;
		float elevation;
	};
	
	struct AudioParameter 
	{
		float distributionLow;
		float distributionHigh;
		const char* name;
		float value;
	};

	bool setup(std::string csd, GLuint shaderProg);
	void update(glm::mat4 viewMat, glm::vec3 camPos, MachineLearning& machineLearning, glm::vec3 controllerWorldPos_0, glm::vec3 controllerWorldPos_1, glm::quat controllerQuat_0, glm::quat controllerQuat_1, PBOInfo& pboInfo, glm::vec3 translateVec);
	void draw(glm::mat4 projMat, glm::mat4 viewMat, glm::mat4 eyeMat, RaymarchData& raymarchData, GLuint mengerProg);
	CsoundSession* PCsoundSetup(std::string _csdName);
	bool BSoundSourceSetup(CsoundSession* _session, int numSources);
	void SoundSourceUpdate(std::vector<SoundSourceData>& soundSources, glm::mat4 _viewMat);
	void RaymarchQuadSetup(GLuint _shaderProg);
	void DrawStart(glm::mat4 _projMat, glm::mat4 _eyeMat, glm::mat4 _viewMat, GLuint _shaderProg);
	void DrawEnd();
	bool BCsoundSend(CsoundSession* _session, std::vector<const char*>& sendName, std::vector<MYFLT*>& sendVal);
	bool BCsoundReturn(CsoundSession* _session, std::vector<const char*>& returnName, std::vector<MYFLT*>& returnVal);
	void MLRegressionSetup();
	void MLRegressionUpdate(MachineLearning& machineLearning, PBOInfo& pboInfo, std::vector<AudioParameter>& params);
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
	MYFLT* m_cspRandVal;

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
	GLint m_gliRmsOutLoc;
	glm::mat4 modelViewEyeProjectionMat;
	glm::mat4 inverseMVEPMat;
	
	//control variables
	float sineControlVal;

	bool m_bFirstLoop; 

	std::vector<SoundSourceData> m_vSoundSources;
	std::vector<const char*> m_vSendNames;
	std::vector<MYFLT*> m_vSendVals;
	std::vector<const char*> m_vReturnNames;
	std::vector<MYFLT*> m_vReturnVals;
	std::vector<const char*> m_vMLParamSendNames;
	std::vector<MYFLT*> m_vMLParamSendVals;

	//machine learning controls
	bool m_bPrevSaveState;
	bool m_bPrevRandomState;
	bool m_bPrevTrainState;
	bool m_bPrevHaltState;
	bool m_bPrevLoadState;
	bool m_bCurrentMsgState;
	bool m_bMsg;
	bool m_bRunMsg;
	bool m_bCurrentRunMsgState;
	float sizeVal;
	bool m_bModelTrained;

	//machine learning 
#ifdef __APPLE__
	rapidmix::staticRegression staticRegression;
	rapidmix::trainingData trainingData;
#elif _WIN32
	regression staticRegression;
	trainingExample trainingData;
	std::vector<trainingExample> trainingSet;
	bool m_bPrevRunHaltState;
#endif

	std::vector<double> inputData;
	std::vector<double> outputData;	


};
#endif
