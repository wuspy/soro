#include "mbeddataparser.h"

#define LOG_TAG "MbedDataParser"

const char *TAGS[] =
{
    "!", // Wheeldata 1
    "@", // Wheeldata 2
    "#", // Wheeldata 3
    "$", // Wheeldata 4
    "%", // Wheeldata 5
    "^", // Wheeldata 6
    "+!", // IMUdata 1 X
    "+@", // IMUdata 1 Y
    "+#", // IMUdata 1 Z
    "~!", // IMUdata 2 X
    "~@", // IMUdata 2 Y
    "~#" // IMUdata 2 Z
};

namespace Soro {
namespace Rover {

MbedDataParser::MbedDataParser(MbedChannel *mbed, QObject *parent) : QObject(parent) {
    _mbed = mbed;
    connect(_mbed, SIGNAL(messageReceived(MbedChannel*,const char*,int)),
            this, SLOT(messageReceived(MbedChannel*,const char*,int)));
}

MbedDataParser::~MbedDataParser() {

}

void MbedDataParser::messageReceived(MbedChannel *mbed, const char* data, int len) {
    _buffer.append(data, len);
}

void MbedDataParser::parseBuffer() {
    for (int i = 0; i < 12; i++) {
        if (_buffer.startsWith(TAGS[i])) {
            parseNext(reinterpret_cast<DataTag&>(i), strlen(TAGS[i]));
            parseBuffer();
            return;
        }
    }
    LOG_E(LOG_TAG, "Invalid token, buffer contents: " + QString(_buffer));
}

bool MbedDataParser::isNumeric(char c) {
    unsigned char v = reinterpret_cast<unsigned char&>(c);
    return (v == 46) || ((v > 47) && (v < 58));
}

void MbedDataParser::parseNext(DataTag tag, int start) {
    if (start >= _buffer.length()) return;

    int end = start;
    while (isNumeric(_buffer.at(end))) {
        end++;
        if (end == _buffer.length()) return;
    }

    float value = _buffer.mid(start, end - start).toFloat();
    _buffer.remove(0, end);
    emit newData(tag, value);
}

}
}
