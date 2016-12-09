#ifndef SORO_VIDEOCLIENT_H
#define SORO_VIDEOCLIENT_H

#include <QObject>
#include <QUdpSocket>
#include <QDataStream>
#include <QByteArray>
#include <QList>

#include "channel.h"
#include "videoformat.h"
#include "socketaddress.h"

#include "soro_global.h"
#include "mediaclient.h"

namespace Soro {

/* Receives a video stream from a VideoServer by
 * implementing MediaClient
 */
class LIBSORO_EXPORT VideoClient : public MediaClient {
    Q_OBJECT

public:
    explicit VideoClient(int mediaId, SocketAddress server, QHostAddress host, QObject *parent = 0);

    VideoFormat getVideoFormat() const;

private:
    VideoFormat _format;

protected:
    void onServerStreamingMessageInternal(QDataStream& stream) Q_DECL_OVERRIDE;
    void onServerStartMessageInternal() Q_DECL_OVERRIDE;
    void onServerEosMessageInternal() Q_DECL_OVERRIDE;
    void onServerErrorMessageInternal() Q_DECL_OVERRIDE;
    void onServerConnectedInternal() Q_DECL_OVERRIDE;
    void onServerDisconnectedInternal() Q_DECL_OVERRIDE;
};

} // namespace Soro

#endif // SORO_VIDEOCLIENT_H
