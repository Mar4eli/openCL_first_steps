#include "openCLWindow.h"
#include "ui_openCLWindow.h"

openCLWindow::openCLWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::openCLWindow)
{
    ui->setupUi(this);
}

openCLWindow::~openCLWindow()
{
    delete ui;
}
