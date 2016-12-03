#ifndef MISSIONCONTROLNETWORK_H
#define MISSIONCONTROLNETWORK_H

#include <QObject>
#include <QUdpSocket>
#include <QDataStream>

#include "libsoro/channel.h"
#include "libsoro/logger.h"
#include "libsoro/enums.h"

#include "soro_missioncontrol_global.h"

namespace Soro {
namespace MissionControl {

/* Manages a
 */
class SORO_MISSIONCONTROLSHARED_EXPORT MissionControlNetwork : public QObject {
    Q_OBJECT
public:
    MissionControlNetwork(QObject *parent);
    ~MissionControlNetwork();

    bool init(QString *error);
    void requestRole(Role role);
    Role getRole() const;
    bool isBroker() const;
    void sendSharedMessage(const char *message, Channel::MessageSize size);
    QHostAddress getBrokerAddress() const;

signals:
    void connected(bool broker);
    void disconnected();
    void roleGranted(Role role);
    void roleDenied(Role role);
    void statisticsUpdate(quint16 networkSize);
    void sharedMessageReceived(const char *message, Channel::MessageSize size);
    void newClientConnected(Channel *channel);

protected:
    void timerEvent(QTimerEvent *e);

private:
    struct Connection {
        Channel *channel = NULL;
        Role role = SpectatorRole;

        ~Connection() {
            delete channel;
        }
    };
    char _buffer[100];
    bool _isBroker = false;
    SocketAddress _brokerAddress;
    bool _connected = false;
    QUdpSocket *_broadcastSocket = NULL;
    QList<Connection*> _brokerConnections;
    Channel *_clientChannel = NULL;
    Role _role = SpectatorRole;
    Role _pendingRole;
    int _broadcastIntentTimerId = TIMER_INACTIVE;
    int _broadcastStateTimerId = TIMER_INACTIVE;
    int _requestRoleTimerId = TIMER_INACTIVE;
    int _requestConnectionTimerId = TIMER_INACTIVE;
    QString _name;

    void clearConnections();
    QString generateName();
    void denyClientRole(SocketAddress address, Role role);
    void acceptClientRole(SocketAddress address, Role role, QString name);

private slots:
    void endNegotiation();
    bool startNegotiation();
    void ensureConnection();
    void negotiation_broadcastSocketReadyRead();
    void broker_broadcastSocketReadyRead();
    void client_broadcastSocketReadyRead();
    void client_channelStateChanged(Channel *channel, Channel::State state);
    void broker_clientChannelStateChanged(Channel *channel, Channel::State state);
    void broker_clientChannelMessageReceived(Channel *channel, const char* message, Channel::MessageSize size);
    void client_channelMessageReceived(Channel *channel, const char* message, Channel::MessageSize size);
    void socketError(QAbstractSocket::SocketError err);
};

}
}

#endif // MISSIONCONTROLNETWORK_H
