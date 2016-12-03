#ifndef SORO_MISSIONCONTROL_CLICKABLELABEL_H
#define SORO_MISSIONCONTROL_CLICKABLELABEL_H

#include <QLabel>
#include <QEvent>

class ClickableLabel : public QLabel {
Q_OBJECT
public:
    explicit ClickableLabel(QWidget* parent=0);
    ~ClickableLabel();
signals:
    void clicked();
    void hover();
protected:
    void mouseReleaseEvent(QMouseEvent *event);

};

#endif // SORO_MISSIONCONTROL_CLICKABLELABEL_H
