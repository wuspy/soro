#ifndef SORO_NMEAMESSAGE_H
#define SORO_NMEAMESSAGE_H

#include <QDataStream>

namespace Soro {

/* Structure for holding GPS information and parsing NMEA messages
 */
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

#endif // SORO_NMEAMESSAGE_H
