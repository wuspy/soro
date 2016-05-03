#include "flycapsource.h"

namespace Soro {
namespace Rover {

FlycapSource::FlycapSource(FlyCapture2::PGRGuid guid, Logger *log, QObject *parent) : QObject(parent) {
    _log = log;
    LOG_TAG = "FlycapSource(*-" + QString::number(guid.value[3]) + ")";

    LOG_I("Creating new FlycapSource");

    // connect to flycapture camera
    FlyCapture2::Error error = _camera.Connect(&guid);
    if (error != FlyCapture2::PGRERROR_OK) {
        LOG_E("Could not connect to flycap camera with given guid");
    }

    // get camera resolution for caps
    FlyCapture2::CameraInfo info;
    _camera.GetCameraInfo(&info);

    QString resolution(info.sensorResolution);
    _width = atoi(resolution.mid(0, resolution.indexOf("x")).toLocal8Bit().constData());
    _height = atoi(resolution.mid(resolution.indexOf("x") + 1).toLocal8Bit().constData());
    if ((_width == 0) | (_height == 0)) {
        LOG_E("Could not determine flycap sensor resolution");
    }

    LOG_I("Camera sensor is " + QString(info.sensorResolution) + " " +  QString(info.sensorInfo));

    setLatency(-1, -1);
    setLive(true);
    setStreamType(QGst::AppStreamTypeStream);
    setSize(-1);

    // start flycapture camera
    _camera.StartCapture();
    _enabled = false;

    setFramerate(30); // default
}

FlycapSource::~FlycapSource() {
    if (_camera.IsConnected()) {
        _camera.StopCapture();
        _camera.Disconnect();
    }
}

void FlycapSource::setFramerate(int framerate) {
    _framerate = framerate;
    KILL_TIMER(_captureTimerId);
    START_TIMER(_captureTimerId, 1000 / _framerate);

    // create caps string
    QString caps = "video/x-raw,format=RGB,"
                    "width=(int)" + QString::number(_width) + ","
                    "height=(int)" + QString::number(_height) + ","
                    "framerate=(fraction)" + QString::number(_framerate) + "/1";

    // configure application source
    setCaps(QGst::Caps::fromString(caps));
}

void FlycapSource::timerEvent(QTimerEvent *e) {
    Q_UNUSED(e);
    if (_enabled) {
        if (!_camera.IsConnected()) {
            LOG_E("Flycap camera is no longer connected (or never was connected), sending EOS");
            endOfStream();
            _enabled = false;
            return;
        }

        // get raw image from camera
        FlyCapture2::Image rawImage;
        FlyCapture2::Error error = _camera.RetrieveBuffer(&rawImage);
        if (error != FlyCapture2::PGRERROR_OK) {
            LOG_W("Error reading image from flycap camera");
            return;
        }

        // convert image
        FlyCapture2::Image convertedImage;
        error = rawImage.Convert(FlyCapture2::PIXEL_FORMAT_RGB, &convertedImage);
        if (error != FlyCapture2::PGRERROR_OK) {
            LOG_W("Error converting camera RAW data to RGB");
            return;
        }

        // wrap in a GstBuffer
        QGst::BufferPtr ptr = QGst::Buffer::create(convertedImage.GetDataSize());
        QGst::MapInfo memory;
        ptr->map(memory, QGst::MapWrite);
        memcpy(memory.data(), convertedImage.GetData(), convertedImage.GetDataSize());
        ptr->unmap(memory);

        // push buffer to app source pad
        pushBuffer(ptr);
    }
}

void FlycapSource::needData(uint length) {
    Q_UNUSED(length);
    _enabled = true;
}

void FlycapSource::enoughData() {
    _enabled = false;
}

int FlycapSource::getVideoWidth() const {
    return _width;
}

int FlycapSource::getFramerate() const {
    return _framerate;
}

const FlyCapture2::Camera* FlycapSource::getCamera() const {
    return &_camera;
}

int FlycapSource::getVideoHeight() const {
    return _height;
}

} // namespace Rover
} // namespace Soro
