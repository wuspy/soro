#include "researchwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ResearchWindow w;
    w.show();

    return a.exec();
}
