#include "mbed.h"
#include "mbedchannel.h"
#include "soro_global.h"
#include "drivemessage.h"
#include "Servo.h"

#define INTERVAL 5

Servo FL(p21); //B
Servo FR(p22); //C
Servo ML(p23); //A
Servo MR(p24); //D
Servo BL(p25); //F
Servo BR(p26); //E

using namespace Soro;

void preResetListener() {
    FL.write(0.5);
    FR.write(0.5);
    ML.write(0.5);
    MR.write(0.5);
    BL.write(0.5);
    BR.write(0.5);
}

int main() {
    MbedChannel ethernet(MBED_ID_DRIVE);
    ethernet.setResetListener(&preResetListener);
    ethernet.setTimeout(500); //rover will stop if this timeout is reached
    char buffer[50];
    while(1) {
        int len = ethernet.read(&buffer[0], 50);
        if ((len == -1) || !DriveMessage::hasValidHeader(buffer)) {
            //stop
            FL.write(0.5);
            FR.write(0.5);
            ML.write(0.5);
            MR.write(0.5);
            BL.write(0.5);
            BR.write(0.5);
        }
        else {
            int fl = DriveMessage::frontLeft(buffer);
            int fr = DriveMessage::frontRight(buffer);
            int ml = DriveMessage::middleLeft(buffer);
            int mr = DriveMessage::middleRight(buffer);
            int bl = DriveMessage::backLeft(buffer);
            int br = DriveMessage::backRight(buffer);
            
            FL.write(((float)fl)/200.0 + 0.5);
            FR.write(((float)fr)/200.0 + 0.5);
            ML.write(((float)ml)/200.0 + 0.5);
            MR.write(((float)mr)/200.0 + 0.5);
            BL.write(((float)bl)/200.0 + 0.5);
            BR.write(((float)br)/200.0 + 0.5);
        }
    }
}
