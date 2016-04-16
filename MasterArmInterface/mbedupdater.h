#ifndef MBEDUPDATER_H
#define MBEDUPDATER_H

#include "mbed.h"
#include "EthernetInteface.h"
#include "TCPSocketConnection.h"
#include "soro_global.h"
#include <cstring>

extern "C" void mbed_reset();

namespace Soro {

    static void checkUpdates(char mbedId, const char* server, int port) {
        TCPSocketConnection socket;
        socket.init();
        char buffer[1024];
        //connect and send mbed ID
        socket.connect(server, port);
        socket.send(&mbedId, 1);
        //read update size
        socket.recieve(&buffer[0], 4);
        int length;
        deserialize<int>(&buffer[0], length);
        //read update hash
        char hash[16];
        socket.recieve(&buffer[0], 16);
        if (length > 0)
            LocalFileSystem local("local");
            FILE *fp = fopen("/local/update.bin", "w");
            int position = 0;
            while (position < length) {
                int status = socket.recieve(&buffer[0], 1024);
                position += status;
                fwrite(&buffer[0], sizeof(char), status, fp);
            }
            fclose(fp);
        }
        mbed_reset();
    }
}

#endif // MBEDUPDATER_H
