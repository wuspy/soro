#include "mbed.h"
#include "serialchannel3.h"
#include "drivemessage.h"
#include "Servo.h"

#define INTERVAL 5

Servo FL(p21);
Servo FR(p22);
Servo ML(p23);
Servo MR(p24);
Servo BL(p25);
Servo BR(p26);

using namespace Soro;

int _loopsWithoutMessage;

SerialChannel3 *_serial;

//char _debugBuf[200];

void serialMessageReceived(const char *message, int length) {
    if ((length == 0) || !DriveMessage::hasValidHeader(message)) return;
    _loopsWithoutMessage = 0;
    
    //sprintf(_debugBuf, "L: %d, R: %d", DriveMessage::middleLeft(message), DriveMessage::middleRight(message));
    //_serial->log(_debugBuf, SERIAL_LOG_INFO_ID); 
    
    int fl = DriveMessage::frontLeft(message);
    int fr = DriveMessage::frontRight(message);
    int ml = DriveMessage::middleLeft(message);
    int mr = DriveMessage::middleRight(message);
    int bl = DriveMessage::backLeft(message);
    int br = DriveMessage::backRight(message);
    
    FL.write(((float)fl)/200.0 + 0.5);
    FR.write(((float)fr)/200.0 + 0.5);
    ML.write(((float)ml)/200.0 + 0.5);
    MR.write(((float)mr)/200.0 + 0.5);
    BL.write(((float)bl)/200.0 + 0.5);
    BR.write(((float)br)/200.0 + 0.5);
}

int main() {
    _serial = new SerialChannel3(DRIVE_SERIAL_CHANNEL_NAME, USBTX, USBRX, &serialMessageReceived);
    while(1) {
        _serial->check();
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
