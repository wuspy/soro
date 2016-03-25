#include "mbed.h"
#include "Servo.h"
#include "serialinterop.h"
#include "armmessage.h"

#define INTERVAL 30
 
AnalogIn yaw(p15);
AnalogIn shoulder(p16);
AnalogIn elbow(p17);
AnalogIn wrist(p18);
AnalogIn bucket(p19);

using namespace Soro;

int main() {
    SerialChannel serial(SERIAL_MASTER_ARM_CHANNEL_NAME, USBTX, USBRX, INTERVAL);
    char buffer[ARM_MASTER_MESSAGE_SIZE];
    unsigned short shoulderVal, yawVal, elbowVal, wristVal, bucketVal;
    
    while(1) {
        serial.process();
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
