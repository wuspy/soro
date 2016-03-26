#ifndef GLFWMAPDIALOG_H
#define GLFWMAPDIALOG_H

#include <QtCore>
#include <QDialog>
#include <QAbstractButton>
#include <QMessageBox>
#include <QTimerEvent>
#include <QDebug>

#include "soroutil.h"
#include "glfwmap.h"

#include "GLFW/glfw3.h"

namespace Ui {
    class GlfwMapDialog;
}

namespace Soro {
namespace MissionControl {

    class GlfwMapDialog : public QDialog {
        Q_OBJECT

    public:
        explicit GlfwMapDialog(QWidget *parent = 0, int controllerId = -1, Soro::GlfwMap *map = NULL);
        ~GlfwMapDialog();

    private:
        Ui::GlfwMapDialog *ui;
        Soro::GlfwMap *_map;
        int _controllerId;
        int _joyTimerId = TIMER_INACTIVE;
        void populateTable();

    private slots:
        void tableCellClicked(int row, int column);
        void buttonClicked(QAbstractButton *button);

    protected:
        void timerEvent(QTimerEvent *e);
        void showEvent(QShowEvent *e);
        void hideEvent(QHideEvent *e);
    };

}
}

#endif // GLFWMAPDIALOG_H
