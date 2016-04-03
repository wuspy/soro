#include "videopuncherworker.h"

#define CONFIG_TAG_IN_HOST "InboundHost"
#define CONFIG_TAG_IN_PORT "InboundPort"
#define CONFIG_TAG_LH_PORT "LocalPort"
#define CONFIG_TAG_REMOTE_HOST "RemoteHost"
#define CONFIG_TAG_REMOTE_PORT "RemotePort"

using namespace Soro;

VideoPuncherWorker::VideoPuncherWorker(QObject *parent) : QObject(parent){
    IniParser parser;
    QFile configFile(QCoreApplication::applicationDirPath() + "/video_puncher.ini");
    if (!parser.load(configFile)) {
        qCritical() << "Failed to load video_puncher.ini";
    }

    QHostAddress host;
    int tmp;
    if (!parser.valueAsInt(CONFIG_TAG_IN_PORT, &tmp)) {
        qCritical() << "Cannot parse value for InboundPort";
        return;
    }
    _inPort = (quint16)tmp;
    if (!parser.valueAsInt(CONFIG_TAG_LH_PORT, &tmp)) {
        qCritical() << "Cannot parse value for LocalPort";
        return;
    }
    _lhPort = (quint16)tmp;
    if (!parser.valueAsIP(CONFIG_TAG_IN_HOST, &host, true)) {
        qWarning() << "No host specified, using default";
        host = QHostAddress::Any;
    }
    if (!parser.valueAsInt(CONFIG_TAG_REMOTE_PORT, &tmp)) {
        qCritical() << "Cannot parse value for RemotePort";
        return;
    }
    _remoteAddress.port = tmp;
    if (!parser.valueAsIP(CONFIG_TAG_REMOTE_HOST, &_remoteAddress.address, true)) {
        qCritical() << "Cannot parse value for RemoteHost";
        return;
    }

    _inSocket = new QUdpSocket(this);
    _outSocket = new QUdpSocket(this);
    connect(_inSocket, SIGNAL(readyRead()),
            this, SLOT(inReadyRead()));
    connect(_inSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(inSocketError(QAbstractSocket::SocketError)));
    connect(_outSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(lhSocketError(QAbstractSocket::SocketError)));

    qWarning() << "Forwarding internet port" << _inPort
               << "(From remote host " << _remoteAddress.toString()
               << ") to localhost port " << _lhPort;
    _inSocket->bind(host, _inPort);
    _outSocket->bind(QHostAddress::LocalHost, 12345);
    START_TIMER(_punchTimerId, 500);
}

void VideoPuncherWorker::lhSocketError(QAbstractSocket::SocketError err) {
    qCritical() << "Localhost socket error: " << _outSocket->errorString();
}

void VideoPuncherWorker::inSocketError(QAbstractSocket::SocketError err) {
    qCritical() << "Inbound socket error: " << _inSocket->errorString();
}

void VideoPuncherWorker::inReadyRead() {
    quint64 length;
    qWarning() << "Receiving packets from remote host";
    while (_inSocket->hasPendingDatagrams()) {
        length = _inSocket->readDatagram(_buffer, 100000);
        _outSocket->writeDatagram(_buffer, length, QHostAddress::LocalHost, _lhPort);
    }
}

void VideoPuncherWorker::timerEvent(QTimerEvent *e) {
    QObject::timerEvent(e);
    if (e->timerId() == _punchTimerId) {
        qWarning() << "Sending Hello World to remote host " << _remoteAddress.toString();
        _inSocket->writeDatagram(QByteArray("Hello, World!"), _remoteAddress.address, _remoteAddress.port);
    }
}
