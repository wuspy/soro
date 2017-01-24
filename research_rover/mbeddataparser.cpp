#include "mbeddataparser.h"

#define LOG_TAG "MbedDataParser"

#define TAG_WHEELDATA_1 "!"
#define TAG_WHEELDATA_2 "@"
#define TAG_WHEELDATA_3 "#"
#define TAG_WHEELDATA_4 "$"
#define TAG_WHEELDATA_5 "%"
#define TAG_WHEELDATA_6 "^"

#define TAG_IMUDATA_1_X "+!"
#define TAG_IMUDATA_1_Y "+@"
#define TAG_IMUDATA_1_Z "+#"
#define TAG_IMUDATA_2_X "~!"
#define TAG_IMUDATA_2_Y "~@"
#define TAG_IMUDATA_2_Z "~#"

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
    if (_buffer.startsWith(TAG_WHEELDATA_1)) {
        parseNext(DATATAG_WHEELDATA_1, strlen(TAG_WHEELDATA_1));
    }
    else if (_buffer.startsWith(TAG_WHEELDATA_2)) {
        parseNext(DATATAG_WHEELDATA_2, strlen(TAG_WHEELDATA_2));
    }
    else if (_buffer.startsWith(TAG_WHEELDATA_3)) {
        parseNext(DATATAG_WHEELDATA_3, strlen(TAG_WHEELDATA_3));
    }
    else if (_buffer.startsWith(TAG_WHEELDATA_4)) {
        parseNext(DATATAG_WHEELDATA_4, strlen(TAG_WHEELDATA_4));
    }
    else if (_buffer.startsWith(TAG_WHEELDATA_5)) {
        parseNext(DATATAG_WHEELDATA_5, strlen(TAG_WHEELDATA_5));
    }
    else if (_buffer.startsWith(TAG_WHEELDATA_6)) {
        parseNext(DATATAG_WHEELDATA_6, strlen(TAG_WHEELDATA_6));
    }
}

bool MbedDataParser::isNumeric(char c) {
    unsigned char v = reinterpret_cast<unsigned char&>(c);
    return (v == 46) || ((v > 47) && (v < 58));
}

void MbedDataParser::parseNext(DataTag tag, int offset) {
    int len = offset;
    while (isNumeric(_buffer.at(len))) {
        len++;
    }
    float value = _buffer.mid(offset, len).toFloat();
    _buffer.remove(0, offset + len);
    emit newData(tag, value);
}

}
}
