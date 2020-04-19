#include "Studio.hpp"

#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <assert.h>
#include <math.h>
#include <cmath>
#include <iostream>
#include <random>

#include "stb_image.h"

#ifdef __APPLE__ 
#include "GLFW/glfw3.h"
#define TRAINING_SET_SIZE trainingData.trainingSet.size()
#elif _WIN32 
#include "glfw3.h"
#define TRAINING_SET_SIZE trainingSet.size()
#endif

#include "SystemInfo.hpp"
#include "ShaderManager.hpp"

#define PI 3.14159265359

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

//*******************************************************************************************
// Setup 
//*******************************************************************************************
bool Studio::setup(std::string csd, GLuint shaderProg)
{
	// bool to indicate first loop through update and draw functions to 
	// set initial paramaters
	m_bFirstLoop = true;


//*****************************Audio Setup********************************************************

	CsoundSession* csSession = PCsoundSetup(csd);
	
	if(!BSoundSourceSetup(csSession, NUM_SOUND_SOURCES))
	{
		std::cout << "Studio::setup sound sources not set up" << std::endl;
		return false;
	}

//********* send values to csound *******************//

	m_vSendNames.push_back("sineControlVal");
	m_vSendVals.push_back(m_cspSineControlVal);	
	m_vSendNames.push_back("randomVal");
	m_vSendVals.push_back(m_cspRandVal);
	BCsoundSend(csSession, m_vSendNames, m_vSendVals);

//********* return values from csound *******************//

	// example return value - RMS
	m_vReturnNames.push_back("rmsOut");
	m_vReturnVals.push_back(m_pRmsOut);
	BCsoundReturn(csSession, m_vReturnNames, m_vReturnVals);	
	
//************************************************************************************************
//************************************************************************************************




//*************************************Visual Setup***********************************************

	// Set up quad to use for raymarching
	RaymarchQuadSetup(shaderProg);
	
	// shader uniforms
	m_gliSineControlValLoc = glGetUniformLocation(shaderProg, "sineControlVal");
	m_gliRmsOutLoc = glGetUniformLocation(shaderProg, "rmsOut");
	
	modelMatrix = glm::mat4(1.0f);

//************************************************************************************************
//************************************************************************************************


	MLRegressionSetup();

	return true;
}
//*******************************************************************************************


//*******************************************************************************************
// Update 
//*******************************************************************************************
void Studio::update(glm::mat4 viewMat, glm::vec3 camPos, MachineLearning& machineLearning, glm::vec3 controllerWorldPos_0, glm::vec3 controllerWorldPos_1, glm::quat controllerQuat_0, glm::quat controllerQuat_1, PBOInfo& pboInfo, glm::vec3 translateVec){

	// For return values from shader
	// vec4 for each fragment is returned in the order ABGR
	// you have to wait until the 2nd frame to read from the buffer 
	//if(!m_bFirstLoop)
	//std::cout << (double)pboInfo.pboPtr[0] << std::endl;

	m_vec3Translation = translateVec;
	
	glm::vec4 viewerPosCameraSpace = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	// example sound source at origin
	SoundSourceData soundSource1;
	soundSource1.position = viewerPosCameraSpace;
	m_vSoundSources.push_back(soundSource1);

	SoundSourceUpdate(m_vSoundSources, viewMat);

	//example control signal - sine function
	//sent to shader and csound
	sineControlVal = sin(glfwGetTime() * 0.33f);
	*m_vSendVals[0] = (MYFLT)sineControlVal;

	//run machine learning
	MLRegressionUpdate(machineLearning, pboInfo);	
}
//*********************************************************************************************


//*********************************************************************************************
// Draw 
//*********************************************************************************************
void Studio::draw(glm::mat4 projMat, glm::mat4 viewMat, glm::mat4 eyeMat, RaymarchData& raymarchData, GLuint mengerProg)
{
	DrawStart(projMat, eyeMat, viewMat, mengerProg);
	
	glUniform1f(m_gliSineControlValLoc, sineControlVal);
	glUniform1f(m_gliRmsOutLoc, *m_vReturnVals[0]);

	DrawEnd();

	// update first loop switch
	m_bFirstLoop = false;
}
//*********************************************************************************************

















//*********************************************************************************************
//*********************************************************************************************
//*********************************************************************************************
CsoundSession* Studio::PCsoundSetup(std::string _csdName)
{
	std::string csdName = "";
	if(!_csdName.empty()) csdName = _csdName;
	session = new CsoundSession(csdName);

#ifdef _WIN32
	session->SetOption("-b -32"); 
	session->SetOption("-B 2048");
#endif
	session->StartThread();
	session->PlayScore();

	return session;
}

bool Studio::BSoundSourceSetup(CsoundSession* _session, int numSources)
{

	for(int i = 0; i < NUM_SOUND_SOURCES; i++)
	{
		std::string val1 = "azimuth" + std::to_string(i);
		const char* azimuth = val1.c_str();	
		if(session->GetChannelPtr(azimuthVals[i], azimuth, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
		{
			std::cout << "GetChannelPtr could not get the azimuth" << i << " input" << std::endl;
			return false;
		}

		std::string val2 = "elevation" + std::to_string(i);
		const char* elevation = val2.c_str();
		if(session->GetChannelPtr(elevationVals[i], elevation, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
		{
			std::cout << "GetChannelPtr could not get the elevation" << i << " input" << std::endl;
			return false;
		}	

		std::string val3 = "distance" + std::to_string(i);
		const char* distance = val3.c_str();
		if(session->GetChannelPtr(distanceVals[i], distance, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
		{
			std::cout << "GetChannelPtr could not get the distance" << i << " input" << std::endl;
			return false;
		}
	}

	return true;
}

void Studio::SoundSourceUpdate(std::vector<SoundSourceData>& soundSources, glm::mat4 _viewMat)
{
	for(int i = 0; i < soundSources.size(); i++)
	{
		// camera space positions
		soundSources[i].posCamSpace = _viewMat * modelMatrix * soundSources[i].position;

		// distance value
		soundSources[i].distCamSpace = sqrt(pow(soundSources[i].posCamSpace.x, 2) + pow(soundSources[i].posCamSpace.y, 2) + pow(soundSources[i].posCamSpace.z, 2));

		//azimuth in camera space
		float valX = soundSources[i].posCamSpace.x - soundSources[i].position.x;
		float valZ = soundSources[i].posCamSpace.z - soundSources[i].position.z;

		soundSources[i].azimuth = atan2(valX, valZ);
		soundSources[i].azimuth *= (180.0f/PI); 	

		//elevation in camera space
		float oppSide = soundSources[i].posCamSpace.y - soundSources[i].position.y;
		float sinVal = oppSide / soundSources[i].distCamSpace;
		soundSources[i].elevation = asin(sinVal);
		soundSources[i].elevation *= (180.0f/PI);		

		//send values to Csound pointers
		*azimuthVals[i] = (MYFLT)soundSources[i].azimuth;
		*elevationVals[i] = (MYFLT)soundSources[i].elevation;
		*distanceVals[i] = (MYFLT)soundSources[i].distCamSpace;
	}
	
	soundSources.clear();

}

void Studio::RaymarchQuadSetup(GLuint _shaderProg)
{
	float sceneVerts[] = {
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f
	};
	m_uiNumSceneVerts = _countof(sceneVerts);

	unsigned int sceneIndices[] = {
		0, 1, 2,
		2, 3, 0
	};
	m_uiNumSceneIndices = _countof(sceneIndices);

	float groundRayTexCoords [] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
	};
	m_uiNumSceneTexCoords = _countof(groundRayTexCoords);	

	glGenVertexArrays(1, &m_uiglSceneVAO);

	glBindVertexArray(m_uiglSceneVAO);

	GLuint m_uiglSceneVBO;
	glGenBuffers(1, &m_uiglSceneVBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiglSceneVBO);
	glBufferData(GL_ARRAY_BUFFER, m_uiNumSceneVerts * sizeof(float), sceneVerts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	GLuint m_uiglGroundTexCoords;
	glGenBuffers(1, &m_uiglGroundTexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, m_uiglGroundTexCoords);
	glBufferData(GL_ARRAY_BUFFER, m_uiNumSceneTexCoords * sizeof(float), groundRayTexCoords, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL); 

	glGenBuffers(1, &m_uiglIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiglIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uiNumSceneIndices * sizeof(unsigned int), sceneIndices, GL_STATIC_DRAW);
	
	
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);

	m_gliMVEPMatrixLocation = glGetUniformLocation(_shaderProg, "MVEPMat");
	m_gliInverseMVEPLocation = glGetUniformLocation(_shaderProg, "InvMVEP");
}

void Studio::DrawStart(glm::mat4 _projMat, glm::mat4 _eyeMat, glm::mat4 _viewMat, GLuint _shaderProg)
{
	modelMatrix = glm::translate(modelMatrix, m_vec3Translation);

	//matrices for raymarch shaders
	modelViewEyeProjectionMat = _projMat * _eyeMat * _viewMat * modelMatrix;
	inverseMVEPMat = glm::inverse(modelViewEyeProjectionMat);

	glBindVertexArray(m_uiglSceneVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiglIndexBuffer);
	glUseProgram(_shaderProg);

	glUniformMatrix4fv(m_gliMVEPMatrixLocation, 1, GL_FALSE, &modelViewEyeProjectionMat[0][0]);
	glUniformMatrix4fv(m_gliInverseMVEPLocation, 1, GL_FALSE, &inverseMVEPMat[0][0]);
}

void Studio::DrawEnd()
{
	glDrawElements(GL_TRIANGLES, m_uiNumSceneIndices * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

bool Studio::BCsoundSend(CsoundSession* _session, std::vector<const char*>& sendName, std::vector<MYFLT*>& sendVal)
{
	for(int i = 0; i < sendName.size(); i++)
	{
		const char* chName = sendName[i];
		std::cout << chName << std::endl;
		if(_session->GetChannelPtr(sendVal[i], chName, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
		{
			std::cout << "GetChannelPtr could not get the send value at position " << sendName[i] << std::endl;
			return false;
		}
	}
	
	return true;
}

bool Studio::BCsoundReturn(CsoundSession* _session, std::vector<const char*>& returnName, std::vector<MYFLT*>& returnVal)
{
	for(int i = 0; i < returnName.size(); i++)
	{
		const char* retName = returnName[i];
		if(_session->GetChannelPtr(returnVal[i], retName, CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
		{
			std::cout << "Csound return value not available at position " << returnName[i] << std::endl;
			return false;
		}
	}
	
	return true;
}

void Studio::MLRegressionSetup()
{
	m_bPrevSaveState = false;
	m_bPrevRandomState = false;
	m_bPrevTrainState = false;
	m_bPrevHaltState = false;
	m_bPrevLoadState = false;
	m_bMsg = true;
	m_bCurrentMsgState = false;
	m_bRunMsg = true;
	m_bCurrentRunMsgState = false;
	sizeVal = 0.0f;
	m_bModelTrained = false;
}

void Studio::MLRegressionUpdate(MachineLearning& machineLearning, PBOInfo& pboInfo)
{
//*********************************************************************************************
// Machine Learning 
//*********************************************************************************************

	bool currentRandomState = m_bPrevRandomState;

	// randomise parameters
	if(machineLearning.bRandomParams != currentRandomState && machineLearning.bRandomParams == true)
	{
		//random device
		std::random_device rd;

		//random audio params
		
		// example randomised parameter
		// oscil frequency (kcps) 
		std::uniform_real_distribution<float> distFreq(50.0f, 1000.0f);
		std::default_random_engine genFreq(rd());
		float valFreq = distFreq(genFreq);
		*m_vSendVals[1] = (MYFLT)valFreq;
			
	}
	m_bPrevRandomState = machineLearning.bRandomParams;

	// record training examples
	if(machineLearning.bRecord)
	{
		//example shader values provide input to neural network
		for(int i = 0; i < pboInfo.pboSize; i+=pboInfo.pboSize * 0.01)
		{
			inputData.push_back((double)pboInfo.pboPtr[i]); //13
		}

		//neural network outputs to audio parameter
		outputData.push_back((double)*m_vSendVals[1]); //0

#ifdef __APPLE__
		trainingData.recordSingleElement(inputData, outputData);	
#elif _WIN32
		trainingData.input = inputData;
		trainingData.output = outputData;
		trainingSet.push_back(trainingData);
#endif

		std::cout << "Recording Data" << std::endl;
		inputData.clear();
		outputData.clear();
	}
	machineLearning.bRecord = false;

	// train model
	bool currentTrainState = m_bPrevTrainState;
	if(machineLearning.bTrainModel != currentTrainState && machineLearning.bTrainModel == true && TRAINING_SET_SIZE > 0)
	{

#ifdef __APPLE__
		staticRegression.train(trainingData);
#elif _WIN32
		staticRegression.train(trainingSet);
#endif
		m_bModelTrained = true;
		std::cout << "Model Trained" << std::endl;
	}	
	else if(machineLearning.bTrainModel != currentTrainState && machineLearning.bTrainModel == true && TRAINING_SET_SIZE == 0)
	{
		std::cout << "Can't train model. No training data." << std::endl;
	}

	m_bPrevTrainState = machineLearning.bTrainModel;

#ifdef __APPLE__

	// run/stop model
	bool currentHaltState = m_bPrevHaltState;
	if(machineLearning.bRunModel && !machineLearning.bHaltModel && m_bModelTrained)
	{
		std::vector<double> modelOut;
		std::vector<double> modelIn;

		for(int i = 0; i < pboInfo.pboSize; i+=pboInfo.pboSize * 0.01)
		{
			modelIn.push_back((double)pboInfo.pboPtr[i]); 
		}
		
		modelOut = staticRegression.run(modelIn);

		if(modelOut[0] > 1000.0f) modelOut[0] = 1000.0f;
		if(modelOut[0] < 50.0f) modelOut[0] = 50.0f;
		*m_vSendVals[1] = (MYFLT)modelOut[0];
		
		std::cout << "Model Running" << std::endl;
		modelIn.clear();
		modelOut.clear();
	} 
	else if(!machineLearning.bRunModel && machineLearning.bHaltModel != currentHaltState)
	{

		std::cout << "Model Stopped" << std::endl;
	}
	m_bPrevHaltState = machineLearning.bHaltModel;
#elif _WIN32
	if(machineLearning.bRunModel && m_bModelTrained)
	{
		std::vector<double> modelOut;
		std::vector<double> modelIn;

		for(int i = 0; i < pboInfo.pboSize; i+=pboInfo.pboSize * 0.01)
		{
			modelIn.push_back((double)pboInfo.pboPtr[i]); 
		}

		modelOut = staticRegression.run(modelIn);
		
		if(modelOut[0] > 1000.0f) modelOut[0] = 1000.0f;
		if(modelOut[0] < 50.0f) modelOut[0] = 50.0f;
		*m_vSendVals[1] = (MYFLT)modelOut[0];

		bool prevRunMsgState = m_bCurrentRunMsgState;
		if(m_bRunMsg != prevRunMsgState && m_bRunMsg == true)
		{
			std::cout << "Model Running" << std::endl;
			m_bRunMsg = !m_bRunMsg;
		}
		m_bCurrentRunMsgState = m_bRunMsg;

		modelIn.clear();
		modelOut.clear();
		m_bMsg = true;
	} 
	else if(!machineLearning.bRunModel)
	{
		bool prevMsgState = m_bCurrentMsgState;
		if(m_bMsg != prevMsgState && m_bMsg == true)
		{
			std::cout << "Model Stopped" << std::endl;
			m_bMsg = !m_bMsg;
		}
		m_bCurrentMsgState = m_bMsg;
		m_bRunMsg = true;
	}
#endif
		
	// save model
	std::string mySavedModel = "mySavedModel.json";
	bool currentSaveState = m_bPrevSaveState;
#ifdef __APPLE__
	if(machineLearning.bSaveTrainingData!= currentSaveState && machineLearning.bSaveTrainingData == true)
	{
		trainingData.writeJSON(mySavedModel);	
		std::cout << "Saving Training Data" << std::endl;
	}
	m_bPrevSaveState = machineLearning.bSaveTrainingData;
#elif _WIN32
	if(machineLearning.bSaveModel!= currentSaveState && machineLearning.bSaveModel == true)
	{
		staticRegression.writeJSON(mySavedModel);
		std::cout << "Saving Training Data" << std::endl;
	}
	m_bPrevSaveState = machineLearning.bSaveModel;
#endif

	// load model
	bool currentLoadState = m_bPrevLoadState;
#ifdef __APPLE__
	if(machineLearning.bLoadTrainingData != currentLoadState && machineLearning.bLoadTrainingData == true)
	{
		trainingData.readJSON(mySavedModel);
		staticRegression.train(trainingData);
		std::cout << "Loading Data and Training Model" << std::endl;
	}
	m_bPrevLoadState = machineLearning.bLoadTrainingData;
#elif _WIN32
	if(machineLearning.bLoadModel != currentLoadState && machineLearning.bLoadModel == true)
	{
		staticRegression.readJSON(mySavedModel);	
		m_bModelTrained = true;
		std::cout << "Loading Data and Training Model" << std::endl;
	}
	m_bPrevLoadState = machineLearning.bLoadModel;
#endif

}

void Studio::exit(){
	//stop csound
	session->StopPerformance();
	//close GL context and any other GL resources
	glfwTerminate();
}
