/*
 * rosserial Servo Control Example
 *
 * This sketch demonstrates the control of hobby R/C servos
 * using ROS and the arduiono
 *
 * For the full tutorial write up, visit
 * www.ros.org/wiki/rosserial_mbed_demos
 *
 */

// #include "mbed.h"
// #include "Servo.h"
// #include <ros.h>
// #include <std_msgs/UInt16.h>

// ros::NodeHandle  nh;

// #ifdef TARGET_LPC1768
// Servo servo(p21);
// #elif defined(TARGET_KL25Z) || defined(TARGET_NUCLEO_F401RE)
// Servo servo(D8);
// #else
// #error "You need to specify a pin for the Servo"
// #endif
// DigitalOut myled(LED1);

// void servo_cb( const std_msgs::UInt16& cmd_msg) {
//     servo.position(cmd_msg.data); //set servo angle, should be from 0-180
//     myled = !myled;  //toggle led
// }


// ros::Subscriber<std_msgs::UInt16> sub("servo", servo_cb);

// int main() {

//     nh.initNode();
//     nh.subscribe(sub);

//     while (1) {
//         nh.spinOnce();
//         wait_ms(1);
//     }
// }
// Hello World to sweep a servo through its full range

#include "mbed.h"
#include "Servo.h"
#include "armmessage.h"
#include "serialchannel3.h"

//DO NOT CHANGE THESE THE PROGRAM WILL NOT WORK CORRECTLY
#define PI 3.1415926
#define FOREARM_LENGTH 300
#define BICEP_LENGTH 275
/*#define FOREARM_LENGTH 339
#define BICEP_LENGTH 266
#define BUCKET_HEIGHT 146
#define BUCKET_DEPTH 76
#define BUCKET_OFFSET 19
#define SHOULDER_OFFSET 66*/

/*                          BUCKET_OFFSET
                            o   | 
            SPRING_HEIGHT - |\  |
                            | o--O Wrist
                           /--|   \
          BUCKET_HEIGHT - /   |    \
                          \   |     \ - FOREARM_LENGTH
                           \__|      \  
                            |         \
                            |          \
                      BUCKET_DEPTH      O Elbow
                                       /
                                      /
                                     /
                                    / - BICEP_LENGTH
                                   / 
                                  /
                                 O Shoulder
                                 | - SHOULDER_OFFSET
                                -O-
                                Yaw
*/

// 0.46 BOTTOM FOR SHOULDER
// 0.21 UP FOR SHOULDER
// 0.05 BACK FOR SHOULDER

// Elbow low 0.05
// Elbow high 0.3

#define SHOULDER_MIN 0.05
#define SHOULDER_MAX 0.55
#define ELBOW_MIN 0.1
#define ELBOW_MAX 0.60
#define WRIST_MIN 0
#define WRIST_MAX 0.4
#define BUCKET_MIN 0
#define BUCKET_MAX 0.56
#define YAW_MIN 0
#define YAW_MAX 1

#define INTERVAL 30

using namespace Soro;

Servo _yawServo(p23);
Servo _shoulderServo(p22);
Servo _elbowServo(p21);
Servo _wristServo(p24);
Servo _bucketServo(p25);

SerialChannel3 *_serial;

float _yawRangeRatio;
float _shoulderRangeRatio;
float _elbowRangeRatio;
float _wristRangeRatio;
float _bucketRangeRatio;

int _x;
int _y;
float _t;

inline void setShoulder(float percent) {
    if(percent > SHOULDER_MAX){
        percent = SHOULDER_MAX;
    }else if(percent < SHOULDER_MIN){
        percent = SHOULDER_MIN;
    }
    _shoulderServo = percent;
}

inline void setElbow(float percent) {
     if(percent > ELBOW_MAX){
        percent = ELBOW_MAX;
    }else if(percent < ELBOW_MIN){
        percent = ELBOW_MIN;
    }
    _elbowServo = percent;
}

inline void setWrist(float percent) {
    if(percent > WRIST_MAX){
        percent = WRIST_MAX;
    }else if(percent < WRIST_MIN){
        percent = WRIST_MIN;
    }
    _wristServo = percent;
}

inline void setBucket(float percent){
    if(percent > BUCKET_MAX){
        percent = BUCKET_MAX;
    }else if(percent < BUCKET_MIN){
        percent = BUCKET_MIN;
    }
    _bucketServo = percent;
}

inline void setYaw(float percent) {
    if(percent > YAW_MAX){
        percent = YAW_MAX;
    }else if(percent < YAW_MIN){
        percent = YAW_MIN;
    }
    _yawServo = percent;
}

void setElbowAngle(int angle){
    angle -= 11;
    float res = (0.1) + (((float) angle) / 360.0f);
    setElbow(res);
}

void setShoulderAngle(int angle){
    angle -= 10;
    float res = (0.46) - (((float) angle) / 360.0f);
    setShoulder(res);
}

void setWristAngle(int angle){
    angle -= 45;
    float res = ((float) angle) / 180;
    setWrist(res);
}

inline float toDegrees(float theta){
    return ((theta * 180.0f) / PI);
}


inline float signum(float x){
    return x < 0 ? -1 : 1;
}

void calcAngles(int x, int y, float t){
    float lengthOne = BICEP_LENGTH;
    float lengthTwo = FOREARM_LENGTH;

    float lengthSq = (x*x) + (y*y);
    float length = sqrt(lengthSq);

    float phiOne = signum(y) * acos(x / length);
    float phiTwo = acos( ((length * length) + (lengthOne * lengthOne) - (lengthTwo * lengthTwo)) / (2 * length * lengthOne) );

    float thetaOne = phiOne + phiTwo;

    float thetaTwo = acos( ((lengthOne * lengthOne) + (lengthTwo * lengthTwo) - (lengthSq)) / (2 * lengthOne * lengthTwo) );



    float wristAngle = (PI - phiTwo - thetaTwo) + ((PI / 2) - phiOne) + t;
    //pc.printf("Wrist angle: %d\r\n", (int) toDegrees(wristAngle));
    //pc.printf("Setting shoulder to: %d\r\n", (int) toDegrees(thetaOne) );
    //pc.printf("Setting elbow to: %d\r\n", (int) toDegrees(thetaTwo) );
    setShoulderAngle((int) toDegrees(thetaOne));
    setElbowAngle((int) toDegrees(thetaTwo));
    setWristAngle((int) toDegrees(wristAngle));
}


float length(int x, int y){
    return sqrt(((float) (x * x)) + ((float) (y * y)));
}

int newY(int newx, int newy){
    int ret = newy;
    if(length(newx, newy) > BICEP_LENGTH + FOREARM_LENGTH){
        //pc.printf("too big %d > %d\r\n", (int) length(newx, newy), (int) BICEP_LENGTH + FOREARM_LENGTH);
        float ang = atan2((float)newy,(float)newx);
        ret = (BICEP_LENGTH + FOREARM_LENGTH - 10) * sin(ang);
    }

    if(ret < -200){
        ret = -200;
    }

    return ret;
}

int newX(int newx, int newy){
    int ret = newx;
    if(length(newx, newy) >= BICEP_LENGTH + FOREARM_LENGTH){
        //pc.printf("too big %d > %d\r\n", (int) length(newx, newy), (int) BICEP_LENGTH + FOREARM_LENGTH);
        float ang = atan2((float)newy,(float)newx);
        ret = (BICEP_LENGTH + FOREARM_LENGTH - 10) * cos(ang);
    }

    if(ret < 30){
        ret = 30;
    }

    return ret;
}

void stow() {
    setYaw(0.25);
    //NEED TO DELAY THESE VVV
    setWrist(0.5);
    setBucket(0.08);
    setElbow(0.05);
    setShoulder(0.05);
}

void serialMessageReceived(const char *message, int length) {
    switch (ArmMessage::messageType(message)) {
    case ArmMessage::JOYSTICK: ///////////////////////////////////////////
        //TODO - this is very rough. Needs cleaning up and more adding wrist, bucket functionality.
        _x -= (ArmMessage::joyX(message) * 8) / 100;
        _y -= (ArmMessage::joyY(message) * 8) / 100;
        setYaw(_yawServo - ((float)ArmMessage::joyYaw(message) * 0.008) / 100.0);
        if (ArmMessage::bucketFullOpen(message)) {
            setBucket(BUCKET_MAX);
        }
        else if (ArmMessage::bucketFullClose(message)) {
            setBucket(BUCKET_MIN);
        }
        else {
            setBucket(_bucketServo - ((float)ArmMessage::joyBucket(message) * 0.008) / 100);
        }
        if ((_t < 1) & (_t > -1)) {
            _t = _t - ((float)ArmMessage::joyWrist(message) * 0.008) / 100;    
        }
        if (ArmMessage::stowMacro(message)) {
            stow();
        }
        else {
            _x = newX(_x,_y);
            _y = newY(_x,_y);
            calcAngles(_x, _y, _t);
        }
        break;
    case ArmMessage::MASTER_SLAVE: //////////////////////////////////////////
        if (ArmMessage::stowMacro(message)) {
            stow();
        }
        else {
            setYaw(ArmMessage::masterYaw(message) * _yawRangeRatio + YAW_MIN);
            setShoulder(ArmMessage::masterShoulder(message) * _shoulderRangeRatio + SHOULDER_MIN);
            setElbow(ArmMessage::masterElbow(message) * _elbowRangeRatio + ELBOW_MIN);
            setWrist(ArmMessage::masterWrist(message) * _wristRangeRatio + WRIST_MIN);
            if (ArmMessage::bucketFullOpen(message)) {
                setBucket(BUCKET_MAX);
            }
            else if (ArmMessage::bucketFullClose(message)) {
                setBucket(BUCKET_MIN);
            }
            else {
                setBucket(ArmMessage::masterBucket(message) * _bucketRangeRatio + BUCKET_MIN);
            }
        }
        break;
    }
}

int main() {
    _serial = new SerialChannel3(ARM_SERIAL_CHANNEL_NAME, USBTX, USBRX, &serialMessageReceived);
    //used to calculate positions in master/slave control
    _yawRangeRatio = (YAW_MAX - YAW_MIN) / (float)MAX_VALUE_14BIT;
    _shoulderRangeRatio = (SHOULDER_MAX - SHOULDER_MIN) / (float)MAX_VALUE_14BIT;
    _elbowRangeRatio = (ELBOW_MAX - ELBOW_MIN) / (float)MAX_VALUE_14BIT;
    _wristRangeRatio = (WRIST_MAX - WRIST_MIN) / (float)MAX_VALUE_14BIT;
    _bucketRangeRatio = (BUCKET_MAX - BUCKET_MIN) / (float)MAX_VALUE_14BIT;
    
    _x = 50;    //extend/retract
    _y = 50;    //elevation
    _t = 0;    //wrist
    
    //calcAngles(_x, _y, _t);

    while(1) {
        _serial->check();
        wait_ms(5);
    }
}
