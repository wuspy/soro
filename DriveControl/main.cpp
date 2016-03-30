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

SerialChannel3 *_serial;

void serialMessageReceived(const char *message, int length) {
    if ((length == 0) || !DriveMessage::hasValidHeader(message)) return;
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
    _serial = new SerialChannel3(DRIVE_SERIAL_CHANNEL_NAME, USBTX, USBRX, &serialMessageReceived);
    while(1) {
        _serial->checkConnection();
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
