/*
    Implementation for XboxControllerInterface library
*/

#include <iostream>
#include <SDL/SDL.h>
#include <XboxControllerInterface.h>

XboxController::XboxController(void) {
    // Joystick doesnt work w/o video...stupid
    int res = SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO);

    if(res < 0) {
        std::cerr << "Couldn't initialize SDL: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    int joysticks = SDL_NumJoysticks();
    SDL_JoystickEventState(SDL_ENABLE);

    std::cout << "Joysticks found: " << joysticks << std::endl;
    for(int i = 0; i < joysticks; i++) {
        std::cout << "    " << SDL_JoystickName(i) << std::endl;
        this->joystick_ptr_vec.push_back(SDL_JoystickOpen(i));
    }

	if(joysticks == 0) {
		std::cerr << "No joysticks attached...exiting" << std::endl;
		exit(EXIT_FAILURE);	
	}
}

void XboxController::update(void) {
    SDL_Event event;

    while(SDL_PollEvent(&event)) {
        //std::cout << "Event found..." << std::endl;
        switch(event.type) {
            case SDL_JOYAXISMOTION:
                //std::cout << "SDL_JOYAXISMOTION\n";
                switch(event.jaxis.axis) {
                    case 0:
                        this->leftJoystickX = event.jaxis.value; break;
                    case 1:
                        this->leftJoystickY = event.jaxis.value; break;
                    case 2:
                        this->leftTrigger = event.jaxis.value; break;
                    case 3:
                        this->rightJoystickX = event.jaxis.value; break;
                    case 4:
                        this->rightJoystickY = event.jaxis.value; break;
                    case 5:
                        this->rightTrigger = event.jaxis.value; break;
                    default:
                        break;
                }
                break;
            case SDL_JOYBUTTONDOWN:
                //std::cout << "SDL_JOYBUTTONDOWN\n";
                switch(event.jbutton.button) {
                    case 0:
                        this->btn_A = true; break;
                    case 1:
                        this->btn_B = true; break;
                    case 2:
                        this->btn_X = true; break;
                    case 3:
                        this->btn_Y = true; break;
                    case 4:
                        this->btn_LB = true; break;
                    case 5:
                        this->btn_RB = true; break;
                    case 6:
                        this->btn_SELECT = true; break;
                    case 7:
                        this->btn_START = true; break;
                    case 9:
                        this->btn_L3 = true; break;
                    case 10:
                        this->btn_R3 = true; break;
                    default:
                        break;
                }
                break;
            case SDL_JOYBUTTONUP:
                //std::cout << "SDL_JOYBUTTONUP\n";
                switch(event.jbutton.button) {
                    case 0:
                        this->btn_A = false; break;
                    case 1:
                        this->btn_B = false; break;
                    case 2:
                        this->btn_X = false; break;
                    case 3:
                        this->btn_Y = false; break;
                    case 4:
                        this->btn_LB = false; break;
                    case 5:
                        this->btn_RB = false; break;
                    case 6:
                        this->btn_SELECT = false; break;
                    case 7:
                        this->btn_START = false; break;
                    case 9:
                        this->btn_L3 = false; break;
                    case 10:
                        this->btn_R3 = false; break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

int32_t XboxController::getJoyX(xBox_STICK s) {
    switch(s) {
        case xBox_LEFT:
            return leftJoystickX;
        case xBox_RIGHT:
            return rightJoystickX;
        default:
            throw std::invalid_argument("Argument value not recognized. Value: " + std::to_string((int)s));
    }
}

int32_t XboxController::getJoyY(xBox_STICK s) {
    switch(s) {
        case xBox_LEFT:
            return leftJoystickY;
        case xBox_RIGHT:
            return rightJoystickY;
        default:
            throw std::invalid_argument("Argument value not recognized. Value: " + std::to_string((int)s));
    }
}

int32_t XboxController::getTrigger(xBox_STICK s) {
    switch(s) {
        case xBox_LEFT:
            return leftTrigger;
        case xBox_RIGHT:
            return rightTrigger;
        default:
            throw std::invalid_argument("Argument value not recognized. Value: " + std::to_string((int)s));
    }
}

bool XboxController::buttonPressed(xBox_BUTTON btn) {
    switch(btn) {
        case xBox_A:
            return this->btn_A; break;
        case xBox_B:
            return this->btn_B; break;
        case xBox_X:
            return this->btn_X; break;
        case xBox_Y:
            return this->btn_Y; break;
        case xBox_RB:
            return this->btn_RB; break;
        case xBox_LB:
            return this->btn_LB; break;
        case xBox_SELECT:
            return this->btn_SELECT; break;
        case xBox_START:
            return this->btn_START; break;
        case xBox_R3:
            return this->btn_R3; break;
        case xBox_L3:
            return this->btn_L3; break;
        default:
            throw std::invalid_argument("Argument value not recognized. Value: " + std::to_string((int)btn));
    }
}
