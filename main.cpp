#include "openCLWindow.h"
#include <QApplication>

const ulong ITERATION_STEP=10;
const ulong ITERATION_SIZE=100;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    openCLWindow w;
    w.show();

//    get all platforms (drivers)
    return a.exec();
}

