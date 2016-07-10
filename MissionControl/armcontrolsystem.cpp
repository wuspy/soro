#include "armcontrolsystem.h"

#define LOG_TAG "ArmControlSystem"
#define MASTER_ARM_FILE "master_arm.ini"

namespace Soro {
namespace MissionControl {

ArmControlSystem::ArmControlSystem(const Configuration *config, QObject *parent) : ControlSystem(config, parent) { }

ArmControlSystem::~ArmControlSystem() {
    if (_mbed) {
        delete _mbed;
    }
}

bool ArmControlSystem::init(QString *errorString) {
    _enabled = false;
    if (!ControlSystem::init(CHANNEL_NAME_ARM, _config->ArmChannelPort, errorString)) return false;

    if (_mbed) {
        disconnect(_mbed, 0, this, 0);
        delete _mbed;
    }

    _mbed = new MbedChannel(SocketAddress(QHostAddress::Any, _config->MasterArmPort), MBED_ID_MASTER_ARM);

    connect(_mbed, SIGNAL(stateChanged(MbedChannel*,MbedChannel::State)),
            this, SLOT(mbedStateChanged(MbedChannel*,MbedChannel::State)));
    connect(_mbed, SIGNAL(messageReceived(const char*,int)),
            this, SLOT(mbedMessageReceived(const char*,int)));

    return reloadMasterArmConfig();
}

void ArmControlSystem::enable() {
    _enabled = true;
}

void ArmControlSystem::disable() {
     _enabled = false;
}

bool ArmControlSystem::reloadMasterArmConfig() {
    QFile masterArmFile(QCoreApplication::applicationDirPath() + "/config/" + MASTER_ARM_FILE);
    if (_armConfig.load(masterArmFile)) {
        LOG_I(LOG_TAG, "Loaded new master arm configuration");
        return true;
    }
    else {
        LOG_E(LOG_TAG, "Failed to load master arm configuration");
        return false;
    }
}

void ArmControlSystem::mbedMessageReceived(const char *message, int size) {
    if (_enabled && _channel && _armConfig.isLoaded) {
        //translate message from master pot values to slave servo values
        memcpy(_buffer, message, size);
        ArmMessage::translateMasterArmValues(_buffer, _armConfig);
        if (_channel != NULL) {
            _channel->sendMessage(_buffer, size);
        }
        else {
            LOG_E(LOG_TAG, "Got message from master arm with null control channel");
        }
    }
    emit masterArmStateChanged(message);
}

void ArmControlSystem::mbedStateChanged(MbedChannel *channel, MbedChannel::State state) {
    Q_UNUSED(channel);
    emit masterArmStateChanged(state == MbedChannel::ConnectedState);
}

bool ArmControlSystem::isMasterArmConnected() const {
    return _mbed->getState() == MbedChannel::ConnectedState;
}

}
}
