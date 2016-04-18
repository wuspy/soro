#include "mbed.h"
#include "Servo.h"
#include "armmessage.h"
#include "soro_global.h"
#include "mbedchannel.h"

#define INTERVAL 50
 
AnalogIn yaw(p15);
AnalogIn shoulder(p16);
AnalogIn elbow(p17);
AnalogIn wrist(p18);
AnalogIn bucket(p19);

using namespace Soro;

int main() {
    MbedChannel ethernet(MBED_ID_MASTER_ARM);
    char buffer[50];
    short shoulderVal, yawVal, elbowVal, wristVal, bucketVal;
    while(1) {
        yawVal = yaw.read() * USHRT_MAX;
        shoulderVal = shoulder.read() * USHRT_MAX;
        elbowVal = elbow.read() * USHRT_MAX;
        wristVal = wrist.read() * USHRT_MAX;
        bucketVal = bucket.read() * USHRT_MAX;
        ArmMessage::setMasterArmData(&buffer[0], yawVal, shoulderVal, elbowVal, wristVal, bucketVal);
        ethernet.sendMessage(&buffer[0], ArmMessage::size(&buffer[0]));
        wait_ms(INTERVAL);
    }
}
