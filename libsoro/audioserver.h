#ifndef AUDIOSERVER_H
#define AUDIOSERVER_H

#include <QObject>

#include "socketaddress.h"
#include "soro_global.h"
#include "mediaserver.h"
#include "audioformat.h"

namespace Soro {

/**
 * Sends an audio stream to an AudioClient by
 * implementing MediaServer
 */
class LIBSORO_EXPORT AudioServer: public MediaServer {
    Q_OBJECT
public:
    /**
     * @param mediaId Used to identify this particular media stream. Must match on both ends, and should be unique across
     * all media streams to prevent any problems if ports are not properly configured.
     * @param host Address for the socket that will be used to communicate with the media client.
     * @param parent
     */
    explicit AudioServer(int mediaId, SocketAddress host, QObject *parent = nullptr);

    /**
     * Starts an audio stream. If the server is already streaming, it will be stopped and restarted to
     * accomodate any configuration changes.
     *
     * @param deviceName The audio device to connect to and start streaming (hw:*)
     * @param format The audio format to stream.
     */
    void start(QString deviceName, AudioFormat format);

    /**
     * Gets the audio format currently being streamed. If no audio is streaming, this will return AudioFormat_Null.
     */
    AudioFormat getAudioFormat() const;

private:
    AudioFormat _format;
    QString _audioDevice;
    bool _starting = false;

protected:
    /**
     * Begins streaming video to the provided address.
     * This will fail if the stream is not in WaitingState
     */
    void constructChildArguments(QStringList& outArgs, SocketAddress host, SocketAddress address, quint16 ipcPort) Q_DECL_OVERRIDE;

    void onStreamStoppedInternal() Q_DECL_OVERRIDE;

    void constructStreamingMessage(QDataStream& stream) Q_DECL_OVERRIDE;
};

} // namespace Soro

#endif // AUDIOSERVER_H
