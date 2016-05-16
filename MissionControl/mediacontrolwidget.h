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

signals:
    void optionSelected(MediaControlWidget::Option option);

private:
    Ui::MediaControlWidget *ui;
    Mode _mode;
    bool _available = true;

private slots:
    void optionButtonClicked();
};

}
}

#endif // MEDIACONTROLWIDGET_H
