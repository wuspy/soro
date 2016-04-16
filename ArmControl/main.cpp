#include "mbed.h"
#include "Servo.h"
#include "armmessage.h"
#include "mbedchannel.h"
#include "soro_global.h"

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

//values from old arm
/*#define SHOULDER_MIN 0.05
#define SHOULDER_MAX 0.55
#define ELBOW_MIN 0.1
#define ELBOW_MAX 0.60
#define WRIST_MIN 0
#define WRIST_MAX 0.4
#define BUCKET_MIN 0
#define BUCKET_MAX 0.56
#define YAW_MIN 0
#define YAW_MAX 1*/

#define SHOULDER_MIN 0.01
#define SHOULDER_MAX 0.46
#define ELBOW_MIN 0.55
#define ELBOW_MAX 1
#define WRIST_MIN 0.3
#define WRIST_MAX 1
#define BUCKET_MIN 0
#define BUCKET_MAX 0.75
#define YAW_MIN 0
#define YAW_MAX 1

#define SHOULDER_HOME SHOULDER_MIN
#define ELBOW_HOME ELBOW_MAX
#define YAW_HOME 0.05
#define WRIST_HOME 0.5
#define BUCKET_HOME 0.4
#define BUCKET_OPEN BUCKET_MIN
#define BUCKET_CLOSED BUCKET_MAX

using namespace Soro;

Servo _yawServo(p23);
Servo _shoulderServo(p22);
Servo _elbowServo(p21);
Servo _wristServo(p24);
Servo _bucketServo(p25);

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

/* Sets the arm in the stow position.
 * If 'safe' it will wait until the yaw is positioned before
 * moving any other servos. Otherwise, all servos will be moved at once.
 */
void stow(bool safe) {
    float yawOrig = _yawServo;
    setYaw(YAW_HOME);
    if (safe) {
        //wait until the yaw is stowed
        wait((yawOrig - YAW_HOME) * 3); //3 seconds times distance to home
    }
    setWrist(WRIST_HOME);
    setBucket(BUCKET_HOME);
    setElbow(ELBOW_HOME);
    setShoulder(SHOULDER_HOME);
}

/* Listener which receives the ethernet's disconnected
 * event (which triggers a reset). The arm must be stowed
 * before this happens otherwise it may stow improperly
 * when the mbed turns back on.
 */
void preResetListener() {
    stow(true);
}

int main() {
    //used to calculate positions in master/slave control
    _yawRangeRatio = (YAW_MAX - YAW_MIN) / (float)MAX_VALUE_14BIT;
    _shoulderRangeRatio = (SHOULDER_MAX - SHOULDER_MIN) / (float)MAX_VALUE_14BIT;
    _elbowRangeRatio = (ELBOW_MAX - ELBOW_MIN) / (float)MAX_VALUE_14BIT;
    _wristRangeRatio = (WRIST_MAX - WRIST_MIN) / (float)MAX_VALUE_14BIT;
    _bucketRangeRatio = (BUCKET_MAX - BUCKET_MIN) / (float)MAX_VALUE_14BIT;
    
    MbedChannel ethernet(MBED_ID_ARM);   
    ethernet.setResetListener(&preResetListener);
    ethernet.setTimeout(500);
    char buffer[50];
    
    //Stow the arm. This will end very bad if the arm is not
    //alrady close to stow position, but we have no choice.
    stow(false);
    
    while(1) {
        int len = ethernet.read(&buffer[0], 50);
        if (len != -1) {
            switch (ArmMessage::messageType(buffer)) {
             case ArmMessage::JOYSTICK: ///////////////////////////////////////////
                //TODO - this is very rough. Needs cleaning up and more adding wrist, bucket functionality.
                _x -= (ArmMessage::joyX(buffer) * 8) / 100;
                _y -= (ArmMessage::joyY(buffer) * 8) / 100;
                setYaw(_yawServo - ((float)ArmMessage::joyYaw(buffer) * 0.008) / 100.0);
                if (ArmMessage::bucketFullOpen(buffer)) {
                    setBucket(BUCKET_MAX);
                }
                else if (ArmMessage::bucketFullClose(buffer)) {
                    setBucket(BUCKET_MIN);
                }
                else {
                    setBucket(_bucketServo - ((float)ArmMessage::joyBucket(buffer) * 0.008) / 100);
                }
                if ((_t < 1) & (_t > -1)) {
                    _t = _t - ((float)ArmMessage::joyWrist(buffer) * 0.008) / 100;    
                }
                if (ArmMessage::stowMacro(buffer)) {
                    stow(true);
                }
                else {
                    _x = newX(_x,_y);
                    _y = newY(_x,_y);
                    calcAngles(_x, _y, _t);
                }
                break;
            case ArmMessage::MASTER_SLAVE: //////////////////////////////////////////
                if (ArmMessage::stowMacro(buffer)) {
                    stow(true);
                }
                else {
                    setYaw(ArmMessage::masterYaw(buffer) * _yawRangeRatio + YAW_MIN);
                    setShoulder(ArmMessage::masterShoulder(buffer) * _shoulderRangeRatio + SHOULDER_MIN);
                    setElbow(ArmMessage::masterElbow(buffer) * _elbowRangeRatio + ELBOW_MIN);
                    setWrist(ArmMessage::masterWrist(buffer) * _wristRangeRatio + WRIST_MIN);
                    if (ArmMessage::bucketFullOpen(buffer)) {
                        setBucket(BUCKET_OPEN);
                    }
                    else if (ArmMessage::bucketFullClose(buffer)) {
                        setBucket(BUCKET_CLOSED);
                    }
                    else {
                        setBucket(ArmMessage::masterBucket(buffer) * _bucketRangeRatio + BUCKET_MIN);
                    }
                }
                break;
            }
        }
    }
}