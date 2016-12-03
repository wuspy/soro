#include "researchwindow.h"
#include "ui_researchwindow.h"

ResearchWindow::ResearchWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ResearchWindow)
{
    ui->setupUi(this);
}

ResearchWindow::~ResearchWindow()
{
    delete ui;
}
