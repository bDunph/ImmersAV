#ifndef STUDIO_HPP
#define STUDIO_HPP

#define NUM_HRTF_VALS 3
#define NUM_SOUND_SOURCES 1

#include <string>
#include <vector>

#include "RapidLib/regression.h"
#include "StudioTools.hpp"

class Studio {

public:
	
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
		bool bHaltModel;
		bool bSaveModel;
		bool bLoadModel;
		bool bDevMode;
	};

	struct MLAudioParameter 
	{
		float distributionLow;
		float distributionHigh;
		int sendVecPosition;
	};

	bool Setup(std::string csd, GLuint shaderProg);
	void Update(glm::mat4 viewMat, MachineLearning& machineLearning, glm::vec3 controllerWorldPos_0, glm::vec3 controllerWorldPos_1, glm::quat controllerQuat_0, glm::quat controllerQuat_1, PBOInfo& pboInfo);
	void Draw(glm::mat4 projMat, glm::mat4 viewMat, glm::mat4 eyeMat, GLuint mengerProg, glm::vec3 translateVec);
	bool BCsoundReturn(CsoundSession* _session, std::vector<const char*>& returnName, std::vector<MYFLT*>& returnVal);
	void MLRegressionSetup();
	void MLRegressionUpdate(MachineLearning& machineLearning, PBOInfo& pboInfo, std::vector<MLAudioParameter>& params);
	void Exit();

private:

	StudioTools* m_pStTools;

	glm::vec4 cameraPos;
	
	MYFLT* m_pPitchOut;
	MYFLT* m_pFreqOut;
	MYFLT* m_cspSineControlVal;
	MYFLT* m_cspRandVal;

	GLint m_gliSineControlValLoc;
	GLint m_gliPitchOutLoc;
	GLint m_gliFreqOutLoc;
	
	//control variables
	bool m_bFirstLoop; 
	float m_fSineControlVal;
	float m_fPitch;
	float m_fDeltaTime;
	float m_fLastFrame;
	float m_fCurrentFrame;
	float m_fTargetVal;
	float m_fCurrentVal;

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
	regression staticRegression;
	trainingExample trainingData;
	std::vector<trainingExample> trainingSet;
	std::vector<double> inputData;
	std::vector<double> outputData;	
};
#endif
