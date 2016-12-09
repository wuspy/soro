#ifndef SORO_MISSIONCONTROL_AUDIOCONTROLWIDGET_H
#define SORO_MISSIONCONTROL_AUDIOCONTROLWIDGET_H

#include <QWidget>

#include "libsoro/audioformat.h"

#include "soro_missioncontrol_global.h"

namespace Ui {
class AudioControlWidget;
}

namespace Soro {
namespace MissionControl {

/* UI control for controlling the state of an audio stream
 */
class LIBSOROMC_EXPORT AudioControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit AudioControlWidget(QWidget *parent = 0);
    ~AudioControlWidget();

    void selectPlay();
    void selectStop();
    void setMute(bool mute);
    void setAvailable(bool available);
    bool isMuted() const;

signals:
    void playSelected();
    void stopSelected();
    void muteToggled(bool mute);

private:
    Ui::AudioControlWidget *ui;

private slots:
    void optionButtonClicked();
};

}
}

#endif // SORO_MISSIONCONTROL_AUDIOCONTROLWIDGET_H
