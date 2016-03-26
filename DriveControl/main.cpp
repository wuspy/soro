#include "mbed.h"
#include "serialinterop.h"
#include "drivemessage.h"

#define INTERVAL 30

int main() {
    SerialChannel serial(DRIVE_SERIAL_CHANNEL_NAME, USBTX, USBRX, INTERVAL);
    while(1) {
        serial.process();
        if (serial.getAvailableMessage(message, messageSize)) {
            //TODO
        } else sleep_ms(INTERVAL);
    }
}

