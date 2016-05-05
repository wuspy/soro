#include "flycapcamera.h"

namespace Soro {
namespace Rover {

/////////////////////////////////////////////////////

FlycapCamera::FlycapCamera(FlyCapture2::PGRGuid guid, int framerate, QObject *parent) : QObject(parent) {
    // connect to flycapture camera
    FlyCapture2::Error error = _camera.Connect(&guid);
    if (error != FlyCapture2::PGRERROR_OK) {
        qCritical() << "Could not connect to FlyCapture camera";
        QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
    }

    // get camera resolution for caps
    FlyCapture2::CameraInfo info;
    _camera.GetCameraInfo(&info);

    QString resolution(info.sensorResolution);
    int width = atoi(resolution.mid(0, resolution.indexOf("x")).toLocal8Bit().constData());
    int height = atoi(resolution.mid(resolution.indexOf("x") + 1).toLocal8Bit().constData());
    if ((width == 0) | (height == 0)) {
        qCritical() << "Could not determine FlyCapture camera sensor information";
        QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
    }

    setLatency(-1, -1);
    setLive(true);
    setStreamType(QGst::AppStreamTypeStream);
    setSize(-1);
    QString caps = "video/x-raw,format=RGB,"
                    "width=(int)" + QString::number(width) + ","
                    "height=(int)" + QString::number(height) + ","
                    "framerate=(fraction)" + QString::number(framerate) + "/1";

    // configure application source
    setCaps(QGst::Caps::fromString(caps));

    //start capturing
    _camera.StartCapture();
    _captureTimerId = startTimer(1000 / framerate, Qt::PreciseTimer);
}

FlycapCamera::~FlycapCamera() {
    endOfStream();
    killTimer(_captureTimerId);
    if (_camera.IsConnected()) {
        _camera.StopCapture();
        _camera.Disconnect();
    }
}

void FlycapCamera::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (_needsData) {
        if (!_camera.IsConnected()) {
            // camera is no longer connected
            qCritical() << "FlyCapture camera does not appear to be connected";
            endOfStream();
            _needsData = false;
            return;
        }

        // get raw image
        FlyCapture2::Error error = _camera.RetrieveBuffer(&_imageBuffer);
        if (error != FlyCapture2::PGRERROR_OK) {
            // error receiving raw image
            _errorCount++;
            if (_errorCount > 30) {
                qWarning() << "Error receiving raw image from FlyCapture camera";
                _camera.StopCapture();
                _camera.Disconnect();
                QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
            }
            return;
        }

        // create a GstBuffer to hold the converted image
        QGst::BufferPtr ptr = QGst::Buffer::create(3 * _imageBuffer.GetRows() * _imageBuffer.GetCols());
        if (!ptr) {
            // error allocating frame buffer
            _errorCount++;
            if (_errorCount > 30) {
                qWarning() << "Error allocating frame buffer";
                _camera.StopCapture();
                _camera.Disconnect();
                QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
            }
            return;
        }
        QGst::MapInfo memory;
        ptr->map(memory, QGst::MapWrite);
        if (!memory.data()) {
            // could not map memory
            _errorCount++;
            if (_errorCount > 30) {
                qWarning() << "Error mapping memory from GstBuffer";
                _camera.StopCapture();
                _camera.Disconnect();
                QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
            }
            return;
        }
        // convert image
        FlyCapture2::Image convertedImage;
        error = convertedImage.SetData(memory.data(), memory.size());
        if (error != FlyCapture2::PGRERROR_OK) {
            // could not assign buffer
            _errorCount++;
            if (_errorCount > 30) {
                qWarning() << "Could not mutate GstBuffer";
                _camera.StopCapture();
                _camera.Disconnect();
                QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
            }
            return;
        }
        error = _imageBuffer.Convert(FlyCapture2::PIXEL_FORMAT_RGB, &convertedImage);
        if (error != FlyCapture2::PGRERROR_OK) {
            // could not convert raw to rgb
            _errorCount++;
            if (_errorCount > 30) {
                qWarning() << "Could not convert FlyCapture RAW to RGB";
                _camera.StopCapture();
                _camera.Disconnect();
                QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
            }
            return;
        }

        //push frame to source
        ptr->unmap(memory);
        pushBuffer(ptr);
        _errorCount = 0;
    }
}

void FlycapCamera::needData(uint length) {
    Q_UNUSED(length);
    _needsData = true;
}

void FlycapCamera::enoughData() {
    _needsData = false;
}

} // namespace Rover
} // namespace Soro
