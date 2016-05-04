#include "flycapcamera.h"

namespace Soro {
namespace Rover {

FlycapCamera::FlycapCamera(FlyCapture2::PGRGuid guid, Logger *log, QObject *parent) : QObject(parent) {
    _log = log;
    LOG_TAG = "FlycapCamera(*-" + QString::number(guid.value[3]) + ")";

    LOG_I("Creating new FlycapCamera");

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
}

FlycapCamera::~FlycapCamera() {
    clearSource();
    if (_camera.IsConnected()) {
        _camera.StopCapture();
        _camera.Disconnect();
    }
}

QGst::ElementPtr FlycapCamera::createSource(int framerate) {
    LOG_I("Creating new gstreamer source with framerate " + QString::number(framerate));
    clearSource();

    _framerate = framerate;
    _source = new FlycapSource(_width, _height, _framerate);

    _camera.StartCapture();
    START_TIMER(_captureTimerId, 1000 / _framerate);

    return _source->element();
}

void FlycapCamera::clearSource() {
    KILL_TIMER(_captureTimerId);
    _camera.StopCapture();
    _framerate = 0;

    if (_source) {
        delete _source;
        _source = NULL;
    }
}

void FlycapCamera::timerEvent(QTimerEvent *e) {
    Q_UNUSED(e);
    if (_source->NeedsData) {
        if (!_camera.IsConnected()) {
            LOG_E("Flycap camera is no longer connected (or never was connected), sending EOS");
            _source->endOfStream();
            _source->NeedsData = false;
            return;
        }

        // get raw image
        FlyCapture2::Error error = _camera.RetrieveBuffer(&_imageBuffer);
        if (error != FlyCapture2::PGRERROR_OK) {
            LOG_W("Error reading image from flycap camera");
            return;
        }

        // create a GstBuffer to hold the converted image
        QGst::BufferPtr ptr = QGst::Buffer::create(3 * _imageBuffer.GetRows() * _imageBuffer.GetCols());
        QGst::MapInfo memory;
        ptr->map(memory, QGst::MapWrite);

        // convert image
        FlyCapture2::Image convertedImage;
        error = convertedImage.SetData(memory.data(), memory.size());
        if (error != FlyCapture2::PGRERROR_OK) {
            LOG_W("Error assigning buffer to FlyCapture2::Image");
            return;
        }
        error = _imageBuffer.Convert(FlyCapture2::PIXEL_FORMAT_RGB, &convertedImage);
        qDebug() << convertedImage.GetDataSize();
        if (error != FlyCapture2::PGRERROR_OK) {
            LOG_W("Error converting camera RAW data to RGB");
            return;
        }

        //push frame to source
        ptr->unmap(memory);
        _source->pushBuffer(ptr);
    }
}

int FlycapCamera::getVideoWidth() const {
    return _width;
}

int FlycapCamera::getFramerate() const {
    return _framerate;
}

const FlyCapture2::Camera* FlycapCamera::getCamera() const {
    return &_camera;
}

int FlycapCamera::getVideoHeight() const {
    return _height;
}

} // namespace Rover
} // namespace Soro
