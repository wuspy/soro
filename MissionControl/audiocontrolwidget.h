#ifndef AUDIOCONTROLWIDGET_H
#define AUDIOCONTROLWIDGET_H

#include <QWidget>

#include "soro_global.h"

namespace Ui {
class AudioControlWidget;
}

namespace Soro {
namespace MissionControl {

class AudioControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit AudioControlWidget(QWidget *parent = 0);
    ~AudioControlWidget();

    void selectOption(AudioFormat option);
    void setMute(bool mute);
    void setAvailable(bool available);
    bool isMuted();

signals:
    void optionSelected(AudioFormat option);
    void muteToggled(bool mute);

private:
    Ui::AudioControlWidget *ui;

private slots:
    void optionButtonClicked();
};

}
}

#endif // AUDIOCONTROLWIDGET_H
