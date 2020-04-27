#include "Studio.hpp"

#include <iostream>
#include <random>

#ifdef __APPLE__ 
#include "GLFW/glfw3.h"
#define TRAINING_SET_SIZE trainingData.trainingSet.size()
#elif _WIN32 
#include "glfw3.h"
#define TRAINING_SET_SIZE trainingSet.size()
#endif

//*******************************************************************************************
// Setup 
//*******************************************************************************************
bool Studio::Setup(std::string csd, GLuint shaderProg)
{
	// bool to indicate first loop through update and draw functions to 
	// set initial paramaters. For reading from pboInfo in update
	m_bFirstLoop = true;

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
	returnNames.push_back("rmsOut");
	m_vReturnVals.push_back(m_pRmsOut);
	m_pStTools->BCsoundReturn(csSession, returnNames, m_vReturnVals);	
	
	//setup quad to use for raymarching
	m_pStTools->RaymarchQuadSetup(shaderProg);
	
	//shader uniforms
	m_gliSineControlValLoc = glGetUniformLocation(shaderProg, "sineControlVal");
	m_gliRmsOutLoc = glGetUniformLocation(shaderProg, "rmsOut");
	
	//machine learning setup
	MLRegressionSetup();

	return true;
}
//*******************************************************************************************


//*******************************************************************************************
// Update 
//*******************************************************************************************
void Studio::Update(glm::mat4 viewMat, MachineLearning& machineLearning, glm::vec3 controllerWorldPos_0, glm::vec3 controllerWorldPos_1, glm::quat controllerQuat_0, glm::quat controllerQuat_1, PBOInfo& pboInfo){

	// For return values from shader
	// vec4 for each fragment is returned in the order ABGR
	// you have to wait until the 2nd frame to read from the buffer 
	//if(!m_bFirstLoop)
	//std::cout << (double)pboInfo.pboPtr[0] << std::endl;

	// example sound source at origin
	StudioTools::SoundSourceData soundSource1;
	glm::vec4 viewerPosCameraSpace = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	soundSource1.position = viewerPosCameraSpace;
	std::vector<StudioTools::SoundSourceData> soundSources;
	soundSources.push_back(soundSource1);

	m_pStTools->SoundSourceUpdate(soundSources, viewMat);

	//example control signal - sine function
	//sent to shader and csound
	m_fSineControlVal = sin(glfwGetTime() * 0.33f);
	*m_vSendVals[0] = (MYFLT)m_fSineControlVal;

	//run machine learning
	MLAudioParameter paramData;
	paramData.distributionLow = 50.0f;
	paramData.distributionHigh = 200.0f;
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
	glUniform1f(m_gliRmsOutLoc, *m_vReturnVals[0]);

	m_pStTools->DrawEnd();

	// update first loop switch
	m_bFirstLoop = false;
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
		
		// example randomised parameter
		// oscil frequency (kcps) 
		//std::uniform_real_distribution<float> distFreq(50.0f, 200.0f);
		//std::default_random_engine genFreq(rd());
		//float valFreq = distFreq(genFreq);
		//*m_vSendVals[1] = (MYFLT)valFreq;

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
		//example shader values provide input to neural network
		for(int i = 0; i < pboInfo.pboSize; i+=pboInfo.pboSize * 0.01)
		{
			inputData.push_back((double)pboInfo.pboPtr[i]); //13
		}

		//neural network outputs to audio parameter
		for(int i = 0; i < params.size(); i++)
		{
			outputData.push_back((double)*m_vSendVals[params[i].sendVecPosition]); //0
		}

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
		
		//if(modelOut[0] > 200.0f) modelOut[0] = 200.0f;
		//if(modelOut[0] < 50.0f) modelOut[0] = 50.0f;
		//*m_vSendVals[1] = (MYFLT)modelOut[0];

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
		
		//if(modelOut[0] > 200.0f) modelOut[0] = 200.0f;
		//if(modelOut[0] < 50.0f) modelOut[0] = 50.0f;
		//*m_vSendVals[1] = (MYFLT)modelOut[0];

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
// Clean up
//*********************************************************************************************
void Studio::Exit(){

	//delete StudioTools pointer
	m_pStTools->Exit();
	delete m_pStTools;

	//close GL context and any other GL resources
	glfwTerminate();
}
