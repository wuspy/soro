#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H

#include <QObject>

#include "socketaddress.h"
#include "soro_global.h"
#include "mediaserver.h"
#include "enums.h"

namespace Soro {

/* Sends an audio stream to an AudioClient by
 * implementing MediaServer
 */
class SORO_COMMON_SHARED_EXPORT AudioServer: public MediaServer {
    Q_OBJECT
public:

    explicit AudioServer(int mediaId, SocketAddress host, QObject *parent = NULL);

    void start(QString deviceName, AudioFormat format);

    AudioFormat getAudioFormat() const;

private:
    AudioFormat _format;
    QString _audioDevice;
    bool _starting = false;

protected:
    /* Begins streaming video to the provided address.
     * This will fail if the stream is not in WaitingState
     */
    void constructChildArguments(QStringList& outArgs, SocketAddress host, SocketAddress address, quint16 ipcPort) Q_DECL_OVERRIDE;

    void onStreamStoppedInternal() Q_DECL_OVERRIDE;

    void constructStreamingMessage(QDataStream& stream) Q_DECL_OVERRIDE;
};

} // namespace Soro

#endif // AUDIOSERVER_H
