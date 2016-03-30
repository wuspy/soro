#include "mbed.h"
#include "serialchannel3.h"
#include "drivemessage.h"
#include "Servo.h"

#define INTERVAL 30

Servo FL(p21);
Servo FR(p22);
Servo ML(p23);
Servo MR(p24);
Servo BL(p25);
Servo BR(p26);

using namespace Soro;

int _loopsWithoutMessage;

void serialMessageReceived(const char *message, int length) {
    if (message == NULL || !DriveMessage::hasValidHeader(message)) return;
    _loopsWithoutMessage = 0;
    signed char fl = DriveMessage::frontLeft(message);
    signed char fr = DriveMessage::frontRight(message);
    signed char ml = DriveMessage::middleLeft(message);
    signed char mr = DriveMessage::middleLeft(message);
    signed char bl = DriveMessage::backLeft(message);
    signed char br = DriveMessage::backRight(message);
    
    FL.write(((float)fl)/100.0 + 0.5);
    FR.write(((float)fr)/100.0 + 0.5);
    ML.write(((float)ml)/100.0 + 0.5);
    MR.write(((float)mr)/100.0 + 0.5);
    BL.write(((float)bl)/100.0 + 0.5);
    BR.write(((float)br)/100.0 + 0.5);
}

int main() {
    SerialChannel3 serial(DRIVE_SERIAL_CHANNEL_NAME, USBTX, USBRX, &serialMessageReceived);
    while(1) {
        serial.checkMessages();
        if (_loopsWithoutMessage != 0) {
            if (_loopsWithoutMessage * INTERVAL > 1000) {
                //stop after 1 second without instruction
                FL.write(0.5);
                FR.write(0.5);
                ML.write(0.5);
                MR.write(0.5);
                BL.write(0.5);
                BR.write(0.5);
            }
            wait_ms(INTERVAL);
        }
        _loopsWithoutMessage++;
    }
}
