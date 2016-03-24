#ifndef GLFWMAPDIALOG_H
#define GLFWMAPDIALOG_H

#include <QDialog>
#include <QAbstractButton>
#include <QTimerEvent>

#include <glfwmap.h>

#include "GLFW/glfw3.h"

namespace Ui {
    class GlfwMapDialog;
}

namespace Soro {
namespace MissionControl {

    class GlfwMapDialog : public QDialog {
        Q_OBJECT

    public:
        explicit GlfwMapDialog(QWidget *parent = 0, Soro::GlfwMap *map = NULL);
        ~GlfwMapDialog();

    private:
        Ui::GlfwMapDialog *ui;
        Soro::GlfwMap *_map;
        int _controllerId = GLFW_JOYSTICK_1;
        int _joyTimerId = -1;
        int joyIdByName(QString name);
        void populateJoyMenu();
        void populateTable();

    private slots:
        void tableCellClicked(int row, int column);
        void joyMenuSelectionChanged(int index);
        void buttonClicked(QAbstractButton *button);

    protected:
        void timerEvent(QTimerEvent *);
    };

}
}

#endif // GLFWMAPDIALOG_H
