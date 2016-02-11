#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QKeyEvent>
#include <QtCore>

#include "GLFW/glfw3.h"

#include "soro.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSerialPort *_serial;
    int _mainTimerID;
    char _pressed;
    //char getCharForKeyEvent(QKeyEvent *e);

protected:
    void timerEvent(QTimerEvent *e);
    //void keyPressEvent(QKeyEvent *e);
    //void keyReleaseEvent(QKeyEvent *e);

private slots:
    void serialReadyRead();
};

#endif // MAINWINDOW_H
