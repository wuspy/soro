#include "flycapcamera.h"

namespace Soro {
namespace Rover {

FlycapSource::FlycapSource(int width, int height, int framerate) {
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
}

void FlycapSource::needData(uint length) {
    Q_UNUSED(length);
    NeedsData = true;
}

void FlycapSource::enoughData() {
    NeedsData = false;
}

/////////////////////////////////////////////////////

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

    // start flycapture camera
    _camera.StartCapture();
    _framerate = 0;
}

QGst::ElementPtr FlycapCamera::createElement(int framerate) {
    clearElement();
    _framerate = framerate;
    if (framerate == 0) {
        LOG_W("Cannot create source with framerate to 0, returning null ElementPtr");
        return QGst::ElementPtr::wrap(NULL, false);
    }
    LOG_I("Creating new element with framerate to " + QString::number(framerate) + "/1");
    _source = new FlycapSource(_width, _height, _framerate);
    killTimer(_captureTimerId);
    _captureTimerId = startTimer(1000 / framerate, Qt::PreciseTimer);
    return _source->element();
}

void FlycapCamera::clearElement() {
    if (_source) {
        delete _source;
        _framerate = 0;
    }
}

FlycapCamera::~FlycapCamera() {
    if (_source) {
        _source->endOfStream();
    }
    if (_camera.IsConnected()) {
        _camera.StopCapture();
        _camera.Disconnect();
    }
}

void FlycapCamera::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (_source && _source->NeedsData) {
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
        if (!ptr) {
            LOG_E("Could not allocate frame buffer");
            return;
        }
        QGst::MapInfo memory;
        ptr->map(memory, QGst::MapWrite);
        if (!memory.data()) {
            LOG_E("Could not map frame buffer to contiguous memory block");
            return;
        }
        // convert image
        FlyCapture2::Image convertedImage;
        error = convertedImage.SetData(memory.data(), memory.size());
        if (error != FlyCapture2::PGRERROR_OK) {
            LOG_W("Error assigning buffer to FlyCapture2::Image");
            return;
        }
        error = _imageBuffer.Convert(FlyCapture2::PIXEL_FORMAT_RGB, &convertedImage);
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
