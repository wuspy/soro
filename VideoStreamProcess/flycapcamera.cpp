#include "flycapcamera.h"

namespace Soro {
namespace Rover {

/////////////////////////////////////////////////////

FlycapCamera::FlycapCamera(FlyCapture2::PGRGuid guid, QObject *parent) : QObject(parent) {
    // connect to flycapture camera
    FlyCapture2::Error error = _camera.Connect(&guid);
    if (error != FlyCapture2::PGRERROR_OK) {
        qCritical() << "Could not connect to FlyCapture camera";
        QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
    }

    // Query for available Format 7 modes
    FlyCapture2::Format7Info fmt7Info;
    bool supported;
    fmt7Info.mode = FlyCapture2::MODE_0;
    error = _camera.GetFormat7Info(&fmt7Info, &supported);
    if (error != FlyCapture2::PGRERROR_OK) {
        qCritical() << "Could not determine available FlyCapture2 Format7 modes: " << error.GetDescription();
        QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
    }

    qDebug() << "Queried Format7 info for FlyCapture camera";
    FlyCapture2::Format7ImageSettings fmt7ImageSettings;
    fmt7ImageSettings.mode = FlyCapture2::MODE_0;

    fmt7ImageSettings.offsetX = 0;
    fmt7ImageSettings.offsetY = 0;
    fmt7ImageSettings.width = fmt7Info.maxWidth;
    fmt7ImageSettings.height = fmt7Info.maxHeight;

    fmt7ImageSettings.pixelFormat = FlyCapture2::PIXEL_FORMAT_422YUV8;

    bool valid;
    FlyCapture2::Format7PacketInfo fmt7PacketInfo;

    // Validate the settings to make sure that they are valid
    error = _camera.ValidateFormat7Settings(
            &fmt7ImageSettings,
            &valid,
            &fmt7PacketInfo );
    if (error != FlyCapture2::PGRERROR_OK) {
        qCritical() << "Could not validate FlyCapture camera mode: " << error.GetDescription();
        QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
    }

    qDebug() << "Validated new Format7 settings for FlyCapture camera";

    if (!valid) {
        // Settings are not valid
        qCritical() << "FlyCapture image settings are invalid";
        QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
    }

    // Set the settings to the camera
    error = _camera.SetFormat7Configuration(
            &fmt7ImageSettings,
            fmt7PacketInfo.recommendedBytesPerPacket);
    if (error != FlyCapture2::PGRERROR_OK) {
        qCritical() << "Could not set FlyCapture camera mode: " << error.GetDescription();
        QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
    }

    qDebug() << "Set FlyCapture Format7 configuration";

    setLatency(-1, -1);
    setLive(true);
    setStreamType(QGst::AppStreamTypeStream);
    setSize(-1);
    QString caps = "video/x-raw,format=UYVY,"
                    "width=(int)" + QString::number(fmt7Info.maxWidth) + ","
                    "height=(int)" + QString::number(fmt7Info.maxHeight) + ","
                    "framerate=(fraction)15/1";

    // configure application source
    setCaps(QGst::Caps::fromString(caps));

    //start capturing
    _camera.StartCapture();
    qDebug() << "Beginning capture for FlyCapture2 camera";

    _captureTimerId = startTimer(1000 / 15, Qt::PreciseTimer);
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
    if (_needsData) {
        if (!_camera.IsConnected()) {
            // camera is no longer connected
            qCritical() << "FlyCapture camera does not appear to be connected";
            endOfStream();
            _needsData = false;
            return;
        }

        // get raw image
        FlyCapture2::Image image;
        FlyCapture2::Error error = _camera.RetrieveBuffer(&image);
        if (error != FlyCapture2::PGRERROR_OK) {
            // error receiving raw image
            _errorCount++;
            if (_errorCount > 10) {
                stopWithError("Error receiving raw image from FlyCapture camera");
            }
            return;
        }

        // create a GstBuffer to hold the converted image
        QGst::BufferPtr ptr = QGst::Buffer::create(image.GetDataSize());
        if (!ptr) {
            // error allocating frame buffer
            _errorCount++;
            if (_errorCount > 10) {
                stopWithError("Error allocating frame buffer");
            }
            return;
        }
        QGst::MapInfo memory;
        ptr->map(memory, QGst::MapWrite);
        if (!memory.data()) {
            // could not map memory
            _errorCount++;
            if (_errorCount > 10) {
                stopWithError("Error mapping memory from GstBuffer");
            }
            return;
        }

        memcpy(memory.data(), image.GetData(), (size_t)image.GetDataSize());

        //push frame to source
        ptr->unmap(memory);
        pushBuffer(ptr);
        _errorCount = 0;
    }
}

void FlycapCamera::stopWithError(QString error) {
    qCritical() << "Error mapping memory from GstBuffer";
    if (_camera.IsConnected()) {
        _camera.StopCapture();
        _camera.Disconnect();
    }
    QCoreApplication::exit(STREAMPROCESS_ERR_FLYCAP_ERROR);
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
