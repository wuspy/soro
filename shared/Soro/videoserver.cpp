#include "videoserver.h"

#define MJPEG_COMMAND(device, width, height, frate, quality) \
    

namespace Soro {

VideoServer::VideoServer(SocketAddress host, QString cameraDevice, bool clientFirewalled, QString name,
                         VideoEncoding encoding, QObject *parent) : QObject(parent) {
    _host = host;
    _cameraDevice = cameraDevice;
    _firewalled = clientFirewalled;
    _name = name;
    _encoding = encoding;
    restart();
}

VideoServer::~VideoServer() {
    if (_flycap_camera) {
        _flycap_camera->StopCapture();
        _flycap_camera->Disconnect();
        delete _flycap_camera;
    }
}

void VideoServer::restart() {
    if (_firewalled) {

    }
}

void VideoServer::setDimensions(int width, int height) {
    _width = width;
    _height = height;
}

void VideoServer::setFramerate(int fps) {
    _framerate = fps;
}

void VideoServer::mjpeg_setQuality(int quality) {
    _mjpeg_quality = quality;
}

void VideoServer::flycap_connectCamera() {
    //TODO implement a shared list to avoid connecting
    //to the same PGRguid (Point Grey GUID)
    cout<<"STARTING CONNECTION FUNCTION"<<endl;
    FlyCapture2::Error error;
    FlyCapture2::BusManager busMgr;
    unsigned int numCameras;
    FlyCapture2::PGRGuid guid;

    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != FlyCapture2::PGRERROR_OK) {
        PrintError (error);
    }
    qDebug() << "Number of flycap cameras detected: " << numCameras;

    for (unsigned int i=0; i < numCameras; i++) {
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != FlyCapture2::PGRERROR_OK) {
            PrintError( error );
        }

    }
    // Connect the camera
    error = camera.Connect(&guid);
    if (error != FlyCapture2::PGRERROR_OK) {
        std::cout << "Failed to connect to flycap camera " << guid;
        return;
    }
    else {
      qDebug() << "Connectd to flycap camera " << guid;
    }
}

void VideoServer::flycap_getImagePtr(quint8 * &ptr, int &size, bool stackPointer) {
    // Get the image
    FlyCapture2::Image rawImage;
    FlyCapture2::Error error = camera.RetrieveBuffer(&rawImage);
    if (error != FlyCapture2::PGRERROR_OK) {
        qWarning("Error reading from flycap camera");
        return;
    }

    // convert to rgb
    FlyCapture2::Image bgrImage;
    rawImage.Convert(FlyCapture2::PIXEL_FORMAT_BGR, &bgrImage);

    if (stackPointer) {
        //don't perform memcpy to save time, however
        //this image will be a pointer to the stack
        ptr = bgrImage.GetData();
    }
    else {
        //Copy the image data to the heap
        ptr = new quint8[bgrImage.GetDataSize()];
        if (!ptr) {
            qCritical() << "Error trying to allocate " << bgrImage.GetDataSize() << " bytes";
            return;
        }
        memcpy(ptr, bgrImage.GetData(), bgrImage.GetDataSize());
    }

    size = bgrImage.GetDataSize();
}


}
