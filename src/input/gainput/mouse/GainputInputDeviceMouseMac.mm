
#include "input/gainput/gainput.h"

#ifdef GAINPUT_PLATFORM_MAC
#include "input/gainput/GainputMac.h"
#include "input/gainput/mouse/GainputInputDeviceMouseMac.h"

#include "input/gainput/GainputInputDeltaState.h"
#include "input/gainput/GainputHelpers.h"
#include "input/gainput/GainputLog.h"

#import <Carbon/Carbon.h>

namespace gainput
{
	
	InputDeviceMouseImplMac::InputDeviceMouseImplMac(InputManager& manager, InputDevice& device, InputState& state, InputState& previousState ) :
	manager_(manager),
	device_(device),
	state_(&state),
	previousState_(&previousState),
	nextState_(manager.GetAllocator(), MouseButtonCount + MouseAxisCount),
	delta_(0)
	{
		
		deviceState_ = InputDevice::DeviceState::DS_OK;
	
	}
	
	InputDeviceMouseImplMac::~InputDeviceMouseImplMac()
	{
	}
	
	void InputDeviceMouseImplMac::HandleMouseMove(float x, float y)
	{
		HandleAxis(device_, nextState_, delta_, gainput::MouseAxisX, x);
		HandleAxis(device_, nextState_, delta_, gainput::MouseAxisY, y);
	}
	
	void InputDeviceMouseImplMac::HandleMouseWheel(int deltaY)
	{
		if(deltaY < 0)
		{
			HandleButton(device_, nextState_, delta_, gainput::MouseButtonWheelDown, true);
			
		}
		else
		{
			HandleButton(device_, nextState_, delta_, gainput::MouseButtonWheelUp, true);
		}
	}

	
	void InputDeviceMouseImplMac::HandleMouseButton(bool isPressed, gainput::DeviceButtonId mouseButtonId )
	{
		if(mouseButtonId < gainput::MouseButton0 || mouseButtonId >= gainput::MouseButtonMax)
			return;
		HandleButton(device_, nextState_, delta_, mouseButtonId, isPressed);
		
	}
	
	void InputDeviceMouseImplMac::Update(InputDeltaState* delta)
	{
		delta_ = delta;
		
		// Reset mouse wheel buttons
		HandleButton(device_, nextState_, delta_, MouseButtonWheelUp, false);
		HandleButton(device_, nextState_, delta_, MouseButtonWheelDown, false);
		
		*state_ = nextState_;
	}
	
}
#endif
