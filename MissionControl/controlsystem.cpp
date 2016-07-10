#include "controlsystem.h"

namespace Soro {
namespace MissionControl {

ControlSystem::ControlSystem(const Configuration *config, QObject *parent) : QObject(parent) {
    _config = config;
}

ControlSystem::~ControlSystem() {
    if (_channel) {
        disconnect(_channel, 0, this, 0);
        delete _channel;
    }
}

bool ControlSystem::init(QString channelName, quint16 channelPort, QString *errorString) {
    if (!_config || !_config->isLoaded) {
        if (errorString) *errorString = QString("The main configuration did not load successfully.");
        return false;
    }

    if (_channel) {
        disconnect(_channel, 0, this, 0);
        delete _channel;
    }

    _channel = Channel::createClient(this, SocketAddress(_config->ServerAddress, channelPort), channelName,
            Channel::UdpProtocol, QHostAddress::Any);
    _channel->open();

    if (_channel->getState() == Channel::ErrorState) {
        if (errorString) *errorString = QString("Could not initialize networking for the arm system.");
        delete _channel;
        _channel = NULL;
        return false;
    }

    connect(_channel, SIGNAL(stateChanged(Channel*,Channel::State)),
            this, SLOT(channelStateChanged(Channel*,Channel::State)));
    return true;
}

void ControlSystem::channelStateChanged(Channel *channel, Channel::State state) {
    Q_UNUSED(channel);
    emit connectionStateChanged(state == Channel::ConnectedState);
}

}
}
