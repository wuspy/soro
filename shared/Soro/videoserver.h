#ifndef VIDEOSERVER_H
#define VIDEOSERVER_H

#include <QObject>
#include <QUdpSocket>

#include <flycapture/FlyCapture2.h>

#include <iostream>
#include <sstream>

#include "socketaddress.h"
#include "videoencoding.h"

namespace Soro {

class VideoServer : public QObject {
    Q_OBJECT
public:
    /* Creates a new gstreamer video stream server
     */
    explicit VideoServer(SocketAddress host, QString cameraDevice, bool clientFirewalled, QString name,
                         VideoEncoding encoding, QObject *parent = 0);

    ~VideoServer();

    void setDimensions(int width, int height);
    void setFramerate(int fps);
    void mjpeg_setQuality(int quality);
    void restart();

    /* Connects the first available flycap camera
     * available
     */
    void flycap_connectCamera();

    /* Gets a pointer to the last image taken by the flycap camera
     */
    void flycap_getImagePointer(quint8* &ptr, int &size, bool stackPointer);

private:
    int _width = 640, _height = 480, _framerate = 20;
    int _mjpeg_quality = 50;
    QString _cameraDevice;
    QString _name;
    VideoEncoding _encoding;
    bool _firewalled;
    SocketAddress _host;
    QUdpSocket *_socket = NULL;

    FlyCapture2::Camera *_flycap_camera = NULL;

signals:

public slots:
};

}

#endif // VIDEOSERVER_H
