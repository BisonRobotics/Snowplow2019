#ifndef __JJC__XBOXCONTROLLERINTERFACE__H__
#define __JJC__XBOXCONTROLLERINTERFACE__H__

#include <vector>
#include <SDL/SDL.h>

enum xBox_STICK {
    xBox_LEFT, xBox_RIGHT
};

enum xBox_BUTTON {
    xBox_A, xBox_B, xBox_X, xBox_Y,
    xBox_RB, xBox_LB,
    xBox_R3, xBox_L3,
    xBox_SELECT, xBox_START
};

class XboxController {
public:
    // empty constructor
    XboxController(void);

    // cycle events on the xbox controller
    void update(void);

    int32_t getJoyX(xBox_STICK s);
    int32_t getJoyY(xBox_STICK s);
    int32_t getTrigger(xBox_STICK s);
    bool buttonPressed(xBox_BUTTON btn);

private:
    // left joystick x/y values
    int32_t leftJoystickX = 0;
    int32_t leftJoystickY = 0;

    // right joystick x/y values
    int32_t rightJoystickX = 0;
    int32_t rightJoystickY = 0;

    // triggers
    int32_t rightTrigger = -32768;
    int32_t leftTrigger  = -32768;

    // buttons
    bool btn_A = false; // clicked?
    bool btn_B = false;
    bool btn_X = false;
    bool btn_Y = false;
    bool btn_RB = false;
    bool btn_LB = false;
    bool btn_R3 = false;
    bool btn_L3 = false;
    bool btn_SELECT = false;
    bool btn_START  = false;

    std::vector<SDL_Joystick*> joystick_ptr_vec;
};

// XboxControllerException

#endif // __JJC__XBOXCONTROLLERINTERFACE__H__
