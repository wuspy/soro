#ifndef SORO_AUDIOCLIENT_H
#define SORO_AUDIOCLIENT_H

#include "enums.h"

#include "soro_global.h"
#include "mediaclient.h"

namespace Soro {

/* Receives an audio stream from an AudioServer by
 * implementing MediaClient
 */
class LIBSORO_EXPORT AudioClient : public MediaClient {
    Q_OBJECT

public:
    explicit AudioClient(int mediaId, SocketAddress server, QHostAddress host, QObject *parent = 0);

    AudioFormat getAudioFormat() const;

private:
    AudioFormat _format = AudioFormat_Null;

protected:
    void onServerStreamingMessageInternal(QDataStream& stream) Q_DECL_OVERRIDE;
    void onServerStartMessageInternal() Q_DECL_OVERRIDE;
    void onServerEosMessageInternal() Q_DECL_OVERRIDE;
    void onServerErrorMessageInternal() Q_DECL_OVERRIDE;
    void onServerConnectedInternal();
    void onServerDisconnectedInternal();
};

} // namespace Soro

#endif // SORO_AUDIOCLIENT_H
