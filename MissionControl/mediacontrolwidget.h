#ifndef MEDIACONTROLWIDGET_H
#define MEDIACONTROLWIDGET_H

#include <QWidget>
#include <QRadioButton>
#include <QHBoxLayout>

namespace Ui {
class MediaControlWidget;
}

namespace Soro {
namespace MissionControl {

class MediaControlWidget : public QWidget {
    Q_OBJECT

public:
    enum Mode {
        AudioMode, VideoMode
    };

    enum Option {
        DisabledOption, LowOption, NormalOption, HighOption, UltraOption
    };

    explicit MediaControlWidget(QWidget *parent = 0);
    ~MediaControlWidget();

    void selectOption(MediaControlWidget::Option option);
    void setName(QString name);
    void setAvailable(bool available);
    void setMode(MediaControlWidget::Mode mode);
    QString getName();

signals:
    void optionSelected(MediaControlWidget::Option option);
    void userEditedName(QString newName);

private:
    Ui::MediaControlWidget *ui;
    Mode _mode;
    bool _available = true;

private slots:
    void optionButtonClicked();
    void editButtonClicked();
    void nameEditReturnClicked();
};

}
}

#endif // MEDIACONTROLWIDGET_H
