#include "FiveCell.hpp"

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

bool FiveCell::setup(std::string csd)
{

//************************************************************
//Csound performance thread
//************************************************************

	// bool to indicate first loop through update and draw functions to 
	// set initial paramaters
	m_bFirstLoop = true;

	// initialise val for mapping
	m_fPrevSpecCentVal = 0.0f;

	std::string csdName = "";
	if(!csd.empty()) csdName = csd;
	session = new CsoundSession(csdName);

#ifdef _WIN32
	session->SetOption("-b -32"); 
	session->SetOption("-B 2048");
#endif
	session->StartThread();
	session->PlayScore();

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
	
	//std::string val4 = "azimuth2";
	//const char* azimuth2 = val4.c_str();	
	//if(session->GetChannelPtr(hrtfVals[3], azimuth2, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	//{
	//	std::cout << "GetChannelPtr could not get the azimuth2 input" << std::endl;
	//	return false;
	//}

	//std::string val5 = "elevation2";
	//const char* elevation2 = val5.c_str();
	//if(session->GetChannelPtr(hrtfVals[4], elevation2, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	//{
	//	std::cout << "GetChannelPtr could not get the elevation2 input" << std::endl;
	//	return false;
	//}	

	//std::string val6 = "distance2";
	//const char* distance2 = val6.c_str();
	//if(session->GetChannelPtr(hrtfVals[5], distance2, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	//{
	//	std::cout << "GetChannelPtr could not get the distance2 input" << std::endl;
	//	return false;
	//}

	const char* grainFreq = "grainFreq";
	if(session->GetChannelPtr(m_cspGrainFreq, grainFreq, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the grainFreq value" << std::endl;
		return false;
	} 

	const char* grainPhase = "grainPhse";
	if(session->GetChannelPtr(m_cspGrainPhase, grainPhase, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the randAmp value" << std::endl;
		return false;
	}

	const char* randFreq = "randFreq";
	if(session->GetChannelPtr(m_cspRandFreq, randFreq, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the randFreq value" << std::endl;
		return false;
	}

	const char* randPhase = "randPhase";
	if(session->GetChannelPtr(m_cspRandPhase, randPhase, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the randPhase value" << std::endl;
		return false;
	}

	const char* grainDur = "grainDur";
	if(session->GetChannelPtr(m_cspGrainDur, grainDur, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the grainDur value" << std::endl;
		return false;
	}

	const char* grainDensity = "grainDensity";
	if(session->GetChannelPtr(m_cspGrainDensity, grainDensity, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the grainDensity value" << std::endl;
		return false;
	}

	const char* grainFreqVariationDistrib = "grainFreqVariationDistrib";
	if(session->GetChannelPtr(m_cspGrainFreqVariationDistrib, grainFreqVariationDistrib, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the grainFreqVariationDistrib value" << std::endl;
		return false;
	}

	const char* grainPhaseVariationDistrib = "grainPhaseVariationDistrib";
	if(session->GetChannelPtr(m_cspGrainPhaseVariationDistrib, grainPhaseVariationDistrib, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the grainPhaseVariationDistrib value" << std::endl;
		return false;
	}

	const char* grainWaveform = "grainWaveform";
	if(session->GetChannelPtr(m_cspGrainWaveform, grainWaveform, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the grainWaveform value" << std::endl;
		return false;
	}

	const char* sineVal = "sineControlVal";
	if(session->GetChannelPtr(m_cspSineControlVal, sineVal, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the sineControlVal value" << std::endl;
		return false;
	}

	const char* gaussRange = "gaussRange";
	if(session->GetChannelPtr(m_cspGaussRange, gaussRange, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "GetChannelPtr could not get the gaussRange value" << std::endl;
		return false;
	}

	//for(int i = 0; i < MAX_MANDEL_STEPS; i++){

	//	std::string mandelEscapeValString = "mandelEscapeVal" + std::to_string(i);
	//	const char* mandelEscapeVal = mandelEscapeValString.c_str();
	//	if(session->GetChannelPtr(m_cspMandelEscapeVals[i], mandelEscapeVal, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
	//		std::cout << "GetChannelPtr could not get the mandelEscapeVal " << std::to_string(i) << " value" << std::endl;
	//		return false;
	//	}

	//}
	
	//const char* mandelMaxPoints = "mandelMaxPoints";
	//if(session->GetChannelPtr(m_cspMaxSteps, mandelMaxPoints, CSOUND_INPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0){
	//	std::cout << "GetChannelPtr could not get the mandelMaxPoints value" << std::endl;
	//	return false;
	//}

//********* output values from csound to avr *******************//

	m_fPrevRms = 0.0f;
	const char* rmsOut = "rmsOut";
	if(session->GetChannelPtr(m_pRmsOut, rmsOut, CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "Csound output value rmsOut not available" << std::endl;
		return false;
	}
	
	for(int i = 0; i < NUM_FFT_BINS; i++)
	{
		fftAmpBinsOut[i] = "fftAmpBin" + std::to_string(i);
		if(session->GetChannelPtr(m_pFftAmpBinOut[i], fftAmpBinsOut[i].c_str(), CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
		{
		std::cout << "Csound output value fftAmpBinsOut" << std::to_string(i) << " not available" << std::endl;
		return false;
		}
	}
	
	const char* specCentOut = "specCentOut";
	if(session->GetChannelPtr(m_cspSpecCentOut, specCentOut, CSOUND_OUTPUT_CHANNEL | CSOUND_CONTROL_CHANNEL) != 0)
	{
		std::cout << "Csound output value specCentOut not available" << std::endl;
		return false;
	}
//**********************************************************


//**********************************************************
// Lighting Components
//**********************************************************

	m_vec3MoonDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
	m_vec3MoonColour = glm::vec3(0.86f, 0.9f, 0.88f);
	m_vec3MoonAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	m_vec3MoonDiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
	m_vec3MoonSpecular = glm::vec3(1.0f, 1.0f, 1.0f);

//**********************************************************
// Material Properties
//**********************************************************

	//Ground
	m_vec3GroundColour = glm::vec3(0.0f, 0.0f, 0.0f);
	m_vec3GroundAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	m_vec3GroundDiffuse = glm::vec3(0.2f, 0.2f, 0.2f);	
	m_vec3GroundSpecular = glm::vec3(0.2f, 0.2f, 0.2f);
	m_fGroundShininess = 8.0f;

	//Cube
	m_vec3CubeAmbient = glm::vec3(0.1f, 0.1f, 0.1f);
	m_vec3CubeDiffuse = glm::vec3(0.2f, 0.2f, 0.2f);
	m_vec3CubeSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
	m_fCubeShininess = 256.0f;

//*********************************************************************************************
// Machine Learning
//********************************************************************************************

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

//********************************************************************************************

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//******************************************************************************************
// Matrices & Light Positions
//*******************************************************************************************
	
	//model matrix
	modelMatrix = glm::mat4(1.0f);

//*********************************************************************************************
	return true;
}

bool FiveCell::BSetupRaymarchQuad(GLuint shaderProg)
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

	m_uiglCubeMoonDirectionLoc = glGetUniformLocation(shaderProg, "moonlight.direction");
	m_uiglCubeMoonColourLoc = glGetUniformLocation(shaderProg, "moonlight.colour");
	m_uiglCubeMoonAmbientLoc = glGetUniformLocation(shaderProg, "moonlight.ambient");
	m_uiglCubeMoonDiffuseLoc = glGetUniformLocation(shaderProg, "moonlight.diffuse");
	m_uiglCubeMoonSpecularLoc = glGetUniformLocation(shaderProg, "moonlight.specular");

	m_uiglCubeMaterialAmbientLoc = glGetUniformLocation(shaderProg, "material.ambient");
	m_uiglCubeMaterialDiffuseLoc = glGetUniformLocation(shaderProg, "material.diffuse");
	m_uiglCubeMaterialSpecularLoc = glGetUniformLocation(shaderProg, "material.specular");
	m_uiglCubeMaterialShininessLoc = glGetUniformLocation(shaderProg, "material.shininess");

	m_uiglGroundPlaneColourLoc = glGetUniformLocation(shaderProg, "ground.colour");
	m_uiglGroundPlaneAmbientLoc = glGetUniformLocation(shaderProg, "ground.ambient");
	m_uiglGroundPlaneDiffuseLoc = glGetUniformLocation(shaderProg, "ground.diffuse");
	m_uiglGroundPlaneSpecularLoc = glGetUniformLocation(shaderProg, "ground.specular");
	m_uiglGroundPlaneShininessLoc = glGetUniformLocation(shaderProg, "ground.shininess");
	
	m_gliMVEPMatrixLocation = glGetUniformLocation(shaderProg, "MVEPMat");
	m_gliInverseMVEPLocation = glGetUniformLocation(shaderProg, "InvMVEP");
	m_gliRandomSizeLocation = glGetUniformLocation(shaderProg, "randSize");
	m_gliValBinScaleLoc = glGetUniformLocation(shaderProg, "fftBinValScale");
	m_gliRMSModulateValLocation = glGetUniformLocation(shaderProg, "rmsModVal");
	m_gliSpecCentOutLoc = glGetUniformLocation(shaderProg, "specCentVal");
	m_gliHighFreqAvgLoc = glGetUniformLocation(shaderProg, "highFreqVal");
	m_gliLowFreqAvgLoc = glGetUniformLocation(shaderProg, "lowFreqVal");
	m_gliSineControlValLoc = glGetUniformLocation(shaderProg, "sineControlVal");
	m_gliNumFftBinsLoc = glGetUniformLocation(shaderProg, "numFftBins");
	m_gliThetaAngleLoc = glGetUniformLocation(shaderProg, "thetaScale");
	m_gliPhiAngleLoc = glGetUniformLocation(shaderProg, "phiScale");

	m_uiglSkyboxTexLoc = glGetUniformLocation(shaderProg, "skyboxTex");
	m_uiglGroundTexLoc = glGetUniformLocation(shaderProg, "ground.texture");
	m_gluiFftAmpBinsLoc = glGetUniformLocation(shaderProg, "fftAmpBins");
	m_gliTimeValLoc = glGetUniformLocation(shaderProg, "timeVal");

	return true;
}

//*******************************************************************************************
// Update Stuff Here
//*******************************************************************************************
void FiveCell::update(glm::mat4 viewMat, glm::vec3 camPos, MachineLearning& machineLearning, glm::vec3 controllerWorldPos_0, glm::vec3 controllerWorldPos_1, glm::quat controllerQuat_0, glm::quat controllerQuat_1, PBOInfo& pboInfo, glm::vec3 translateVec){

	modelMatrix = glm::mat4(1.0f);

	m_vec3Translation = translateVec;
	//std::cout << m_vec3Translation.x << "	" << m_vec3Translation.y << "	" << m_vec3Translation.z << std::endl;
	m_fTranslationMag = glm::length(m_vec3Translation);

	//rms value from Csound
	float avgRms = (*m_pRmsOut + m_fPrevRms) / 2;
	
	modulateVal = avgRms;			
	
	m_fPrevRms = *m_pRmsOut;

	// spectral centroid value from csound
	//std::cout << "Spectral Centroid Value : " << *m_cspSpecCentOut << std::endl; 
	if(*m_cspSpecCentOut > 0)
	{
		float currentSpecCentVal = *m_cspSpecCentOut;
		float lerpFraction = 0.8f;
		m_fInterpolatedSpecCentVal = currentSpecCentVal + lerpFraction * (m_fPrevSpecCentVal - currentSpecCentVal);
		m_fPrevSpecCentVal = currentSpecCentVal;
	}	

	double lowFreqVals = 0.0f;
	double highFreqVals = 0.0f;

	//fft frequency bin values from Csound
	for(int i = 0; i < NUM_FFT_BINS; i++)
	{
		//std::cout << *m_pFftAmpBinOut[i] << std::endl;	
		if(i < 342 && i > 0)
		{
			lowFreqVals += *m_pFftAmpBinOut[i];
		}
		else if(i < NUM_FFT_BINS && i >= 342)
		{
			highFreqVals += *m_pFftAmpBinOut[i];
		}
	}	

	m_dLowFreqAvg = lowFreqVals / 341;
	m_dHighFreqAvg = highFreqVals / 171;	

	//std::cout << "Average amplitudes in low bins: " << m_dLowFreqAvg << std::endl;
	//std::cout << "Average amplitudes in high bins: " << m_dHighFreqAvg << std::endl;

	glm::vec4 viewerPosCameraSpace = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	glm::vec4 pos1 = glm::vec4(-4.0f, 0.0f, 0.0f, 1.0f);
	float newPosX = (pos1.x * cos(glfwGetTime())) - (pos1.y * sin(glfwGetTime()));
	float newPosY = (pos1.x * sin(glfwGetTime())) + (pos1.y * cos(glfwGetTime())); 
	glm::vec4 rotPos = glm::vec4(newPosX, newPosY, 0.0f, 1.0f);
	SoundSourceData soundSource1;
	soundSource1.position = rotPos;
	m_vSoundSources.push_back(soundSource1);

	glm::vec4 pos2 = glm::vec4(4.0f, 0.0f, 0.0f, 1.0f);
	float newPosX2 = (pos2.x * cos(glfwGetTime())) - (pos2.y * sin(glfwGetTime()));
	float newPosY2 = (pos2.x * sin(glfwGetTime())) + (pos2.y * cos(glfwGetTime())); 
	glm::vec4 rotPos2 = glm::vec4(newPosX2, newPosY2, 0.0f, 1.0f);
	SoundSourceData soundSource2;
	soundSource2.position = rotPos2;
 	m_vSoundSources.push_back(soundSource2);
	
	glm::vec4 pos3 = glm::vec4(0.0f, 0.0f, -2.0f, 1.0f);
	float newPosZ3 = (pos3.z * cos(glfwGetTime())) - (pos3.y * sin(glfwGetTime()));
	float newPosY3 = (pos3.z * sin(glfwGetTime())) + (pos3.y * cos(glfwGetTime())); 
	glm::vec4 rotPos3 = glm::vec4(0.0f, newPosY3, newPosZ3, 1.0f);
	SoundSourceData soundSource3;
	soundSource3.position = rotPos3;
 	m_vSoundSources.push_back(soundSource3);

	for(int i = 0; i < NUM_SOUND_SOURCES; i++)
	{
		// camera space positions
		m_vSoundSources[i].posCamSpace = viewMat * modelMatrix * m_vSoundSources[i].position;

		// distance value
		m_vSoundSources[i].distCamSpace = sqrt(pow(m_vSoundSources[i].posCamSpace.x, 2) + pow(m_vSoundSources[i].posCamSpace.y, 2) + pow(m_vSoundSources[i].posCamSpace.z, 2));

		//azimuth in camera space
		float valX = m_vSoundSources[i].posCamSpace.x - viewerPosCameraSpace.x;
		float valZ = m_vSoundSources[i].posCamSpace.z - viewerPosCameraSpace.z;

		m_vSoundSources[i].azimuth = atan2(valX, valZ);
		m_vSoundSources[i].azimuth *= (180.0f/PI); 	

		//elevation in camera space
		float oppSide = m_vSoundSources[i].posCamSpace.y - viewerPosCameraSpace.y;
		float sinVal = oppSide / m_vSoundSources[i].distCamSpace;
		m_vSoundSources[i].elevation = asin(sinVal);
		m_vSoundSources[i].elevation *= (180.0f/PI);		

		//send values to Csound pointers
		*azimuthVals[i] = (MYFLT)m_vSoundSources[i].azimuth;
		*elevationVals[i] = (MYFLT)m_vSoundSources[i].elevation;
		*distanceVals[i] = (MYFLT)m_vSoundSources[i].distCamSpace;
	}
	
	m_vSoundSources.clear();

	//sine function
	sineControlVal = sin(glfwGetTime() * 0.15f);

	*m_cspSineControlVal = (MYFLT)sineControlVal;

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
		
		// grain3 parameters

		// grain frequency (kcps) 
		std::uniform_real_distribution<float> distGrainFreq(50.0f, 1000.0f);
		std::default_random_engine genGrainFreq(rd());
		float valGrainFreq = distGrainFreq(genGrainFreq);
		*m_cspGrainFreq = (MYFLT)valGrainFreq;
			
		// grain phase (kphs) 
		std::uniform_real_distribution<float> distGrainPhase(0.0f, 1.0f);
		std::default_random_engine genGrainPhase(rd());
		float valGrainPhase = distGrainPhase(genGrainPhase);
		*m_cspGrainPhase = (MYFLT)valGrainPhase;

		// random variation in grain frequency (kfmd) 
		std::uniform_real_distribution<float> distRandFreq(1.0f, 500.0f);
		std::default_random_engine genRandFreq(rd());
		float valRandFreq = distRandFreq(genRandFreq);
		*m_cspRandFreq = (MYFLT)valRandFreq;	

		// random variation in phase (kpmd) 
		std::uniform_real_distribution<float> distRandPhase(0.0f, 1.0f);
		std::default_random_engine genRandPhase(rd());
		float valRandPhase = distRandPhase(genRandPhase);
		*m_cspRandPhase = (MYFLT)valRandPhase;

		// grain duration (kgdur)
		std::uniform_real_distribution<float> distGrainDur(0.01f, 0.2f);
		std::default_random_engine genGrainDur(rd());
		float valGrainDur = distGrainDur(genGrainDur);
		if(m_bFirstLoop) 
		{	
			*m_cspGrainDur = (MYFLT)0.08f;
		} 
		else
		{
			*m_cspGrainDur = (MYFLT)valGrainDur;
		}

		// grain density (kdens)
		std::uniform_real_distribution<float> distGrainDensity(50.0f, 500.0f);
		std::default_random_engine genGrainDensity(rd());
		float valGrainDensity = floor(distGrainDensity(genGrainDensity));
		*m_cspGrainDensity = (MYFLT)valGrainDensity;

		// distribution of random grain frequency variation (kfrpow)
		std::uniform_real_distribution<float> distGrainFreqVariationDistrib(-1.0f, 1.0f);
		std::default_random_engine genGrainFreqVariationDistrib(rd());
		float valGrainFreqVariationDistrib = distGrainFreqVariationDistrib(genGrainFreqVariationDistrib);
		*m_cspGrainFreqVariationDistrib = (MYFLT)valGrainFreqVariationDistrib;

		// distribution of random grain phase variation (kprpow)
		std::uniform_real_distribution<float> distGrainPhaseVariationDistrib(-1.0f, 1.0f);
		std::default_random_engine genGrainPhaseVariationDistrib(rd());
		float valGrainPhaseVariationDistrib = distGrainPhaseVariationDistrib(genGrainPhaseVariationDistrib);
		*m_cspGrainPhaseVariationDistrib = (MYFLT)valGrainPhaseVariationDistrib;

		// grain waveform (kfn)
		std::uniform_real_distribution<float> distGrainWaveform(1.0f, 4.0f);
		std::default_random_engine genGrainWaveform(rd());
		float valGrainWaveform = floor(distGrainWaveform(genGrainWaveform));
		*m_cspGrainWaveform = (MYFLT)valGrainWaveform;

		// wgbow params
		
		// percentage deviation range
		std::uniform_real_distribution<float> distGaussRange(1.0f, 10.0f);
		std::default_random_engine genGaussRange(rd());
		float valGaussRange = floor(distGaussRange(genGaussRange));
		*m_cspGaussRange = (MYFLT)valGaussRange;

	}
	m_bPrevRandomState = machineLearning.bRandomParams;

	// record training examples
	if(machineLearning.bRecord)
	{
		//input every 100th orbit value from frag shader = 51,840 on the mbp 
		for(int i = 0; i < pboInfo.pboSize; i+=pboInfo.pboSize * 0.01)
		{
			inputData.push_back((double)pboInfo.pboPtr[i]); //13
		}

		outputData.push_back((double)*m_cspGrainFreq); //0
		outputData.push_back((double)*m_cspGrainPhase); //1
		outputData.push_back((double)*m_cspRandFreq); //2
		outputData.push_back((double)*m_cspRandPhase); //3
		outputData.push_back((double)*m_cspGrainDur); //4
		outputData.push_back((double)*m_cspGrainDensity); //5
		outputData.push_back((double)*m_cspGrainFreqVariationDistrib); //6
		outputData.push_back((double)*m_cspGrainPhaseVariationDistrib); //7
		outputData.push_back((double)*m_cspGrainWaveform); //8
		outputData.push_back((double)*m_cspGaussRange); //9

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
		*m_cspGrainFreq = (MYFLT)modelOut[0];

		if(modelOut[1] > 1.0f) modelOut[1] = 1.0f;
		if(modelOut[1] < 0.0f) modelOut[1] = 0.0f;
		*m_cspGrainPhase = (MYFLT)modelOut[1];

		if(modelOut[2] > 500.0f) modelOut[2] = 500.0f;
		if(modelOut[2] < 1.0f) modelOut[2] = 1.0f;
		*m_cspRandFreq = (MYFLT)modelOut[2];

		if(modelOut[3] > 1.0f) modelOut[3] = 1.0f;
		if(modelOut[3] < 0.0f) modelOut[3] = 0.0f;
		*m_cspRandPhase = (MYFLT)modelOut[3];

		if(modelOut[4] > 0.2f) modelOut[4] = 0.2f;
		if(modelOut[4] < 0.01f) modelOut[4] = 0.01f;
		*m_cspGrainDur = (MYFLT)modelOut[4];

		if(modelOut[5] > 500.0f) modelOut[5] = 500.0f;
		if(modelOut[5] < 50.0f) modelOut[5] = 50.0f;
		*m_cspGrainDensity = (MYFLT)floor(modelOut[5]);
	
		if(modelOut[6] > 1.0f) modelOut[6] = 1.0f;
		if(modelOut[6] < -1.0f) modelOut[6] = -1.0f;
		*m_cspGrainFreqVariationDistrib = (MYFLT)modelOut[6];

		if(modelOut[7] > 1.0f) modelOut[7] = 1.0f;
		if(modelOut[7] < -1.0f) modelOut[7] = -1.0f;
		*m_cspGrainPhaseVariationDistrib = (MYFLT)modelOut[7];

		if(modelOut[8] > 4.0f) modelOut[8] = 4.0f;
		if(modelOut[8] < 1.0f) modelOut[8] = 1.0f;
		*m_cspGrainWaveform = (MYFLT)floor(modelOut[8]);

		if(modelOut[9] > 10.0f) modelOut[9] = 10.0f;
		if(modelOut[9] < 1.0f) modelOut[9] = 1.0f;
		*m_cspGaussRange = (MYFLT)modelOut[9];

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
		*m_cspGrainFreq = (MYFLT)modelOut[0];

		if(modelOut[1] > 1.0f) modelOut[1] = 1.0f;
		if(modelOut[1] < 0.0f) modelOut[1] = 0.0f;
		*m_cspGrainPhase = (MYFLT)modelOut[1];

		if(modelOut[2] > 500.0f) modelOut[2] = 500.0f;
		if(modelOut[2] < 1.0f) modelOut[2] = 1.0f;
		*m_cspRandFreq = (MYFLT)modelOut[2];

		if(modelOut[3] > 1.0f) modelOut[3] = 1.0f;
		if(modelOut[3] < 0.0f) modelOut[3] = 0.0f;
		*m_cspRandPhase = (MYFLT)modelOut[3];

		if(modelOut[4] > 0.2f) modelOut[4] = 0.2f;
		if(modelOut[4] < 0.01f) modelOut[4] = 0.01f;
		*m_cspGrainDur = (MYFLT)modelOut[4];

		if(modelOut[5] > 500.0f) modelOut[5] = 500.0f;
		if(modelOut[5] < 50.0f) modelOut[5] = 50.0f;
		*m_cspGrainDensity = (MYFLT)floor(modelOut[5]);
	
		if(modelOut[6] > 1.0f) modelOut[6] = 1.0f;
		if(modelOut[6] < -1.0f) modelOut[6] = -1.0f;
		*m_cspGrainFreqVariationDistrib = (MYFLT)modelOut[6];

		if(modelOut[7] > 1.0f) modelOut[7] = 1.0f;
		if(modelOut[7] < -1.0f) modelOut[7] = -1.0f;
		*m_cspGrainPhaseVariationDistrib = (MYFLT)modelOut[7];

		if(modelOut[8] > 4.0f) modelOut[8] = 4.0f;
		if(modelOut[8] < 1.0f) modelOut[8] = 1.0f;
		*m_cspGrainWaveform = (MYFLT)floor(modelOut[8]);

		if(modelOut[9] > 10.0f) modelOut[9] = 10.0f;
		if(modelOut[9] < 1.0f) modelOut[9] = 1.0f;
		*m_cspGaussRange = (MYFLT)modelOut[9];

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
//*********************************************************************************************

//*********************************************************************************************
// Draw Stuff Here
//*********************************************************************************************
void FiveCell::draw(glm::mat4 projMat, glm::mat4 viewMat, glm::mat4 eyeMat, RaymarchData& raymarchData, GLuint mengerProg)
{
	//std::cout << m_vec3Translation.x << "	" << m_vec3Translation.y << "	" << m_vec3Translation.z << std::endl;
	//std::cout << m_fTranslationMag << std::endl;
	//if(m_fTranslationMag <= 30.0f)
	//{
		//std::cout << "below 30" << std::endl;
		modelMatrix = glm::translate(modelMatrix, m_vec3Translation);
	//}
	//else
	//{
	      //std::cout << "above 30" << std::endl;
	//      float scaleVal = -m_fTranslationMag;
	//      modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleVal));	
	//}
	//matrices for raymarch shaders
	modelViewEyeProjectionMat = projMat * eyeMat * viewMat * modelMatrix;
	inverseMVEPMat = glm::inverse(modelViewEyeProjectionMat);

	//draw glass mandelbulb -----------------------------------------------------------------
	float mengerAspect = raymarchData.aspect;
	float mengerTanFovYOver2 = raymarchData.tanFovYOver2;

	glBindVertexArray(m_uiglSceneVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiglIndexBuffer);

	glUseProgram(mengerProg);

	glUniform1i(m_uiglSkyboxTexLoc, 0);
	glUniform1i(m_uiglGroundTexLoc, 1);
	glUniformMatrix4fv(m_gliMVEPMatrixLocation, 1, GL_FALSE, &modelViewEyeProjectionMat[0][0]);
	glUniformMatrix4fv(m_gliInverseMVEPLocation, 1, GL_FALSE, &inverseMVEPMat[0][0]);

	glUniform3f(m_uiglCubeMoonDirectionLoc, m_vec3MoonDirection.x, m_vec3MoonDirection.y, m_vec3MoonDirection.z);
	glUniform3f(m_uiglCubeMoonColourLoc, m_vec3MoonColour.x, m_vec3MoonColour.y, m_vec3MoonColour.z);
	glUniform3f(m_uiglCubeMoonAmbientLoc, m_vec3MoonAmbient.x, m_vec3MoonAmbient.y, m_vec3MoonAmbient.z);
	glUniform3f(m_uiglCubeMoonDiffuseLoc, m_vec3MoonDiffuse.x, m_vec3MoonDiffuse.y, m_vec3MoonDiffuse.z);
	glUniform3f(m_uiglCubeMoonSpecularLoc, m_vec3MoonSpecular.x, m_vec3MoonSpecular.y, m_vec3MoonSpecular.z);

	glUniform3f(m_uiglCubeMaterialAmbientLoc, m_vec3CubeAmbient.x, m_vec3CubeAmbient.y, m_vec3CubeAmbient.z);
	glUniform3f(m_uiglCubeMaterialDiffuseLoc, m_vec3CubeDiffuse.x, m_vec3CubeDiffuse.y, m_vec3CubeDiffuse.z);
	glUniform3f(m_uiglCubeMaterialSpecularLoc, m_vec3CubeSpecular.x, m_vec3CubeSpecular.y, m_vec3CubeSpecular.z);
	glUniform1f(m_uiglCubeMaterialShininessLoc, m_fCubeShininess);

	glUniform3f(m_uiglGroundPlaneColourLoc, m_vec3GroundColour.x, m_vec3GroundColour.y, m_vec3GroundColour.z);
	glUniform3f(m_uiglGroundPlaneAmbientLoc, m_vec3GroundAmbient.x, m_vec3GroundAmbient.y, m_vec3GroundAmbient.z);
	glUniform3f(m_uiglGroundPlaneDiffuseLoc, m_vec3GroundDiffuse.x, m_vec3GroundDiffuse.y, m_vec3GroundDiffuse.z);
	glUniform3f(m_uiglGroundPlaneSpecularLoc, m_vec3GroundSpecular.x, m_vec3GroundSpecular.y, m_vec3GroundSpecular.z);
	glUniform1f(m_uiglGroundPlaneShininessLoc, m_fGroundShininess);
	
	glUniform1f(m_gliRandomSizeLocation, sizeVal);
	glUniform1f(m_gliRMSModulateValLocation, modulateVal);
	glUniform1f(m_gliSpecCentOutLoc, m_fInterpolatedSpecCentVal);
	glUniform1f(m_gliHighFreqAvgLoc, m_dHighFreqAvg);
	glUniform1f(m_gliLowFreqAvgLoc, m_dLowFreqAvg);
	glUniform1f(m_gliSineControlValLoc, sineControlVal);
	glUniform1fv(m_gluiFftAmpBinsLoc, NUM_FFT_BINS, (float*)&m_pFftAmpBinOut); 
	//glUniform1i(m_gliNumFftBinsLoc, NUM_FFT_BINS);
	glUniform1f(m_gliTimeValLoc, glfwGetTime() * 0.1);
	//glUniform1f(m_gliValBinScaleLoc, valBinScale);
	//glUniform1f(m_gliThetaAngleLoc, valThetaScale);
	//glUniform1f(m_gliPhiAngleLoc, valPhiScale);
	
	glDrawElements(GL_TRIANGLES, m_uiNumSceneIndices * sizeof(unsigned int), GL_UNSIGNED_INT, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// update first loop switch
	m_bFirstLoop = false;

}


void FiveCell::exit(){
	//stop csound
	session->StopPerformance();
	//close GL context and any other GL resources
	glfwTerminate();
}
