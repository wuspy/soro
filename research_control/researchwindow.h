#ifndef RESEARCHWINDOW_H
#define RESEARCHWINDOW_H

#include <QMainWindow>

namespace Ui {
class ResearchWindow;
}

class ResearchWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ResearchWindow(QWidget *parent = 0);
    ~ResearchWindow();

private:
    Ui::ResearchWindow *ui;
};

#endif // RESEARCHWINDOW_H
