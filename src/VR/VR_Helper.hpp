#ifndef VR_HELPER_HPP
#define VR_HELPER_HPP

#include <string>

#ifdef __APPLE__
#include "openvr/openvr_capi.h"
#include "openvr/openvr.h"
#elif _WIN32
#include "openvr.h"
#endif

class VR_Helper {

public :

	std::string GetTrackedDeviceString(vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *pError = NULL); 

#ifdef __APPLE__
	bool GetDigitalActionRisingEdge(VRActionHandle_t action, VRInputValueHandle_t *pDevicePath = nullptr);
	bool GetDigitalActionState(VRActionHandle_t action, VRInputValueHandle_t *pDevicePath = nullptr);
#elif _WIN32
	bool GetDigitalActionRisingEdge(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath = nullptr);
	bool GetDigitalActionState(vr::VRActionHandle_t action, vr::VRInputValueHandle_t *pDevicePath = nullptr);
#endif


	void ThreadSleep(unsigned long nMilliseconds);
};
#endif
