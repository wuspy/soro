#ifndef NMEAMESSAGE_H
#define NMEAMESSAGE_H

#include <QDataStream>

namespace Soro {

struct NmeaMessage {

    double Latitude, Longitude;
    /* Number of fixed satellites
     */
    int Satellites;
    /* Altitude in meters above sea level
     */
    int Altitude;

    /* Heading, in degrees relative to north
     */
    int Heading;

    /* Ground speed in kilometers per hour
     */
    double GroundSpeed;

    friend QDataStream& operator<<(QDataStream& stream, const NmeaMessage& message) {
        stream << message.Latitude;
        stream << message.Longitude;
        stream << (qint32)message.Satellites;
        stream << (qint32)message.Altitude;
        stream << (qint32)message.Heading;
        stream << message.GroundSpeed;

        return stream;
    }

    friend QDataStream& operator>> (QDataStream& stream, NmeaMessage& message) {
        stream >> message.Latitude;
        stream >> message.Longitude;
        qint32 temp;
        stream >> temp;
        message.Satellites = (int)temp;
        stream >> temp;
        message.Altitude = (int)temp;
        stream >> temp;
        message.Heading = (int)temp;
        stream >> message.GroundSpeed;

        return stream;
    }
};

}

#endif // NMEAMESSAGE_H
