#ifndef OPENCLWINDOW_H
#define OPENCLWINDOW_H

#include <QMainWindow>

namespace Ui {
class openCLWindow;
}

class openCLWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit openCLWindow(QWidget *parent = 0);
    ~openCLWindow();

private:
    Ui::openCLWindow *ui;
};

#endif // OPENCLWINDOW_H
