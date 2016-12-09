#ifndef SORO_MISSIONCONTROL_VIDEOCONTROLWIDGET_H
#define SORO_MISSIONCONTROL_VIDEOCONTROLWIDGET_H

#include <QWidget>
#include <QRadioButton>

#include "libsoro/videoformat.h"

#include "soro_missioncontrol_global.h"

namespace Ui {
class VideoControlWidget;
}

namespace Soro {
namespace MissionControl {

/* UI control for controlling the quality of a video
 * stream.
 */
class LIBSOROMC_EXPORT VideoControlWidget : public QWidget {
    Q_OBJECT

public:

    explicit VideoControlWidget(QWidget *parent = 0);
    ~VideoControlWidget();

    void setFormats(QList<VideoFormat> formats);
    QList<VideoFormat> getFormats() const;
    void selectOption(int index);
    void setName(QString name);
    void setAvailable(bool available);
    QString getName();

signals:
    void optionSelected(int index);
    void userEditedName(QString newName);

private:
    Ui::VideoControlWidget *ui;
    bool _available = true;
    QList<VideoFormat> _formats;

private slots:
    void uiOptionSelected(int index);
    void editButtonClicked();
    void nameEditReturnClicked();
};

}
}

#endif // SORO_MISSIONCONTROL_VIDEOCONTROLWIDGET_H
