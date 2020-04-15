#include <iostream>
#include <cstring>

#include "VR_Manager.hpp"

#include <GL/glew.h>

#ifdef __APPLE__ 
#include "GLFW/glfw3.h"

#define _stricmp strcasecmp
#include <strings.h>

#include "openvr/openvr_capi.h"
#include "openvr/openvr.h"

#elif _WIN32 
#include "glfw3.h"
#include "pathtools.h"
#include "openvr.h"
#endif

//-----------------------------------------
// Constructor
//------------------------------------------
VR_Manager::VR_Manager(std::unique_ptr<ExecutionFlags>& flagPtr) : 
	m_pHMD(NULL),
	m_iValidPoseCount(0),
	m_strPoseClasses(""),
	m_bViveRandParams(false),
	m_bViveRecordTrainingExample(false),
	m_bViveTrainModel(false),
	m_bViveRunModel(false),
	m_bViveSaveModel(false),
	m_bViveLoadModel(false),
	m_bCurrentDeviceState(false){

	helper = std::make_unique<VR_Helper>();

	m_bDebugPrint = flagPtr->flagDPrint; 
}

//-------------------------------------
bool VR_Manager::BInit(){

	// Load SteamVR runtime
	vr::EVRInitError eError = vr::VRInitError_None;

#ifdef _WIN32
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
#else
	m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Utility);
#endif

	if(eError != vr::VRInitError_None)
	{
		m_pHMD = NULL;
		return false;
	}
	
	m_strDriver = "No Driver";
	m_strDisplay = "No Display";

	m_strDriver = helper->GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
	m_strDisplay = helper->GetTrackedDeviceString(vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);

	std::cout << "Device name: " << m_strDriver << "\nDevice number: " << m_strDisplay << std::endl;

	//if (!BInitCompositor())
	//{
	//	std::cout << __FUNCTION__ << " - Failed to initialize VR Compositor!\n" << std::endl;
	//	return false;
	//}

//*********** removing pathtools from osx build *************//
#ifdef __APPLE__
	std::string manifestPath = "/Users/bryandunphy/Projects/5CellAVR/src/VR/avr_iml_actions.json";
	vr::VRInput()->SetActionManifestPath(manifestPath.c_str()); 
#elif _WIN32
	vr::VRInput()->SetActionManifestPath( Path_MakeAbsolute( "avr_iml_actions.json", Path_StripFilename( Path_GetExecutablePath() ) ).c_str() );
#endif
	
	//generic actions
	vr::VRInput()->GetActionHandle( "/actions/avr/in/HideThisController", &m_actionHideThisController);
	vr::VRInput()->GetActionHandle( "/actions/avr/in/TriggerHaptic", &m_actionTriggerHaptic );
	vr::VRInput()->GetActionHandle( "/actions/avr/in/AnalogInput", &m_actionAnalongInput );

	vr::VRInput()->GetActionSetHandle( "/actions/avr", &m_actionSetAvr );

	vr::VRInput()->GetActionHandle( "/actions/avr/out/Haptic_Left", &m_rHand[Left].m_actionHaptic );
	vr::VRInput()->GetInputSourceHandle( "/user/hand/left", &m_rHand[Left].m_source );
	vr::VRInput()->GetActionHandle( "/actions/avr/in/Hand_Left", &m_rHand[Left].m_actionPose );

	vr::VRInput()->GetActionHandle( "/actions/avr/out/Haptic_Right", &m_rHand[Right].m_actionHaptic );
	vr::VRInput()->GetInputSourceHandle( "/user/hand/right", &m_rHand[Right].m_source );
	vr::VRInput()->GetActionHandle( "/actions/avr/in/Hand_Right", &m_rHand[Right].m_actionPose );

	//machine learning actions
	vr::VRInput()->GetActionHandle("/actions/machineLearning/in/RandomiseParameters", &m_actionRandomParameters);
	vr::VRInput()->GetActionHandle("/actions/machineLearning/in/RecordTrainingExample", &m_actionRecordTrainingExample);
	vr::VRInput()->GetActionHandle("/actions/machineLearning/in/TrainModel", &m_actionTrainModel);
	vr::VRInput()->GetActionHandle("/actions/machineLearning/in/RunModel", &m_actionRunModel);
	vr::VRInput()->GetActionHandle("/actions/machinelearning/in/Savemodel", &m_actionSaveModel);
	vr::VRInput()->GetActionHandle("/actions/machineLearning/in/LoadModel", &m_actionLoadModel);
	vr::VRInput()->GetActionHandle("/actions/machineLearning/in/MovementControls", &m_actionMoveCam);

	vr::VRInput()->GetActionSetHandle("/actions/machineLearning", &m_actionSetMachineLearning);

	m_fNearClip = 0.1f;
	m_fFarClip = 10000.0f;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Initialize Compositor. Returns true if the compositor was
//          successfully initialized, false otherwise.
//-----------------------------------------------------------------------------
bool VR_Manager::BInitCompositor()
{
	vr::EVRInitError peError = vr::VRInitError_None;

	if ( !vr::VRCompositor() )
	{
		std::cout << "Compositor initialization failed.\n" << std::endl;
		return false;
	}

	return true;
}



//--------------------------------------
void VR_Manager::ExitVR(){

	if(m_pHMD)
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

	for( std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
	{
		delete (*i);
	}
	m_vecRenderModels.clear();

}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool VR_Manager::HandleInput()
{
	bool bRet = false;

	// Process SteamVR events
	vr::VREvent_t event;
	while(m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		ProcessVREvent(event);
	}

	// Process SteamVR action state
	// UpdateActionState is called each frame to update the state of the actions themselves. The application
	// controls which action sets are active with the provided array of VRActiveActionSet_t structs.
	vr::VRActiveActionSet_t actionSet = {0};
	actionSet.ulActionSet = m_actionSetMachineLearning;
	vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);

	//machine learning controls
	vr::VRInputValueHandle_t ulRandomDevice;
	if(helper->GetDigitalActionState(m_actionRandomParameters, &ulRandomDevice)){
		if(ulRandomDevice == m_rHand[Left].m_source || ulRandomDevice == m_rHand[Right].m_source){
			m_bViveRandParams = true;
		}
	} else {	
		m_bViveRandParams = false;
	}

	vr::VRInputValueHandle_t ulRecordDevice;
	if(helper->GetDigitalActionState(m_actionRecordTrainingExample, &ulRecordDevice)){
		if(ulRecordDevice == m_rHand[Left].m_source || ulRecordDevice == m_rHand[Right].m_source){
			m_bViveRecordTrainingExample = true;
		}
	} else {
		m_bViveRecordTrainingExample = false;
	}

	vr::VRInputValueHandle_t ulTrainDevice;
	if(helper->GetDigitalActionState(m_actionTrainModel, &ulTrainDevice)){
		if(ulTrainDevice == m_rHand[Left].m_source || ulTrainDevice == m_rHand[Right].m_source){
			m_bViveTrainModel = true;
		}
	} else {
		m_bViveTrainModel = false;
	}

	//TODO I have button logic here because it is switching based on the opposite value of 
	//m_bViveRunModel. It would be better to keep the button logic in FiveCell.cpp with the other 
	//controls and just have a true/false switch here.
	vr::VRInputValueHandle_t ulRunDevice;
	bool deviceCall = helper->GetDigitalActionState(m_actionRunModel, &ulRunDevice);
	bool prevDeviceState = m_bCurrentDeviceState;
	if(deviceCall && deviceCall != prevDeviceState){
		if(ulRunDevice == m_rHand[Left].m_source || ulRunDevice == m_rHand[Right].m_source){
			m_bViveRunModel = !m_bViveRunModel;
		}
	}
	m_bCurrentDeviceState = deviceCall;

	vr::VRInputValueHandle_t ulSaveDevice;
	if(helper->GetDigitalActionState(m_actionSaveModel, &ulSaveDevice)){
		if(ulSaveDevice == m_rHand[Left].m_source || ulSaveDevice == m_rHand[Right].m_source){
			m_bViveSaveModel = true;
		}
	} else {
		m_bViveSaveModel = false;
	}

	vr::VRInputValueHandle_t ulLoadDevice;
	if(helper->GetDigitalActionState(m_actionLoadModel, &ulLoadDevice)){
		if(ulLoadDevice == m_rHand[Left].m_source || ulLoadDevice == m_rHand[Right].m_source){
			m_bViveLoadModel = true;
		}
	} else {
		m_bViveLoadModel = false;
	}
	
	vr::InputAnalogActionData_t ulMoveCam;
	if ( vr::VRInput()->GetAnalogActionData( m_actionMoveCam, &ulMoveCam, sizeof( ulMoveCam ), vr::k_ulInvalidInputValueHandle ) == vr::VRInputError_None && ulMoveCam.bActive )
	{
		m_vMoveCam[0] = ulMoveCam.x;
		m_vMoveCam[1] = ulMoveCam.y;
	}

	//generic controls
	vr::VRActiveActionSet_t avrActionSet = {0};
	avrActionSet.ulActionSet = m_actionSetAvr;
	vr::VRInput()->UpdateActionState(&avrActionSet, sizeof(avrActionSet), 1);

	vr::VRInputValueHandle_t ulHapticDevice;
	if (helper->GetDigitalActionRisingEdge(m_actionTriggerHaptic, &ulHapticDevice))
	{
		if (ulHapticDevice == m_rHand[Left].m_source)
		{
			vr::VRInput()->TriggerHapticVibrationAction(m_rHand[Left].m_actionHaptic, 0, 1, 4.f, 1.0f, vr::k_ulInvalidInputValueHandle);
		}
		if (ulHapticDevice == m_rHand[Right].m_source)
		{
			vr::VRInput()->TriggerHapticVibrationAction(m_rHand[Right].m_actionHaptic, 0, 1, 4.f, 1.0f, vr::k_ulInvalidInputValueHandle);
		}
	}

	vr::InputAnalogActionData_t analogData;
	if ( vr::VRInput()->GetAnalogActionData( m_actionAnalongInput, &analogData, sizeof( analogData ), vr::k_ulInvalidInputValueHandle ) == vr::VRInputError_None && analogData.bActive )
	{
		m_vAnalogValue[0] = analogData.x;
		m_vAnalogValue[1] = analogData.y;
	}

	m_rHand[Left].m_bShowController = true;
	m_rHand[Right].m_bShowController = true;

	vr::VRInputValueHandle_t ulHideDevice;
	if (helper->GetDigitalActionState(m_actionHideThisController, &ulHideDevice))
	{
		if (ulHideDevice == m_rHand[Left].m_source)
		{
			m_rHand[Left].m_bShowController = false;
		}
		if (ulHideDevice == m_rHand[Right].m_source)
		{
			m_rHand[Right].m_bShowController = false;
		}
	}

	for (EHand eHand = Left; eHand <= Right; ((int&)eHand)++)
	{
		vr::InputPoseActionData_t poseData;

#ifdef __APPLE__
		if ( vr::VRInput()->GetPoseActionDataRelativeToNow( m_rHand[eHand].m_actionPose, vr::TrackingUniverseStanding, 0, &poseData, sizeof( poseData ), vr::k_ulInvalidInputValueHandle ) != vr::VRInputError_None
			|| !poseData.bActive || !poseData.pose.bPoseIsValid )
		{
			m_rHand[eHand].m_bShowController = false;
		}
		else
		{
			m_rHand[eHand].m_rmat4Pose = ConvertSteamVRMatrixToGlmMat4( poseData.pose.mDeviceToAbsoluteTracking );
#elif _WIN32
		if ( vr::VRInput()->GetPoseActionData( m_rHand[eHand].m_actionPose, vr::TrackingUniverseStanding, 0, &poseData, sizeof( poseData ), vr::k_ulInvalidInputValueHandle ) != vr::VRInputError_None
			|| !poseData.bActive || !poseData.pose.bPoseIsValid )
		{
			//std::cout << "VR_Manager.cpp line 284: \n" << "poseData.bActive = " << poseData.bActive <<"\n" << "poseData.pose.bPoseIsValid = " << poseData.pose.bPoseIsValid << "\n" << std::endl;
			m_rHand[eHand].m_bShowController = false;

		}
		else
		{
			m_rHand[eHand].m_rmat4Pose = ConvertSteamVRMatrixToGlmMat4( poseData.pose.mDeviceToAbsoluteTracking );
#endif

			vr::InputOriginInfo_t originInfo;
			if ( vr::VRInput()->GetOriginTrackedDeviceInfo( poseData.activeOrigin, &originInfo, sizeof( originInfo ) ) == vr::VRInputError_None 
				&& originInfo.trackedDeviceIndex != vr::k_unTrackedDeviceIndexInvalid )
			{

				std::string sRenderModelName = helper->GetTrackedDeviceString( originInfo.trackedDeviceIndex, vr::Prop_RenderModelName_String );
				if ( sRenderModelName != m_rHand[eHand].m_sRenderModelName )
				{
					m_rHand[eHand].m_pRenderModel = FindOrLoadRenderModel(sRenderModelName.c_str());
					m_rHand[eHand].m_sRenderModelName = sRenderModelName;
				}
			}
		}
	}

	return bRet;
}

//-----------------------------------------------------------------------------
// This function gets the dimensions of the far clip plane from
// the projection matrix each frame. These values are then used to update
// the quad sent to the raymarching shader.
// ----------------------------------------------------------------------------
glm::vec4 VR_Manager::GetFarPlaneDimensions(vr::Hmd_Eye nEye){
	
	float* pfLeft = new float;
	float* pfRight = new float;
	float* pfTop = new float;
	float* pfBottom = new float;

	m_pHMD->GetProjectionRaw(nEye, pfLeft, pfRight, pfTop, pfBottom);

	float tanLeft = *pfLeft;
	float tanRight = *pfRight;
	float tanTop = *pfTop;
	float tanBottom = *pfBottom;

	delete pfLeft;
	delete pfRight;
	delete pfTop;
	delete pfBottom;

	glm::vec4 planeDimensions = glm::vec4(tanLeft, tanRight, tanTop, tanBottom);
	return planeDimensions;
}

//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void VR_Manager::ProcessVREvent(const vr::VREvent_t & event)
{
	switch(event.eventType)
	{
	case vr::VREvent_TrackedDeviceDeactivated:
		{
			std::cout << "Device " << event.trackedDeviceIndex << " detached." << std::endl;
		}
		break;
	case vr::VREvent_TrackedDeviceUpdated:
		{
			std::cout << "Device " << event.trackedDeviceIndex << " updated." << std::endl;
		}
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
glm::mat4 VR_Manager::ConvertSteamVRMatrixToGlmMat4(const vr::HmdMatrix34_t &matPose)
{
	glm::mat4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);
	return matrixObj;
}

//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel* VR_Manager::FindOrLoadRenderModel(const char* pchRenderModelName)
{
	//std::string s = pchRenderModelName;
	//std::cout << s << "*****************************" << std::endl; 
	
	CGLRenderModel* pRenderModel = NULL;
	for(std::vector<CGLRenderModel*>::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++)
	{
		if(!_stricmp((*i)->GetName().c_str(), pchRenderModelName))
		{
			pRenderModel = *i;
			break;
		}
	}

	// load the model if we didn't find one
	if(!pRenderModel)
	{
		vr::RenderModel_t* pModel;
		vr::EVRRenderModelError error;
		while (1)
		{
			error = vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel);
			if (error != vr::VRRenderModelError_Loading)
				break;

			helper->ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			if(m_bDebugPrint){

			dprintf("Unable to load render model %s - %s\n", pchRenderModelName, vr::VRRenderModels()->GetRenderModelErrorNameFromEnum(error));
			}
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t* pTexture;
		while (1)
		{
			error = vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture);
			if (error != vr::VRRenderModelError_Loading)
				break;

			helper->ThreadSleep(1);
		}

		if (error != vr::VRRenderModelError_None)
		{
			if(m_bDebugPrint){
			dprintf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName);
			}
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new CGLRenderModel(pchRenderModelName);
		if (!pRenderModel->BInit(*pModel, *pTexture))
		{
			if(m_bDebugPrint){
			dprintf("Unable to create GL model from render model %s\n", pchRenderModelName);
			}
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			m_vecRenderModels.push_back(pRenderModel);
		}
		vr::VRRenderModels()->FreeRenderModel(pModel);
		vr::VRRenderModels()->FreeTexture(pTexture);
	}

	return pRenderModel;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void VR_Manager::UpdateHMDMatrixPose()
{
	if (!m_pHMD)
		return;

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToGlmMat4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (m_rDevClassChar[nDevice]==0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
		m_mat4HMDPose = glm::inverse(m_mat4HMDPose);
	}
}

//-----------------------------------------------------------------------------
//  Gets a Current View Projection Matrix with respect to nEye,
//  which may be an Eye_Left or an Eye_Right.
//-----------------------------------------------------------------------------
glm::mat4 VR_Manager::GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye)
{
	glm::mat4 matMVP;
	if(nEye == vr::Eye_Left)
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
	}
	else if(nEye == vr::Eye_Right)
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
	}

	return matMVP;
}

//-----------------------------------------------------------------------------
// Gets the current ViewEye matrix.
// ----------------------------------------------------------------------------
glm::mat4 VR_Manager::GetCurrentViewEyeMatrix(vr::Hmd_Eye nEye)
{
	glm::mat4 matVE;
	if(nEye == vr::Eye_Left)
	{
		matVE = m_mat4eyePosLeft * m_mat4HMDPose;
	}
	else if(nEye == vr::Eye_Right)
	{
		matVE = m_mat4eyePosRight * m_mat4HMDPose;
	}

	return matVE;
}

//------------------------------------------------------------------------------
// Gets current View Matrix.
// -----------------------------------------------------------------------------
glm::mat4 VR_Manager::GetCurrentViewMatrix()
{
	glm::mat4 matV = m_mat4HMDPose;
	return matV;
}

//-----------------------------------------------------------------------------
// Gets current Eye Matrix
// ----------------------------------------------------------------------------
glm::mat4 VR_Manager::GetCurrentEyeMatrix(vr::Hmd_Eye nEye)
{
	glm::mat4 matE;
	if(nEye == vr::Eye_Left)
	{
		matE = m_mat4eyePosLeft;
	}
	else if(nEye == vr::Eye_Right)
	{
		matE = m_mat4eyePosRight;
	}

	return matE;
}

//-----------------------------------------------------------------------------
// Gets current Projection Matrix
// ----------------------------------------------------------------------------
glm::mat4 VR_Manager::GetCurrentProjectionMatrix(vr::Hmd_Eye nEye)
{
	glm::mat4 matP;
	if(nEye == vr::Eye_Left)
	{
		matP = m_mat4ProjectionLeft;
	}
	else if(nEye == vr::Eye_Right)
	{
		matP = m_mat4ProjectionRight;
	}

	return matP;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool VR_Manager::BSetupCameras()
{
	//Arbitrary vector to test whether matrices returned below are identity
	glm::vec4 testVec4 = glm::vec4(1.3f, 2.6f, 0.5f, 3.9f);

	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);	
	glm::vec4 testResult = m_mat4ProjectionLeft * testVec4;
	if(testResult == testVec4){
		std::cout << "Error: Left eye projection matrix returned identity!" << std::endl;
		return false;
	}

	m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
	testResult = m_mat4ProjectionRight * testVec4;
	if(testResult == testVec4){
		std::cout << "Error: Right eye projection matrix returned identity!" << std::endl;
		return false;
	}

	m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
	testResult = m_mat4eyePosLeft * testVec4;
	if(testResult == testVec4){
		std::cout << "Error: Left eye pose matrix returned identity!" << std::endl;
		return false;
	}

	m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
	testResult = m_mat4eyePosRight * testVec4;
	if(testResult == testVec4){
		std::cout << "Error: Right eye pose matrix returned identity!" << std::endl;
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Gets a Matrix Projection Eye with respect to nEye.
//-----------------------------------------------------------------------------
glm::mat4 VR_Manager::GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
{
	if (!m_pHMD)
		return glm::mat4(0.0);

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip);

	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], 
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], 
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

//-----------------------------------------------------------------------------
// Gets an HMDMatrixPoseEye with respect to nEye.
//-----------------------------------------------------------------------------
glm::mat4 VR_Manager::GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
{
	if ( !m_pHMD )
		return glm::mat4(0.0);

	vr::HmdMatrix34_t matEyeHead = m_pHMD->GetEyeToHeadTransform(nEye);
	glm::mat4 matrixObj = glm::mat4(
		matEyeHead.m[0][0], matEyeHead.m[1][0], matEyeHead.m[2][0], 0.0, 
		matEyeHead.m[0][1], matEyeHead.m[1][1], matEyeHead.m[2][1], 0.0,
		matEyeHead.m[0][2], matEyeHead.m[1][2], matEyeHead.m[2][2], 0.0,
		matEyeHead.m[0][3], matEyeHead.m[1][3], matEyeHead.m[2][3], 1.0f
		);

	return glm::inverse(matrixObj);
}

//bool VR_Manager::BGetRotate3DTrigger(){
//	return m_bRotate3D;
//}

float VR_Manager::GetNearClip()
{
	return m_fNearClip;
}

float VR_Manager::GetFarClip()
{
	return m_fFarClip;
}

