#include <iostream>
#include <string>

// CPJL interface layer and
// message type
#include <CPJL.hpp>
#include <cpp/XboxData.h>
#include <cpp/Encoder.h>
#include <cpp/MotorControlCommand.h>
#include <cpp/PathVector.h>
#include <cpp/ImuData.h>

#include <misc.h>
#include <unistd.h>

#define ENCODER_TICKS_PER_ROTATION      (2048.0)
#define DEADZONE                        (1)
#define WHEEL_CIRCUMFERENCE             (1.39) // meters
#define WHEEL_CIRCUMFERENCE             (1.39) // meters
#define AUTO_TASK_DELAY                 (1000 * 1000) // us
#define MAX_ROTATION_SPEED              (30.0) // RPM
#define MAX_TRAVEL_SPEED                (50.0) // RPM
#define ROTATION_SLOWDOWN_ANGLE         (30)   // deg
#define TRAVEL_SLOWDOWN_DISTANCE        (1.0)  // meters
#define abs(x)                          ((x < 0) ? -x : x)


enum wheels_e
{
    LEFT_WHEEL = 0,
    RIGHT_WHEEL,
    NUM_OF_WHEELS
};

enum state_e
{
    TELEOP,
    AUTO,
    TEST
};


using namespace std;

XboxData* xbox_data_rx = NULL;
MotorControlCommand* motor_control_command = NULL;
ImuData* imu_data_rx = NULL;
PathVector* AutoVector = NULL;
PathVector* pathStatus = NULL;
Encoder* encoder = NULL;

uint64_t last_timestamp = -1;
float left_setpoint = 0, right_setpoint = 0;
float distanceTraveled_meters = 0;
float current_x_acc = 0;
float current_y_acc = 0;
float current_x_vel = 0;
float current_y_vel = 0;
float current_z_vel = 0;
float current_z_orient = 0;
float last_z_orient = 0;
float requested_z_orient = 0;
float distanceTraveled_x_meters = 0;
float distanceTraveled_y_meters = 0;
float requested_distance_traveled = 0;
float encoder_distance = 0;

enum state_e CurrentState;
mutex requested_data_mtx, encoder_data_mtx, imu_data_mtx, setpoint_mutex;

float rotation_crossing(float angle)
{
    float returnVal = angle;
    if(angle < (-180.0))
    {
        returnVal += 360;
    }
    else if(angle > 180.0)
    {
        returnVal -= 360;
    }
    return returnVal;
}

uint64_t get_us_timestamp(void)
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return 1000000 * tv.tv_sec + tv.tv_usec;
}

int clamp(int value, const int min, const int max) {
    int returnVal = value;
    if(value > max)
        returnVal = max;
    else if(value < min)
        returnVal = min;
    return returnVal;
}

double encoderDataToRPM(double encoderTicks, double timeDelta)
{
    double rotations = encoderTicks / ENCODER_TICKS_PER_ROTATION; // number of rotations since last poll
    double rpm = rotations / timeDelta;
    return rpm;
}

void xbox_callback(void)
{
    if(CurrentState == TEST || CurrentState == TELEOP)
    {
        //receive data in xbox
        int left_motor
            = xbox_data_rx->y_joystick_left;
        int right_motor
            = xbox_data_rx->y_joystick_right;

        left_motor = (int)mapFloat(left_motor, -32768, 32767, 450, -450);
        left_motor = (abs(left_motor) < 50)? 0 : left_motor;
        right_motor = (int)mapFloat(right_motor, -32768, 32767, 450, -450);
        right_motor = (abs(right_motor) < 50)? 0 : right_motor;

        if(xbox_data_rx->button_b)
        {
            for(int i : {0, 1, 2})
            {
                motor_control_command->left = 0;
                motor_control_command->right = 0;
                motor_control_command->timestamp = get_us_timestamp();
                motor_control_command->putMessage();
            }
            exit(EXIT_SUCCESS);
        }

        //package xbox data into motor controller command
        motor_control_command->left = left_motor;
        motor_control_command->right = right_motor;
        motor_control_command->timestamp = get_us_timestamp();
        motor_control_command->putMessage();

        //cout << "Left: " << left_motor << ", Right: " << right_motor << endl;
    }
}

void imu_calback(void)
{
    static uint64_t last_timestamp = get_us_timestamp();
    static float last_z_orient = current_z_orient;
    uint64_t current_time = imu_data_rx->timestamp;

    // Calculate Z velocity
    float temp_current_z_vel = ((imu_data_rx->z_orient - last_z_orient) / (current_time - last_timestamp)) / 1000000; // Deg/microsecond to deg/sec

    //receive data from IMU
    imu_data_mtx.lock();
    current_x_acc = imu_data_rx->x_acc;
    current_y_acc = imu_data_rx->y_acc;
    current_x_vel = imu_data_rx->x_vel;
    current_y_vel = imu_data_rx->y_vel;
    current_z_orient = imu_data_rx->z_orient;
    current_z_vel = temp_current_z_vel;
    imu_data_mtx.unlock();

    last_timestamp = current_time;
    last_z_orient = imu_data_rx->z_orient;
}

void encoder_callback(void){

    double rotations = encoder->left / ENCODER_TICKS_PER_ROTATION;
    rotations += encoder->right / ENCODER_TICKS_PER_ROTATION;
    rotations /=2; // get the average rotations of the encoders
    distanceTraveled_meters = rotations * WHEEL_CIRCUMFERENCE;
    
    encoder_data_mtx.lock();
    encoder_distance += rotations * WHEEL_CIRCUMFERENCE;
    encoder_data_mtx.unlock();

    // Speed control, only done durring auto
    if(CurrentState == AUTO)
    {
        static uint64_t last_encoder_timestamp;
        double dt = encoder->timestamp - last_encoder_timestamp;

        dt /= (60.0 * 1000000.0); // microseconds / minute

        double wheel_rpm[NUM_OF_WHEELS];
        double setpoint[NUM_OF_WHEELS];
        static double prev_cmd[NUM_OF_WHEELS];
        double cmd[NUM_OF_WHEELS];

        // Get wheel RPM
        wheel_rpm[LEFT_WHEEL]  = encoderDataToRPM(encoder->left, dt);
        wheel_rpm[RIGHT_WHEEL] = encoderDataToRPM(encoder->right, dt);

        // Make local copy of setpoints
        setpoint_mutex.lock();
        setpoint[LEFT_WHEEL] = left_setpoint;
        setpoint[RIGHT_WHEEL] = right_setpoint;
        setpoint_mutex.unlock();

        #ifdef PID
            double static error_integral[NUM_OF_WHEELS] = {0,0};
            double p[NUM_OF_WHEELS] = {0,0};
            double i[NUM_OF_WHEELS] = {0,0};
            
            //Find error
            error[LEFT_WHEEL] += setpoint[LEFT_WHEEL] - wheel_rpm[LEFT_WHEEL];
            error[RIGHT_WHEEL] += setpoint[RIGHT_WHEEL] - wheel_rpm[RIGHT_WHEEL];

            //PID Calculation
            cmd[LEFT_WHEEL] = (setpoint[LEFT_WHEEL] * p[LEFT_WHEEL]) + (error[LEFT_WHEEL] * i[LEFT_WHEEL]);
            cmd[RIGHT_WHEEL] = (setpoint[RIGHT_WHEEL]* p[RIGHT_WHEEL] + (error[RIGHT_WHEEL] * i[RIGHT_WHEEL]));

        #else
            for(int i=0; i<NUM_OF_WHEELS; i++)
            {
                if(setpoint[i] > (double)DEADZONE || setpoint[i] < (double)-DEADZONE)
                {
                    if(wheel_rpm[i] < setpoint[i])
                    {
                        double error = setpoint[i] - wheel_rpm[i];
                        cmd[i] = prev_cmd[i] + clamp(((error * error /10) + 1), 0, 50);
                    }
                    else
                    {
                        double error = wheel_rpm[i] - setpoint[i];
                        cmd[i] = prev_cmd[i] - clamp(((error * error /10) + 1), 0, 50);
                    }
                }
                else
                {
                    cmd[i] = 0;
                }
                cmd[i] = clamp(cmd[i], -1000, 1000);
            }
        #endif

    #ifndef NDEBUG
        cout << "Measured: L " << wheel_rpm[LEFT_WHEEL] << ", R " << wheel_rpm[RIGHT_WHEEL] << endl;
        cout << "Setpoint: L " << setpoint[LEFT_WHEEL] << ", R " << setpoint[RIGHT_WHEEL] << endl;
        cout << "Output:   L " << cmd[LEFT_WHEEL] << ", R " << cmd[RIGHT_WHEEL] << endl << endl;
    #endif //NDEBUG

        motor_control_command->left = cmd[LEFT_WHEEL];
        motor_control_command->right = cmd[RIGHT_WHEEL];
        motor_control_command->timestamp = get_us_timestamp();
        motor_control_command->putMessage();

        for(int i=0 ; i<NUM_OF_WHEELS ; i++)
        {
            prev_cmd[i] = cmd[i];
        }

        last_encoder_timestamp = (uint64_t)encoder->timestamp;
    }

}

/**
 * The Auto Callback converts current location and pathvector to motorspeeds
 */
void auto_callback(void){

    //get target info
    requested_data_mtx.lock();
    requested_distance_traveled = AutoVector->mag;
    requested_z_orient = AutoVector->dir;
    requested_data_mtx.unlock();

    cout<< "new vector recieved: m:" << AutoVector->mag << " d: " << AutoVector->dir << endl;
    imu_data_mtx.lock();
    last_z_orient = current_z_orient;
    imu_data_mtx.unlock();
}

void auto_task_100ms(void)
{
    float left_wheel_cmd = 0, right_wheel_cmd = 0;

    imu_data_mtx.lock();
    float local_current_x_acc = current_x_acc;
    float local_current_y_acc = current_y_acc;
    float local_current_x_vel = current_x_vel;
    float local_current_y_vel = current_y_vel;
    float local_current_z_vel = current_z_vel;
    float local_current_z_orient = current_z_orient;
    imu_data_mtx.unlock();

    requested_data_mtx.lock();
    float local_requested_z_orient = requested_z_orient;
    float local_requested_distance_traveled = requested_distance_traveled;
    requested_data_mtx.unlock();

    encoder_data_mtx.lock();
    float local_encoder_distance = encoder_distance;
    encoder_data_mtx.unlock();

    // Take care of compiler warnings
    (void)local_current_x_acc;
    (void)local_current_y_acc;
    (void)local_current_x_vel;
    (void)local_current_y_vel;
    (void)local_current_z_vel;

    float rotation_command = rotation_crossing(local_requested_z_orient - rotation_crossing(local_current_z_orient - last_z_orient));

    // Rotation calulations
    float rotation_wheel_command = clamp((rotation_command / ROTATION_SLOWDOWN_ANGLE) * MAX_ROTATION_SPEED, -MAX_ROTATION_SPEED, MAX_ROTATION_SPEED);
    left_wheel_cmd += rotation_wheel_command;
    right_wheel_cmd -= rotation_wheel_command;

    // Distance calculations
    float travel_command = local_requested_distance_traveled - local_encoder_distance;
    // Can be improved
    if(abs(rotation_command) < 3.0)
    {
        float travel_wheel_command = clamp(
            ((travel_command / (TRAVEL_SLOWDOWN_DISTANCE)) * MAX_TRAVEL_SPEED),
            -MAX_TRAVEL_SPEED,
            MAX_TRAVEL_SPEED
        );

        left_wheel_cmd += travel_wheel_command;
        right_wheel_cmd += travel_wheel_command;
    }

    if((abs(rotation_command) < 3) && (abs(travel_command) < 0.3))
    {
        pathStatus->dir = rotation_command;
        pathStatus->mag = travel_command;
        pathStatus->status = "ReachedTarget";
        pathStatus->putMessage();
        cout << "completed vector: m: "<< pathStatus->mag << " d: " << pathStatus->dir << " " << pathStatus->status<< endl;
        cout<< "requesting New Vector" << endl;

    }else{

        cout << "left: " << left_wheel_cmd << " right: " << right_wheel_cmd << endl;
        setpoint_mutex.lock();
        left_setpoint = left_wheel_cmd;
        left_setpoint = right_wheel_cmd;;
        setpoint_mutex.unlock(); 
    }
}

int main(int argc, char* argv[])
{
    uint64_t next_run_time = -1;

    if(argc != 2) {
        cout << "Usage:\n  " << argv[0] << " <mode>\n";
        exit(EXIT_FAILURE);
    }

    //current mode as the second argument
    string currentMode = argv[1];

    //State machine for the modes' callbacks
    //ignores unwanted data
    if(currentMode == "TELEOP" )
    {
        CurrentState = TELEOP;
        xbox_data_rx = new XboxData(
                    new CPJL("localhost", 14000),
                    "xbox_data",
                    xbox_callback);

    }else if (currentMode == "Auto")
    {
        CurrentState = AUTO;
        AutoVector = new PathVector(
                    new CPJL("localhost", 14000),
                    "path_vector",
                    auto_callback);

        imu_data_rx = new ImuData(
                    new CPJL("localhost", 14000),
                    "imuData",
                    imu_calback);

        encoder = new Encoder(
                    new CPJL("localhost", 14000),
                    "encoder_data",
                    encoder_callback);
        //the Response message node
        pathStatus = new PathVector( new CPJL( "localhost", 14000),
                                    "path_status");
    }else if(currentMode == "TEST")
    {
        AutoVector = new PathVector(
                    new CPJL("localhost", 14000),
                    "path_vector",
                    auto_callback);

        imu_data_rx = new ImuData(
                    new CPJL("localhost", 14000),
                    "imuData",
                    imu_calback);

        encoder = new Encoder(
                    new CPJL("localhost", 14000),
                    "encoder_data",
                    encoder_callback);
        xbox_data_rx = new XboxData(
                    new CPJL("localhost", 14000),
                    "xbox_data",
                    xbox_callback);
    }
    else
    {
        cout << "Incorrect argument:\n" << argv[1] << " <'TELEOP', 'Auto'>\n";
        exit(EXIT_FAILURE);
    }

    motor_control_command = new MotorControlCommand(
        new CPJL("localhost", 14000),
        "motor_control_data"
    );

    // start the asynch loop
    auto loop = CPJL_Message::loop();
    (void)loop;

    // Run 1ms task every 1ms
    next_run_time = get_us_timestamp();
    while(true)
    {
        uint64_t curTime = get_us_timestamp();
        if(curTime < next_run_time)
        {
            // Sleep until next run time
            usleep(next_run_time - curTime);
        }
        next_run_time += AUTO_TASK_DELAY;
        if(CurrentState == AUTO)
        {
            auto_task_100ms();
        }
    }
    return 0;
}
