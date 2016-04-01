#include "mbed.h"
#include "Servo.h"
#include "serialchannel3.h"
#include "armmessage.h"

#define INTERVAL 50
 
AnalogIn yaw(p15);
AnalogIn shoulder(p16);
AnalogIn elbow(p17);
AnalogIn wrist(p18);
AnalogIn bucket(p19);

using namespace Soro;

int main() {
    SerialChannel3 serial(MASTER_ARM_SERIAL_CHANNEL_NAME, USBTX, USBRX, NULL);
    char buffer[50];
    unsigned short shoulderVal, yawVal, elbowVal, wristVal, bucketVal;
    while(1) {
        serial.check();
        yawVal = yaw.read() * MAX_VALUE_14BIT;
        shoulderVal = shoulder.read() * MAX_VALUE_14BIT;
        elbowVal = elbow.read() * MAX_VALUE_14BIT;
        wristVal = wrist.read() * MAX_VALUE_14BIT;
        bucketVal = bucket.read() * MAX_VALUE_14BIT;
        ArmMessage::setMasterArmData(&buffer[0], yawVal, shoulderVal, elbowVal, wristVal, bucketVal);
        serial.sendMessage(&buffer[0], ArmMessage::size(&buffer[0]));
        wait_ms(INTERVAL);
    }
}
