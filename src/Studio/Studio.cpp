#include "Studio.hpp"

#include <iostream>
#include <random>

#ifdef __APPLE__ 
#include "GLFW/glfw3.h"
#elif _WIN32 
#include "glfw3.h"
#endif

//*******************************************************************************************
// Setup 
//*******************************************************************************************
bool Studio::Setup(std::string csd, GLuint shaderProg)
{
	// bool to indicate first loop through update and draw functions to 
	// set initial paramaters. For reading from pboInfo in update
	m_bFirstLoop = true;
	m_fDeltaTime = 0.0f;
	m_fTargetVal = 0.0f;
	m_fCurrentVal = 0.0f;

	m_pStTools = new StudioTools();

	//audio setup
	CsoundSession* csSession = m_pStTools->PCsoundSetup(csd);
	
	if(!m_pStTools->BSoundSourceSetup(csSession, NUM_SOUND_SOURCES))
	{
		std::cout << "Studio::setup sound sources not set up" << std::endl;
		return false;
	}

	//setup sends to csound
	std::vector<const char*> sendNames;

	sendNames.push_back("sineControlVal");
	m_vSendVals.push_back(m_cspSineControlVal);	

	sendNames.push_back("randomVal");
	m_vSendVals.push_back(m_cspRandVal);

	m_pStTools->BCsoundSend(csSession, sendNames, m_vSendVals);

	//setup returns from csound 
	std::vector<const char*> returnNames;

	returnNames.push_back("pitchOut");
	m_vReturnVals.push_back(m_pPitchOut);

	returnNames.push_back("freqOut");
	m_vReturnVals.push_back(m_pFreqOut);

	m_pStTools->BCsoundReturn(csSession, returnNames, m_vReturnVals);	
	
	//setup quad to use for raymarching
	m_pStTools->RaymarchQuadSetup(shaderProg);
	
	//shader uniforms
	m_gliSineControlValLoc = glGetUniformLocation(shaderProg, "sineControlVal");
	m_gliPitchOutLoc = glGetUniformLocation(shaderProg, "pitchOut");
	m_gliFreqOutLoc = glGetUniformLocation(shaderProg, "freqOut");
	
	//machine learning setup
	MLRegressionSetup();

	return true;
}
//*******************************************************************************************


//*******************************************************************************************
// Update 
//*******************************************************************************************
void Studio::Update(glm::mat4 viewMat, MachineLearning& machineLearning, glm::vec3 controllerWorldPos_0, glm::vec3 controllerWorldPos_1, glm::quat controllerQuat_0, glm::quat controllerQuat_1, PBOInfo& pboInfo){

	// For return values from shader.
	// vec4 for each fragment is returned in the order RGBA. 
	// You have to wait until the 2nd frame to read from the buffer. 
	// At the moment, only the RGB channels are useable when the A channel is set to max.
	// The values returned are unpredictable if A channel is anything other than max.
	//if(!m_bFirstLoop)
	//std::cout << (double)pboInfo.pboPtr[0] << ":" << (double)pboInfo.pboPtr[1] << ":" << (double)pboInfo.pboPtr[2] << ":" << (double)pboInfo.pboPtr[3] << std::endl;

	// spectral pitch data processing
	m_fCurrentFrame = glfwGetTime();
	m_fDeltaTime = m_fCurrentFrame - m_fLastFrame;	
	m_fDeltaTime *= 1000.0f;
	if(*m_vReturnVals[0] > 0) m_fTargetVal = *m_vReturnVals[0];	
	if(m_fTargetVal > m_fCurrentVal)
	{
		m_fCurrentVal += m_fDeltaTime;
	} else if(m_fTargetVal <= m_fCurrentVal)
	{
		m_fCurrentVal -= m_fDeltaTime;
	} else if(m_fTargetVal == m_fCurrentVal)
	{
		m_fCurrentVal = m_fTargetVal;
	}
	if(m_fCurrentVal < 0.0f) m_fCurrentVal = 0.0f;
	m_fPitch = m_fCurrentVal;
	
	// example sound source at origin
	StudioTools::SoundSourceData soundSource1;
	glm::vec4 sourcePosWorldSpace = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	soundSource1.position = sourcePosWorldSpace;
	std::vector<StudioTools::SoundSourceData> soundSources;
	soundSources.push_back(soundSource1);

	m_pStTools->SoundSourceUpdate(soundSources, viewMat);

	//example control signal - sine function
	//sent to shader and csound
	m_fSineControlVal = sin(glfwGetTime() * 0.33f);
	*m_vSendVals[0] = (MYFLT)m_fSineControlVal;

	//run machine learning
	MLAudioParameter paramData;
	paramData.distributionLow = 400.0f;
	paramData.distributionHigh = 1000.0f;
	paramData.sendVecPosition = 1;
	std::vector<MLAudioParameter> paramVec;
	paramVec.push_back(paramData);
	MLRegressionUpdate(machineLearning, pboInfo, paramVec);	
}
//*********************************************************************************************


//*********************************************************************************************
// Draw 
//*********************************************************************************************
void Studio::Draw(glm::mat4 projMat, glm::mat4 viewMat, glm::mat4 eyeMat, GLuint shaderProg, glm::vec3 translateVec)
{
	m_pStTools->DrawStart(projMat, eyeMat, viewMat, shaderProg, translateVec);
	
	glUniform1f(m_gliSineControlValLoc, m_fSineControlVal);
	glUniform1f(m_gliPitchOutLoc, m_fPitch);
	glUniform1f(m_gliFreqOutLoc, *m_vReturnVals[1]);

	m_pStTools->DrawEnd();

	// update first loop switch
	m_bFirstLoop = false;
	// set end of frame timestamp
	m_fLastFrame = m_fCurrentFrame;
}
//*********************************************************************************************



//*********************************************************************************************
// Machine Learning 
//*********************************************************************************************
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

void Studio::MLRegressionUpdate(MachineLearning& machineLearning, PBOInfo& pboInfo, std::vector<MLAudioParameter>& params)
{

	bool currentRandomState = m_bPrevRandomState;

	// randomise parameters
	if(machineLearning.bRandomParams != currentRandomState && machineLearning.bRandomParams == true)
	{
		//random device
		std::random_device rd;

		//random audio params
		for(int i = 0; i < params.size(); i++)
		{

			std::uniform_real_distribution<float> distribution(params[i].distributionLow, params[i].distributionHigh);
			std::default_random_engine generator(rd());
			float val = distribution(generator);
			*m_vSendVals[params[i].sendVecPosition] = (MYFLT)val;		
		}
			
	}
	m_bPrevRandomState = machineLearning.bRandomParams;

	// record training examples
	if(machineLearning.bRecord)
	{
		//shader values provide input to neural network
		for(int i = 0; i < pboInfo.pboSize; i+=pboInfo.pboSize * 0.01)
		{
			inputData.push_back((double)pboInfo.pboPtr[i]);
		}

		//neural network outputs to audio engine 
		for(int i = 0; i < params.size(); i++)
		{
			outputData.push_back((double)*m_vSendVals[params[i].sendVecPosition]);
		}

		trainingData.input = inputData;
		trainingData.output = outputData;
		trainingSet.push_back(trainingData);

		std::cout << "Recording Data" << std::endl;
		inputData.clear();
		outputData.clear();
	}
	machineLearning.bRecord = false;

	// train model
	bool currentTrainState = m_bPrevTrainState;
	if(machineLearning.bTrainModel != currentTrainState && machineLearning.bTrainModel == true && trainingSet.size() > 0)
	{

		staticRegression.train(trainingSet);
		m_bModelTrained = true;
		std::cout << "Model Trained" << std::endl;
	}	
	else if(machineLearning.bTrainModel != currentTrainState && machineLearning.bTrainModel == true && trainingSet.size() == 0)
	{
		std::cout << "Can't train model. No training data." << std::endl;
	}

	m_bPrevTrainState = machineLearning.bTrainModel;

	//run and halt model
	if(machineLearning.bDevMode)
	{
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
			
			for(int i = 0; i < modelOut.size(); i++)
			{
				if(modelOut[i] > params[i].distributionHigh) modelOut[i] = params[i].distributionHigh;
				if(modelOut[i] < params[i].distributionLow) modelOut[i] = params[i].distributionLow;
				*m_vSendVals[params[i].sendVecPosition] = (MYFLT)modelOut[i];
			}
			
			std::cout << "Model Running" << std::endl;
			modelIn.clear();
			modelOut.clear();
		} 
		else if(!machineLearning.bRunModel && machineLearning.bHaltModel != currentHaltState)
		{

			std::cout << "Model Stopped" << std::endl;
		}
		m_bPrevHaltState = machineLearning.bHaltModel;
	} else 
	{
		if(machineLearning.bRunModel && m_bModelTrained)
		{
			std::vector<double> modelOut;
			std::vector<double> modelIn;

			for(int i = 0; i < pboInfo.pboSize; i+=pboInfo.pboSize * 0.01)
			{
				modelIn.push_back((double)pboInfo.pboPtr[i]); 
			}

			modelOut = staticRegression.run(modelIn);
			
			for(int i = 0; i < modelOut.size(); i++)
			{
				if(modelOut[i] > params[i].distributionHigh) modelOut[i] = params[i].distributionHigh;;
				if(modelOut[i] < params[i].distributionLow) modelOut[i] = params[i].distributionLow;
				*m_vSendVals[params[i].sendVecPosition] = (MYFLT)modelOut[i];
			}

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
	}
		
	// save model
	std::string mySavedModel = "mySavedModel_cyclicalEx.json";
	bool currentSaveState = m_bPrevSaveState;
	if(machineLearning.bSaveModel!= currentSaveState && machineLearning.bSaveModel == true)
	{
		staticRegression.writeJSON(mySavedModel);
		std::cout << "Saving Training Data" << std::endl;
	}
	m_bPrevSaveState = machineLearning.bSaveModel;

	// load model
	bool currentLoadState = m_bPrevLoadState;
	if(machineLearning.bLoadModel != currentLoadState && machineLearning.bLoadModel == true)
	{
		staticRegression.reset();
		staticRegression.readJSON(mySavedModel);	
		m_bModelTrained = true;
		std::cout << "Loading Data and Training Model" << std::endl;
	}
	m_bPrevLoadState = machineLearning.bLoadModel;
}
//*********************************************************************************************


//*********************************************************************************************
// Clean up
//*********************************************************************************************
void Studio::Exit(){

	//delete StudioTools pointer
	m_pStTools->Exit();
	delete m_pStTools;

	//close GL context and any other GL resources
	glfwTerminate();
}
