#ifndef VR_MANAGER_HPP
#define VR_MANAGER_HPP

#include <string>
#include <vector>
#include <memory>

#include "VR_Helper.hpp"
//#include "openvr.h"
//#include "Matrices.h"
#include "CGLRenderModel.hpp"
#include "SystemInfo.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class VR_Manager {

public:

	VR_Manager(std::unique_ptr<ExecutionFlags>& flagPtr);

	bool BInit();
	bool BInitCompositor();
	void ExitVR();
	bool HandleInput();
	void ProcessVREvent(const vr::VREvent_t &event);
	glm::mat4 ConvertSteamVRMatrixToGlmMat4(const vr::HmdMatrix34_t &matPose);
	CGLRenderModel* FindOrLoadRenderModel(const char* pchRenderModelName);

	bool BSetupCameras();
	void UpdateHMDMatrixPose();
	glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
	glm::mat4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	glm::mat4 GetCurrentViewEyeMatrix(vr::Hmd_Eye nEye);
	glm::mat4 GetCurrentViewMatrix();
	glm::mat4 GetCurrentProjectionMatrix(vr::Hmd_Eye nEye);
	glm::mat4 GetCurrentEyeMatrix(vr::Hmd_Eye nEye);

	//bool BGetRotate3DTrigger();
	float GetNearClip();
	float GetFarClip();

	glm::vec4 GetFarPlaneDimensions(vr::Hmd_Eye nEye);

	enum EHand
	{
		Left = 0,
		Right = 1,
	};

	friend class Graphics;

private:

	std::unique_ptr<VR_Helper> helper;
	vr::IVRSystem *m_pHMD;		

	std::string m_strDriver;
	std::string m_strDisplay;

	struct ControllerInfo_t
	{
#ifdef __APPLE__
		VRInputValueHandle_t m_source = vr::k_ulInvalidInputValueHandle;
		VRActionHandle_t m_actionPose = vr::k_ulInvalidActionHandle;
		VRActionHandle_t m_actionHaptic = vr::k_ulInvalidActionHandle;
#elif _WIN32
		vr::VRInputValueHandle_t m_source = vr::k_ulInvalidInputValueHandle;
		vr::VRActionHandle_t m_actionPose = vr::k_ulInvalidActionHandle;
		vr::VRActionHandle_t m_actionHaptic = vr::k_ulInvalidActionHandle;
#endif
		glm::mat4 m_rmat4Pose;
		CGLRenderModel *m_pRenderModel = nullptr;
		std::string m_sRenderModelName;
		bool m_bShowController;
	};
	ControllerInfo_t m_rHand[2];

	glm::vec2 m_vAnalogValue;

	glm::mat4 m_mat4eyePosLeft;
	glm::mat4 m_mat4eyePosRight;

	glm::mat4 m_mat4ProjectionLeft;
	glm::mat4 m_mat4ProjectionRight;

	glm::mat4 m_mat4HMDPose;

	std::vector<CGLRenderModel*> m_vecRenderModels;
	
#ifdef __APPLE__
	VRActionHandle_t m_actionHideThisController = vr::k_ulInvalidActionHandle;
	VRActionHandle_t m_actionTriggerHaptic = vr::k_ulInvalidActionHandle;
	VRActionHandle_t m_actionAnalongInput = vr::k_ulInvalidActionHandle;
	
	VRActionSetHandle_t m_actionSetAvr = vr::k_ulInvalidActionSetHandle;

	//machine learning actions
	VRActionHandle_t m_actionRandomParameters = vr::k_ulInvalidActionHandle;
	VRActionHandle_t m_actionRecordTrainingExample = vr::k_ulInvalidActionHandle;
	VRActionHandle_t m_actionTrainModel = vr::k_ulInvalidActionHandle;
	VRActionHandle_t m_actionRunModel = vr::k_ulInvalidActionHandle;
	VRActionHandle_t m_actionSaveModel = vr::k_ulInvalidActionHandle;
	VRActionHandle_t m_actionLoadModel = vr::k_ulInvalidActionHandle;
	VRActionHandle_t m_actionMoveCam = vr::k_ulInvalidActionHandle;

	VRActionSetHandle_t m_actionSetMachineLearning = vr::k_ulInvalidActionSetHandle;
#elif _WIN32
	//generic actions
	vr::VRActionHandle_t m_actionHideThisController = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionTriggerHaptic = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionAnalongInput = vr::k_ulInvalidActionHandle;

	vr::VRActionSetHandle_t m_actionSetAvr = vr::k_ulInvalidActionSetHandle;

	//machine learning actions
	vr::VRActionHandle_t m_actionRandomParameters = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionRecordTrainingExample = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionTrainModel = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionRunModel = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionSaveModel = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionLoadModel = vr::k_ulInvalidActionHandle;
	vr::VRActionHandle_t m_actionMoveCam = vr::k_ulInvalidActionHandle;

	vr::VRActionSetHandle_t m_actionSetMachineLearning = vr::k_ulInvalidActionSetHandle;
#endif

	//machine learning bools from vive controller
	bool m_bViveRandParams;
	bool m_bViveRecordTrainingExample;
	bool m_bViveTrainModel;
	bool m_bViveRunModel;
	bool m_bViveSaveModel;
	bool m_bViveLoadModel;
	bool m_bCurrentDeviceState;

	//movement controls
	glm::vec2 m_vMoveCam;

	vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
	int m_iValidPoseCount;
	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	glm::mat4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class
	float m_fNearClip;
	float m_fFarClip;
	bool m_bDebugPrint;
};
#endif
