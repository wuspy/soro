#ifndef SORO_MISSIONCONTROL_VIDEOCONTROLWIDGET_H
#define SORO_MISSIONCONTROL_VIDEOCONTROLWIDGET_H

#include <QWidget>
#include <QRadioButton>

#include "soro_global.h"

namespace Ui {
class VideoControlWidget;
}

namespace Soro {
namespace MissionControl {

/* UI control for controlling the quality of a video
 * stream.
 */
class VideoControlWidget : public QWidget {
    Q_OBJECT

public:

    explicit VideoControlWidget(QWidget *parent = 0);
    ~VideoControlWidget();

    void selectOption(VideoFormat option);
    void setName(QString name);
    void setAvailable(bool available);
    QString getName();

signals:
    void optionSelected(VideoFormat option);
    void userEditedName(QString newName);

private:
    Ui::VideoControlWidget *ui;
    bool _available = true;

private slots:
    void uiOptionSelected(int index);
    void editButtonClicked();
    void nameEditReturnClicked();
};

}
}

#endif // SORO_MISSIONCONTROL_VIDEOCONTROLWIDGET_H
