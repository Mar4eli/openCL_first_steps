#ifndef OPENCLWINDOW_H
#define OPENCLWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <CL/cl.hpp>
#include <QDebug>
#include <QTime>

namespace Ui {
class openCLWindow;
}

class openCLWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit openCLWindow(QWidget *parent = 0);
    ~openCLWindow();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::openCLWindow *ui;
    bool runTest();
    bool asKernel(double *offset, double *inNumber, double *max, ulong *rez, ulong *rez2, ulong zind);
};

#endif // OPENCLWINDOW_H
