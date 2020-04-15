#include "VR_Helper.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

//-----------------------------------------------------------------------
std::string VR_Helper::GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError){

	uint32_t unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if( unRequiredBufferLen == 0 ) return "";
	char *pchBuffer = new char[ unRequiredBufferLen ];
	unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	std::string sResult = pchBuffer;
	delete [] pchBuffer;
	return sResult;	
}

//---------------------------------------------------------------------------------------------------------------------
// Purpose: Returns true if the action is active and had a rising edge
//---------------------------------------------------------------------------------------------------------------------
bool VR_Helper::GetDigitalActionRisingEdge(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath)
{
	vr::InputDigitalActionData_t actionData;
	vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle );
	if (pDevicePath)
	{
		*pDevicePath = vr::k_ulInvalidInputValueHandle;
		if (actionData.bActive)
		{
			vr::InputOriginInfo_t originInfo;
			if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(actionData.activeOrigin, &originInfo, sizeof(originInfo)))
			{
				*pDevicePath = originInfo.devicePath;
			}
		}
	}
	return actionData.bActive && actionData.bChanged && actionData.bState;
}

//---------------------------------------------------------------------------------------------------------------------
// Purpose: Returns true if the action is active and its state is true
//---------------------------------------------------------------------------------------------------------------------
bool VR_Helper::GetDigitalActionState(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath)
{
	vr::InputDigitalActionData_t actionData;
	vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle );
	if (pDevicePath)
	{
		*pDevicePath = vr::k_ulInvalidInputValueHandle;
		if (actionData.bActive)
		{
			vr::InputOriginInfo_t originInfo;
			if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(actionData.activeOrigin, &originInfo, sizeof(originInfo)))
			{
				*pDevicePath = originInfo.devicePath;
			}
		}
	}
	return actionData.bActive && actionData.bState;
}

void VR_Helper::ThreadSleep(unsigned long nMilliseconds)
{
#if defined(_WIN32)
	::Sleep( nMilliseconds );
#elif defined(POSIX)
	usleep( nMilliseconds * 1000 );
#endif
}
